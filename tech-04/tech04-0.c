#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/mman.h>

enum ERRORS {
    OK              = 0,
    UNKNOWN_ERROR   = 1,
};

int main(int argc, const char* argv[]) {

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        return UNKNOWN_ERROR;
    }

    struct stat file_stat;
    fstat(fd, &file_stat);

    if (file_stat.st_size == 0) {
        goto close_file;
    }

    char* buff = mmap ( NULL,               // address will be selected by the system
                                            // (to determine the starting address of the mapping)
                        file_stat.st_size,  // size of file
                        PROT_READ,          // readonly
                        MAP_SHARED,         // work with file, without copy
                        fd,                 // descriptor
                        0                   // offet
                      );


    char* curr_ptr = buff;

    while ((curr_ptr = strstr(curr_ptr, argv[2])) != NULL) {

        printf("%u ", (u_int32_t)(curr_ptr - buff));

        curr_ptr++;
    }

    munmap(buff, file_stat.st_size);

close_file:
    close(fd);

    return OK;
}