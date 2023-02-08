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
#include "dict.h"
#include "byte_parser.h"
#include "pcg_basic.h"

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

int initialize_socket(const char* string_ip, struct sockaddr_in* sin, int port, unsigned int timeout) {
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        printf("unable to initialize the UDP socket\n");
        return -1;
    }

    printf("socket %d created \n", s);

    inet_pton(AF_INET, string_ip, &sin->sin_addr);
    sin->sin_family = AF_INET;
    sin->sin_port = htons(port);

    if (bind(s, (struct sockaddr*)sin, sizeof(*sin))) {
        printf ("unable to bind the UDP socket \n");
        return -1;
    }

    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(unsigned int))) {
        printf("unable to set socket option for recieve timeout");
        return -1;
    }

    //set_nb(s);
    return s;
}

int recieve_packet(int socket, char* buffer, int buffer_size, struct sockaddr_in* sender_in, int sender_in_size) {
    int len = recvfrom(socket, buffer, buffer_size, 0, (struct sockaddr *)sender_in, &sender_in_size);
    if (len > 0) {
        char addr_as_string[64];
        inet_ntop(AF_INET, &sender_in->sin_addr, addr_as_string, 64);
        //printf("received %d bytes from %s:%d\n", len, addr_as_string, ntohs(sender_in->sin_port));
    }
    return len;
}

void add_client_to_dict(dict_t* dict, struct sockaddr_in* sender_in, void* value) {
    char* key = (char*)malloc(20);
    inet_ntop(AF_INET, &sender_in->sin_addr, key, 20);
    int len = strlen(key);
    dict_insert(&dict, key, len, value);
}

void remove_client_from_dict(dict_t* dict, char* key, int key_len) {
    dict_remove(dict, key, key_len);
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

time_t* get_current_time() {
    time_t* current = (time_t*)malloc(sizeof(time_t));
    time(current);
    return current;
}

void check_status(dict_t* known) {
    time_t now = time(NULL);
    int known_clients = dict_get_count(known);
    for (size_t i = 0; i < known_clients; i++)
    {
        struct dictionary_node* current_client = dict_get_from_index(known, i);
        if (current_client) {
            time_t* last_active_time = (time_t*)current_client->value;
            double seconds_from_last_active_time = difftime(now, *last_active_time);
            if (seconds_from_last_active_time > 5.f) {
                printf("Client %s is inactive for more than 5 seconds (%.2fs)\n", current_client->key, seconds_from_last_active_time);
                //TODO close connection with said client
            }
        }
    }
}

int main(int argc, char** argv) {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(0x0202, &wsa_data)) {
        printf ("unable to initialize winsock2 \n");
        return -1;
    }
#endif

    dict_t* blacklisted = dict_new(1);
    dict_t* challenged = dict_new(1);
    dict_t* known = dict_new(1);

    //Socket info and initialization
    struct sockaddr_in sin;
    int s = initialize_socket("192.168.0.176", &sin, 9999, 2000);

    //Sender infos allocation
    struct sockaddr_in sender_in;
    int sender_in_size = sizeof(sender_in);
    char buffer[4096];
    char sender_ip_string[20];
    int buffer_size = 4096;

    pcg32_srandom(time(NULL) ^ (intptr_t)&printf, (intptr_t)&s);

    while(true) {
        //Recieve packet
        clear_buffer(buffer, buffer_size);
        clear_buffer(sender_ip_string, 20);
        int len = recieve_packet(s, buffer, buffer_size, &sender_in, sender_in_size);
        if (len < 0) {
            check_status(known);
            continue;
        }
        inet_ntop(AF_INET, &sender_in.sin_addr, sender_ip_string, 20);

        check_status(known);

        //Check sender status
        //BLACKLISTED
        struct dictionary_node* search_result = dict_search(blacklisted, sender_ip_string, 20);
        if (search_result) {
            printf("Client %s is blacklisted\n", sender_ip_string);
            //Handle blacklisted
            continue;
        }
        //CHALLENGED
        search_result = dict_search(challenged, sender_ip_string, 20);
        if (search_result) {
            unsigned int sent_result = bytes_to_int((unsigned char*)buffer);
            unsigned int expected_result = *((unsigned int*)search_result->value);
            print_buffer((unsigned char*)buffer, 0, 4);
            printf("Result recieved (%u), expected result is (%u), from %s: ", sent_result, expected_result, sender_ip_string);
            if (sent_result == expected_result) {
                add_client_to_dict(known, &sender_in, (void*)get_current_time());
                printf("ACCEPTED\n");
            }
            else {
                add_client_to_dict(blacklisted, &sender_in, (void*)get_current_time());
                printf("BLACKLISTED\n");
            }
            remove_client_from_dict(challenged, sender_ip_string, strlen(sender_ip_string));
            continue;
        }
        //KNOWN
        search_result = dict_search(known, sender_ip_string, 20);
        if (search_result) {
            printf("Client %s: %s\n", sender_ip_string, buffer);
            struct dictionary_node* current_client = dict_search(known, sender_ip_string, 20);
            if (current_client) {
                free(current_client->value);
                current_client->value = (void*)get_current_time();
            }
            continue;
        }

        //New client
        if (len != 4) { //If bytes sent are not 4 (required int), then blacklist the client
            add_client_to_dict(blacklisted, &sender_in, (void*)get_current_time());
            printf("Client %s blacklisted\n", sender_ip_string);
        }
        else //Send a challenge to the client
        {
            unsigned int random_value = bytes_to_int((unsigned char*)buffer);
            print_buffer((unsigned char*)buffer, 0, 4);
            printf("Random value recieved (%u) from new client %s\n", random_value, sender_ip_string);
            unsigned int rand = pcg32_boundedrand(201);
            unsigned int result = random_value + rand;
            unsigned char bytes_to_send[4];
            int_to_bytes(rand, bytes_to_send);
            print_buffer(bytes_to_send, 0, 4);
            int sent_bytes = sendto(s, (char*)bytes_to_send, 4, 0, (struct sockaddr *)&sender_in, sizeof(sender_in));
            printf("Sent random number (%u) to %s\n", rand, sender_ip_string);
            add_client_to_dict(challenged, &sender_in, (void*)&result);
        }
    }
    dict_destroy(blacklisted);
    dict_destroy(known);
    dict_destroy(challenged);
    return 0;
}
