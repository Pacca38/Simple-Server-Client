#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

union {
    float f;
    unsigned long ul;
 } u;

int bytes_to_int(unsigned char* buffer) {
    return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
}

int bytes_to_int_from(unsigned char* buffer, int startIndex) {
    char buffer2[4];
    for (int i = 0; i <4; i++) {
        buffer2[i] = buffer[startIndex + i];
    }
    return (int)(buffer2[0] << 24) + (buffer2[1] << 16) + (buffer2[2] << 8) + buffer2[3];
}

float string_to_float_from(char* buffer, size_t position) {
    char buf[sizeof(float) + 1];
    size_t j = position;
    for (size_t i = 0; i < sizeof(buf); ++i) {
        buf[i] = buffer[j];
        ++j;
    }
    buf[sizeof(buf) - 1] = '\0';
    return strtof(buf, NULL);
}

void int_to_bytes(unsigned int num, unsigned char* bytes) {
    for (int i = 0; i < sizeof(int); i++) {
        bytes[i] = (num >> ((sizeof(int) - i - 1) * 8)) & 0xff;
    }
}

float bytes_to_float(unsigned char* bytes, int startIndex) {
    float num;
    unsigned char byte [4];
    for (int i = 0; i < 4; i++) {
        byte[i] = bytes[startIndex + i];
    }
    unsigned int *tmp = (unsigned int*) byte;
    *tmp = ntohl(*tmp);
    memcpy(&num, byte, sizeof(float));
    return num;
}

unsigned char* float_to_bytes(float toConvert) {
    unsigned char* buffer = (unsigned char* )calloc(1,4);
    memcpy(buffer, &toConvert, sizeof(float));
    return buffer;
}

int bytes_append(unsigned char* Dest, size_t destLength, int startIndex, unsigned char* str2, int bytesToInsert)
{
    for (int i = 0; (i < bytesToInsert); ++i) {
        if(startIndex + i >= destLength)
            return 1;
        char toInsert = str2[i];
        if(toInsert < 0x30)
        {
            //toInsert = 0x30;
        }
        Dest[startIndex + 1] = toInsert;
    }
    return 0;
}

float bytes_to_float2(unsigned char* bytes, int startIndex) {
    u.ul = (bytes[startIndex + 3] << 24) | (bytes[startIndex +2] << 16) | (bytes[startIndex +1] << 8) | bytes[startIndex +0];
    return u.f;
}