#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum CONSTS {
    STR_SIZE = 256,
    BUFF_SIZE = 4096,
};

const char localhost[] = "127.0.0.1";
const char DNS[] = "8.8.8.8";
const int DNS_port = 53;

#pragma pack(push, 1)
struct DNS_HEADER {
    uint16_t id;

    uint8_t req_desired : 1;
    uint8_t truncated : 1;
    uint8_t aa : 1;
    uint8_t opcode : 4;
    uint8_t responce : 1;

    uint8_t r_code : 4;
    uint8_t z : 3;
    uint8_t rec_desired : 1;

    uint16_t questions;
    uint16_t answer;
    uint16_t authority;
    uint16_t additional;
};
#pragma pack(pop)

// QUERRY -----------------

#pragma pack(push, 1)
struct DNS_ENDER {
    uint16_t type;
    uint16_t class;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DNS_ANSWER {
    uint16_t name;
    uint16_t type;
    uint16_t class;
    uint32_t time_to_live;
    uint16_t data_length;
    uint32_t address;
};
#pragma pack(pop)

const uint16_t typeA = 0x0001;
const uint16_t classIN = 0x0001;

int main(int argc, char* argv[])
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sock) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in sockadr;
    sockadr.sin_family = AF_INET;
    sockadr.sin_addr.s_addr = inet_addr(DNS);
    sockadr.sin_port = htons(DNS_port);

    char str[STR_SIZE];
    while (scanf("%s", str) != EOF) {
        int str_len = strnlen(str, sizeof(str));
        str[str_len] = '.';
        str[++str_len] = '\0';
        ++str_len;

        int buff_len =
            sizeof(struct DNS_HEADER) + str_len + sizeof(struct DNS_ENDER);
        uint8_t buff[BUFF_SIZE];
        memset(buff, 0, buff_len);
        struct DNS_HEADER* dns_header = (struct DNS_HEADER*)buff;
        struct DNS_ENDER* dns_ender =
            (struct DNS_ENDER*)(buff + sizeof(struct DNS_HEADER) + str_len);

        dns_header->id = 0x2727;
        dns_header->req_desired = 1;
        dns_header->questions = htons(1);
        dns_ender->type = htons(typeA);
        dns_ender->class = htons(classIN);

        char* ask_begin = (char*)(buff + sizeof(struct DNS_HEADER));
        char* point_pos = NULL;
        memcpy(ask_begin + 1, str, str_len - 1);
        while (NULL != (point_pos = strchr(ask_begin + 1, '.'))) {
            ask_begin[0] = point_pos - ask_begin - 1;
            ask_begin = point_pos;
        }
        ask_begin[0] = 0;

        sendto(
            sock,
            buff,
            buff_len,
            0,
            (struct sockaddr*)&(sockadr),
            sizeof(sockadr));
        recvfrom(sock, buff, sizeof(buff), 0, NULL, 0);

        struct DNS_ANSWER* answer = (struct DNS_ANSWER*)(buff + buff_len);
        // printf("%d\n", answer->address);

        // printf("size: %d\n", sizeof(struct DNS_ANSWER));
        // printf("data-len: %d\n", ntohs(answer->data_length));
        // printf("------------\n");
        // printf("0x%04x\n", ntohs(answer->name));
        // printf("0x%04x\n", ntohs(answer->type));
        // printf("0x%04x\n", ntohs(answer->class));
        // printf("0x%08x\n", ntohs(answer->time_to_live));
        // printf("0x%04x\n", ntohs(answer->data_length));
        // printf("------------\n");

        while (ntohs(answer->type) != typeA) {
            answer = (void*)answer + sizeof(struct DNS_ANSWER) -
                     sizeof(uint32_t) + ntohs(answer->data_length);
        }

        struct in_addr print_addr;
        print_addr.s_addr = answer->address;
        printf("%s\n", inet_ntoa(print_addr));

        // int fd = open("out.bin", O_WRONLY);
        // write(fd, buff, BUFF_SIZE);
    }

    close(sock);

    return 0;
}