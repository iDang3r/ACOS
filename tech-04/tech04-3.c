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

struct List {
    
    char*   buff;
    size_t  buff_size_; 

} list;

enum ERRORS {
    OK              = 0,
    UNKNOWN_ERROR   = -1,
};

int list_init(struct List* list, int fd) {

    if (list == NULL) {
        return UNKNOWN_ERROR;
    }

    if (fd < 0) {
        return UNKNOWN_ERROR;
    }

    struct stat file_stat;
    fstat(fd, &file_stat);

    list->buff = mmap(NULL,               
                        file_stat.st_size, 
                        PROT_READ,         
                        MAP_SHARED,        
                        fd,         
                        0                   
                     );

    return OK;
}

int main(int argc, const char* argv[]) {



    return 0;
}