#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

enum ERRORS {
    OK              = 0,
    UNKNOWN_ERROR   = 1,
};

int main(int argc, const char* argv[]) {
    
    char buff[PATH_MAX];
    size_t size_of_all_files = 0;

    struct stat file_stat_info;

    while (fgets(buff, sizeof(buff), stdin)) {

        char* n_pointer = memchr(buff, '\n', sizeof(buff));
        if (n_pointer != NULL) {
            *n_pointer = '\0';
        }

        int code = lstat(buff, &file_stat_info);
        if (code < 0) {
            return UNKNOWN_ERROR;
        }

        if (S_ISREG(file_stat_info.st_mode)) {
            size_of_all_files += file_stat_info.st_size;
        }

    }
    
    printf("%zu", size_of_all_files);

    return 0;
}