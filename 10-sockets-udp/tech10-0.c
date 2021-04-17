#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const char localhost[] = "127.0.0.1";

int main(int argc, char* argv[])
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sock) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in sockadr;
    sockadr.sin_family = AF_INET;
    sockadr.sin_addr.s_addr = inet_addr(localhost);
    sockadr.sin_port = htons(strtol(argv[1], NULL, 10));

    int num = 0;
    while (EOF != scanf("%d", &num)) {

        sendto(
            sock,
            &num,
            sizeof(num),
            0,
            (struct sockaddr*)&sockadr,
            sizeof(sockadr));
        recvfrom(sock, &num, sizeof(num), 0, NULL, 0);
        printf("%d\n", num);
        fflush(stdout);
    }

    close(sock);

    return 0;
}