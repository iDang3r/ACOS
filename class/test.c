#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, const char* argv[]) {

    int fd = open(argv[1], O_WRONLY | O_CREAT, 0640); // O_CREAT | O_EXCL <- чтобы точно создать, а не перезаписать
    if (-1 == fd) {
        perror("File wasn't open");
        exit(1);
    }

    static const char Hello[] = "Hello\n";
    ssize_t written = write(fd, Hello, sizeof(Hello) - 1);

    if (-1 == written) {
        perror("$err$");
    }

    close(fd);

}