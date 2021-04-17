#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum {
    BUFF_SIZE = 50 * 4096,
};

const char localhost[] = "127.0.0.1";

bool is_term = false;
bool is_wait = false;

void handler_exit(int num)
{
    is_term = true;
    if (is_wait) {
        exit(0);
    }
}

void set_handler()
{
    // set SIGINT and SIGTERM
    struct sigaction sig_exit;
    sig_exit.sa_handler = handler_exit;
    sig_exit.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sig_exit, NULL);
    sigaction(SIGTERM, &sig_exit, NULL);
}

int create_socket_to_listen(const char* port)
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == sock) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in sockadr;
    sockadr.sin_family = AF_INET;
    sockadr.sin_port = htons(strtol(port, NULL, 10));
    sockadr.sin_addr.s_addr = inet_addr(localhost);

    int code = bind(sock, (struct sockaddr*)&sockadr, sizeof(sockadr));
    if (-1 == code) {
        perror("bind");
        exit(2);
    }

    code = listen(sock, SOMAXCONN);
    if (-1 == code) {
        perror("listen");
        exit(3);
    }

    return sock;
}

int main(int argc, char* argv[])
{
    set_handler();

    int sock = create_socket_to_listen(argv[1]);

    char buff[BUFF_SIZE];
    char file_name[FILENAME_MAX];
    char full_path[PATH_MAX];

    while (!is_term) {
        is_wait = true;
        int client_sock = accept(sock, NULL, NULL);
        is_wait = false;
        if (-1 == client_sock) {
            perror("accept");
            exit(4);
        }

        int read_ret = read(client_sock, buff, sizeof(buff));
        if (-1 == read_ret) {
            perror("read");
            exit(5);
        }
        printf("buff: %s\nEND\n", buff);

        int n = 0;
        sscanf(buff, "GET %s HTTP/1.1%n", file_name, &n);
        printf("%s\n", file_name);
        snprintf(full_path, sizeof(full_path), "%s/%s", argv[2], file_name);
        printf("%s\n", full_path);

        int code = access(full_path, F_OK);
        if (-1 == code) {
            dprintf(client_sock, "HTTP/1.1 404 Not Found\r\n");
            // printf("HTTP/1.1 404 Not Found\r\n");
            goto client_end_;
        }

        code = access(full_path, R_OK);
        if (-1 == code) {
            dprintf(client_sock, "HTTP/1.1 403 Forbidden\r\n");
            goto client_end_;
        }

        code = access(full_path, X_OK);
        if (0 == code) {
            pid_t pid = fork();

            if (pid == 0) {
                dprintf(client_sock, "HTTP/1.1 200 OK\r\n");
                dup2(client_sock, 1);
                execl(full_path, argv[2], NULL);
                perror("fork");

                exit(6);
            }

            waitpid(pid, 0, 0);

            goto client_end_;
        }

        struct stat f_stat;
        code = lstat(full_path, &f_stat);
        if (-1 == code) {
            perror("lstat");
            exit(7);
        }

        // everything is OK
        n = 0;
        snprintf(
            buff,
            sizeof(buff),
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: %lld\r\n"
            "\r\n"
            "%n",
            f_stat.st_size,
            &n);

        int fd = open(full_path, O_RDONLY);
        if (-1 == fd) {
            perror("open");
            exit(8);
        }

        read_ret = read(fd, buff + n, sizeof(buff) - n);
        if (-1 == read_ret) {
            perror("read");
            exit(9);
        }
        close(fd);

        int write_ret = write(client_sock, buff, n + read_ret);
        if (-1 == write_ret) {
            perror("write");
            exit(10);
        }

    client_end_:
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
    }

end_:
    shutdown(sock, SHUT_RDWR);
    close(sock);

    return 0;
}