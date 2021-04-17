#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum ERRORS {
    OK = 0,
    UNKNOWN_ERROR = 1,
};

void printer(char* buff, int pos, int w, int cx)
{
    char tmp_c = *(buff + pos + w);
    snprintf(buff + pos, w + 1, "%*d", w, cx);
    *(buff + pos + w) = tmp_c;
}

void fill_table(char* buff, int n, int w)
{
    int i = 0, k = 0, cx = 1, pos = 0;

    while (i < n * n) {
        k++;

        for (int j = k - 1; j < n - k + 1; j++) {
            pos = ((k - 1) * n + j) * w + (k - 1);
            printer(buff, pos, w, cx++);
            i++;
        }

        for (int j = k; j < n - k + 1; j++) {
            pos = (j * n + n - k) * w + (j);
            printer(buff, pos, w, cx++);
            i++;
        }

        for (int j = n - k - 1; j >= k - 1; j--) {
            pos = ((n - k) * n + j) * w + (n - k);
            printer(buff, pos, w, cx++);
            i++;
        }

        for (int j = n - k - 1; j >= k; j--) {
            pos = (j * n + k - 1) * w + (j);
            printer(buff, pos, w, cx++);
            i++;
        }
    }

    for (i = 1; i <= n; ++i) {
        *(buff + i * w * n + i - 1) = '\n';
    }
}

int main(int argc, const char* argv[])
{
    int n = strtol(argv[2], NULL, 10);
    int w = strtol(argv[3], NULL, 10);

    int fd = open(argv[1], O_RDWR | O_CREAT, 0640);
    if (fd < 0) {
        return UNKNOWN_ERROR;
    }

    // n * n - elems, each takes w bytes + n bytes for all '\n'
    size_t file_size = n * n * w + n; 
        
    int code = ftruncate(fd, file_size);
    if (code < 0) {
        goto close_file;
    }

    char* buff = mmap(
        NULL,      // address will be selected by the system
                   // (to determine the starting address of the mapping)
        file_size, // size of file
        PROT_READ | PROT_WRITE, // read and write (able)
        MAP_SHARED,             // work with file, without copy
        fd,                     // descriptor
        0                       // offet
    );

    fill_table(buff, n, w);

    munmap(buff, file_size);

close_file:
    close(fd);

    return OK;
}