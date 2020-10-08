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

const char elf_begin[] = {0x7f, 'E', 'L', 'F'};

int main(int argc, const char* argv[]) {
    
    char buff[PATH_MAX];

    struct stat file_stat_info;

    while (fgets(buff, sizeof(buff), stdin)) {

        char* n_pointer = memchr(buff, '\n', sizeof(buff));
        if (n_pointer != NULL) {
            *n_pointer = '\0';
        }

        int fd = open(buff, O_RDONLY);
        if (fd < 0) {
            continue;
        }

        int code = fstat(fd, &file_stat_info);
        if (code < 0) {
            goto close_file;
        }

        if (!(file_stat_info.st_mode & S_IXUSR)) {
            goto close_file;
        } 

        char file_begin[sizeof(elf_begin)];
        ssize_t read_counter = read(fd, file_begin, sizeof(file_begin));
        if (read_counter < 0) {
            goto close_file;
        }

        if (read_counter >= 2 && strncmp(file_begin, "#!", 2) == 0) {

            char inter_buff[PATH_MAX];
            lseek(fd, 2, SEEK_SET);
            read_counter = read(fd, inter_buff, sizeof(inter_buff));

            n_pointer = memchr(inter_buff, '\n', sizeof(inter_buff));
            if (n_pointer != NULL) {
                *n_pointer = '\0';
            }

            code = lstat(inter_buff, &file_stat_info);
            if (code < 0 || !(file_stat_info.st_mode & S_IXUSR)) {
                puts(buff);
            }

        } else { // if not ELF

            if (strncmp(file_begin, elf_begin, sizeof(elf_begin)) != 0) {
                puts(buff);
            }

        }

    close_file:
        close(fd);

    }

    return 0;
}