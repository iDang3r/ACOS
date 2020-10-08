#include <sys/syscall.h>

#define BUFF_SIZE (1024 * 1024)

long syscall(long number, ...);

void _start() {

    char buff[BUFF_SIZE];
    long read_symbols = 0;

    while (read_symbols = syscall(SYS_read, 0, buff, BUFF_SIZE)) {
        syscall(SYS_write, 1, buff, read_symbols);
    }

    syscall(SYS_exit, 0);
}