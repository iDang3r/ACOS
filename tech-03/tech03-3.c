#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>

enum ERRORS {
    OK              = 0,
    UNKNOWN_ERROR   = 1,
};

int main(int argc, const char* argv[]) {

    char buff[PATH_MAX];

    struct stat file_stat_info;

    while (fgets(buff, sizeof(buff), stdin)) {

        char* n_pointer = memchr(buff, '\n', sizeof(buff));
        if (n_pointer != NULL) {
            *n_pointer = '\0';
        }

        int code = lstat(buff, &file_stat_info);
        if (code) {
            continue;
        }

        if (S_ISLNK(file_stat_info.st_mode)) {

            char path[PATH_MAX];

            ssize_t path_size = readlink(buff, path, sizeof(path) - 1);
            if (path_size < 0) {
                continue;
            }
            path[path_size] = '\0';

            // printf("After readlink: %s\n", path);

            char* dir = dirname(buff);
            if (dir == NULL) {
                continue;
            }

            // printf("dir: %s\n", dir);

            strcat(dir, "/");
            strcat(dir, path);

            // printf("full dir: %s\n", dir);

            char* ptr = realpath(dir, path);
            if (ptr == NULL) {
                continue;
            }

            // printf("path: %s\n", path);

            puts(path);
        }

        if (S_ISREG(file_stat_info.st_mode)) {

            char link_name[PATH_MAX] = "link_to_";
            char* base = basename(buff);
            strcat(link_name, base);

            symlink(buff, link_name);

        }

    }


    return 0;
}