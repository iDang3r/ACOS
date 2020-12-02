#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == sock) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in sockadr;
    sockadr.sin_family = AF_INET;
    sockadr.sin_port = htons(strtol(argv[2], NULL, 10));
    sockadr.sin_addr.s_addr = inet_addr(argv[1]);

    int code = connect(sock, (struct sockaddr*)&sockadr, sizeof(sockadr));
    if (-1 == code) {
        perror("connect");
        exit(2);
    }
    int32_t number = 0;

    while (true) {

        int scanf_ret = scanf("%d", &number);
        if (scanf_ret <= 0) {
            goto end_;
        }

        int write_ret = write(sock, &number, sizeof(number));
        if (write_ret <= 0) {
            goto end_;
        }

        int read_ret = read(sock, &number, sizeof(number));
        if (read_ret <= 0) {
            goto end_;
        }

        printf("%d ", number);
    }

end_:
    shutdown(sock, SHUT_RDWR);
    close(sock);

    return 0;
}