#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "pcg_basic.h"
#include "byte_parser.h"

int set_nb(int s) {
#ifdef _WIN32
    unsigned long nb_mode = 1;
    return ioctlsocket(s, FIONBIO, &nb_mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0)
            return flags;
    flags |= O_NONBLOCK;
    return fcntl(s, F_SETFL, flags);
#endif
}

int initialize_socket(const char* server_ip, struct sockaddr_in* server_addr, int server_port) {
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        printf("unable to initialize the UDP socket\n");
        return -1;
    }

    printf("socket %d created \n", s);

    inet_pton(AF_INET, server_ip, &server_addr->sin_addr);
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(server_port);
    
    set_nb(s);
    return s;
}

int recieve_packet(int socket, char* buffer, int buffer_size, struct sockaddr_in* sender_addr, int sender_in_size) {
    int len = recvfrom(socket, buffer, buffer_size, 0, (struct sockaddr *)sender_addr, &sender_in_size);
    if (len > 0) {
        char addr_as_string[64];
        inet_ntop(AF_INET, &sender_addr->sin_addr, addr_as_string, 64);
        //printf("received %d bytes from %s:%d\n", len, addr_as_string, ntohs(sender_addr->sin_port));
    }
    return len;
}

void clear_buffer(char* buffer, int buffer_size) {
    for (size_t i = 0; i < buffer_size; i++)
    {
        buffer[i] = '\0';
    }
}

void print_buffer(unsigned char* buffer, int start, int end) {
    for (size_t i = start; i < end; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(0x0202, &wsa_data)) {
        printf("unable to initialize winsock2\n");
        return -1;
    }
#endif

    struct sockaddr_in server_addr;
    int s = initialize_socket("101.58.214.25", &server_addr, 9999);

    struct sockaddr_in sender_addr;
    int sender_in_size = sizeof(sender_addr);
    char buffer[4096];
    int buffer_size = 4096;

    pcg32_srandom(time(NULL) ^ (intptr_t)&printf, (intptr_t)&s);

    int state = 0;
    unsigned int rand = 0;

    while (true) {
        //Initial state: send random int
        clear_buffer(buffer, 4096);
        if (state == 0) {
            rand = pcg32_boundedrand(201);
            unsigned char bytes_to_send[4];
            int_to_bytes(rand, bytes_to_send);
            print_buffer(bytes_to_send, 0, 4);
            int sent_bytes = sendto(s, (char*)bytes_to_send, 4, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            printf("Sent random number (%u) to server\n", rand);
            state = 1;
        }
        //Waiting for challenge state: waiting the server to send back the challenge to resolve
        else if (state == 1) {
            int len = recieve_packet(s, buffer, buffer_size, &sender_addr, sender_in_size);
            if (len == 4) {
                unsigned int ret = bytes_to_int((unsigned char*)buffer);
                print_buffer((unsigned char*)buffer, 0, 4);
                printf("Recieved random number (%u) from server\n", ret);
                unsigned int result = rand + ret;
                unsigned char bytes_to_send[4];
                int_to_bytes(result, bytes_to_send);
                print_buffer(bytes_to_send, 0, 4);
                int sent_bytes = sendto(s, (char*)bytes_to_send, 4, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
                printf("Sent sum number (%u + %u = %u) to server\n", rand, ret, result);
                state = 2;
            }
        }
        //Accepted state: free to send strings to the server to print
        else if (state == 2) {
            scanf_s("%s", buffer);
            if (strcmp(buffer, "q") == 0)
                break;
            int len = strlen(buffer);
            int sent_bytes = sendto(s, buffer, len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            printf("Sent %d bytes via UDP\n", sent_bytes);
        }
    }
    return 0;
}
