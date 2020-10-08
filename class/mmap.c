#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char* argv[]) {

    int fd = open(argv[1], O_RDONLY);

    char* buff = mmap(NULL, 
                        4096, 
                        PROT_READ, 
                        MAP_PRIVATE,  // делаем копию файла, чтобы не делать MAP_SHARED; | MMAP_ANONIMOUS
                        fd, 
                        0);

    printf("%s", buff);

    munmap(buff, 4096);

    close(fd);
    return 0;
}