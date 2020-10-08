// #include <unistd.h>
#include <sys/syscall.h>

long syscall(long number, ...);

void _start() {

    const char str[] = "Hello, World!\n";
    // write(1, str, sizeof(str) - 1);

    syscall(SYS_write, 1, str, sizeof(str) - 1);
    syscall(SYS_exit, 0);

}
