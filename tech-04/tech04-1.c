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
#include <stdint.h>

enum ERRORS {
    OK              = 0,
    UNKNOWN_ERROR   = 1,
};

struct Item {
    int       value;
    uint32_t  next_pointer;
} item;

int main(int argc, const char* argv[]) {

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        return UNKNOWN_ERROR;
    }

    struct stat file_stat;
    fstat(fd, &file_stat);

    if (file_stat.st_size < sizeof(struct Item)) {
        goto close_file;
    }

    void* buff = mmap
                      ( NULL,               // address will be selected by the system
                                            // (to determine the starting address of the mapping)
                        file_stat.st_size,  // size of file
                        PROT_READ,          // readonly
                        MAP_SHARED,         // work with file, without copy
                        fd,                 // descriptor
                        0                   // offet
                      );

    struct Item* item_p = &item;
    item_p->next_pointer = 0;

    do {

        item_p = (struct Item*)(buff + item_p->next_pointer);

        printf("%d ", item_p->value);

    } while(item_p->next_pointer);

    munmap(buff, file_stat.st_size);

close_file:
    close(fd);

    return OK;
}