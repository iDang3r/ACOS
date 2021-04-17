#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum {
    MaxEventsToRead = 256,
    BUFF_SIZE = 4096,
};

struct epoll_event events[MaxEventsToRead];
char buff[BUFF_SIZE];

extern size_t read_data_and_count(size_t N, int in[N])
{
    int epoll_fd = epoll_create(MaxEventsToRead);
    uint64_t total_read = 0;
    struct epoll_event event;

    for (size_t i = 0; i < N; ++i) {

        int flags = fcntl(
            in[N], F_GETFL); // получить предыдущие флаги открытия/создания
        flags |= O_NONBLOCK; // добавить флаг неблокируемости
        fcntl(in[i], F_SETFL, flags); // установить новые флаги

        memset(&event, 0, sizeof(event));
        event.events = EPOLLIN | EPOLLHUP | EPOLLERR;
        event.data.fd = in[i];

        epoll_ctl(
            epoll_fd, // дескриптор, созданный epoll_create
            EPOLL_CTL_ADD, // одна из операций:
            //  - EPOLL_CTL_ADD - добавить дескриптор в наблюдение
            //  - EPOLL_CTL_MOD - модицифировать параметры наблюдения
            //  - EPOLL_CTL_DEL - убрать дескриптор из наблюдения
            in[i], // дескриптор, события над которым нас интересуют
            &event // структура, в которой описаны:
            //  - маска интересуемых событий events
            //  - произвольные данные, которые
            //    as-is попадают в структуру, читаемую
            //    epoll_wait
        );
    }

    while (N > 0) { // крутимся, пока есть с чем рабоать

        int new_events = epoll_wait(
            epoll_fd, // дескриптор, созданный epoll_create
            events,   // куда прочитать события
            MaxEventsToRead, // размер массива для чтения
            -1 // таймаут в миллисекундах, или -1
        );

        for (int i = 0; i < new_events; ++i) {

            int fd = events[i].data.fd;

            int read_ret = 0;
            if ((events[i].events & EPOLLIN) &&
                (read_ret = read(fd, buff, sizeof(buff))) > 0) {
                total_read += read_ret;
            } else { // с этим файловым дескриптором можно расстаться
                --N;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
            }
        }
    }

    close(epoll_fd);
    return total_read;
}