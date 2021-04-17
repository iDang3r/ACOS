#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

bool is_term = false;

void handler_exit(int num)
{
    is_term = true;
}

const char localhost[] = "127.0.0.1";

enum {
    MaxEventsToRead = 256,
    BUFF_SIZE = 4096,
};

void set_handler()
{
    struct sigaction sig_exit;
    sig_exit.sa_handler = handler_exit;
    sig_exit.sa_flags = SA_RESTART;
    // sigaction(SIGINT,  &sig_exit, NULL);
    sigaction(SIGTERM, &sig_exit, NULL);
}

struct epoll_event events[MaxEventsToRead];
char buff[BUFF_SIZE];

int create_socket_to_listen(const char* port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock) {
        perror("socket");
        exit(1);
    }

    int flags =
        fcntl(sock, F_GETFL); // получить предыдущие флаги открытия/создания
    flags |= O_NONBLOCK; // добавить флаг неблокируемости
    fcntl(sock, F_SETFL, flags); // установить новые флаги

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

    int epoll_fd = epoll_create(MaxEventsToRead);
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    event.data.fd = sock;

    epoll_ctl(
        epoll_fd,      // дескриптор, созданный epoll_create
        EPOLL_CTL_ADD, // одна из операций:
        //  - EPOLL_CTL_ADD - добавить дескриптор в наблюдение
        //  - EPOLL_CTL_MOD - модицифировать параметры наблюдения
        //  - EPOLL_CTL_DEL - убрать дескриптор из наблюдения
        sock, // дескриптор, события над которым нас интересуют
        &event // структура, в которой описаны:
        //  - маска интересуемых событий events
        //  - произвольные данные, которые
        //    as-is попадают в структуру, читаемую
        //    epoll_wait
    );

    while (!is_term) {

        int new_events = epoll_wait(
            epoll_fd, // дескриптор, созданный epoll_create
            events,   // куда прочитать события
            MaxEventsToRead, // размер массива для чтения
            -1 // таймаут в миллисекундах, или -1
        );

        for (int i = 0; i < new_events; ++i) {

            if (events[i].data.fd == sock) {

                int client_sock = accept(sock, NULL, NULL);
                if (-1 == client_sock) {
                    perror("accept");
                    exit(4);
                }
                int flags = fcntl(
                    client_sock,
                    F_GETFL); // получить предыдущие флаги открытия/создания
                flags |= O_NONBLOCK; // добавить флаг неблокируемости
                fcntl(client_sock, F_SETFL, flags); // установить новые флаги

                event.data.fd = client_sock;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event);

            } else {

                int read_ret = 0;
                while ((read_ret =
                            read(events[i].data.fd, buff, sizeof(buff))) > 0) {
                    for (int j = 0; j < read_ret; ++j) {
                        buff[j] = toupper(buff[j]);
                    }
                    write(events[i].data.fd, buff, read_ret);
                }

                if (read_ret == 0) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
            }
        }
    }

    close(sock);
    close(epoll_fd);

    return 0;
}