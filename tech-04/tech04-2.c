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

void fill_table(int* buff, int n, int w) {

    int i = 0, j = 0, k = 0, cx = 1;
 
    while (i < n * n) {
        
        k++;

        for (j = k - 1; j < n - k + 1; j++) {
            // sprintf(buff + ((k - 1) * n + j) * w, "%*d", w, cx++);
            buff[(k - 1) * n + j] = cx++;
            i++;
        }

        for (j = k; j < n - k + 1; j++) {
            // sprintf(buff + (j * n + n - k) * w, "%*d", w, cx++);
            buff[j * n + n - k] = cx++;
            i++;
        }
 
        for (j = n - k - 1; j >= k - 1; j--) {
            // sprintf(buff + ((n - k) * n + j) * w, "%*d", w, cx++);
            buff[(n - k) * n + j] = cx++;
            i++;
        }

        for (j = n - k - 1; j >= k; j--) {
            // sprintf(buff + (j * n + k - 1) * w, "%*d", w, cx++);
            buff[j * n + k - 1] = cx++;
            i++;
        }
    }
    
}

int main(int argc, const char* argv[]) {

    int n = strtol(argv[2], NULL, 10);
    int w = strtol(argv[3], NULL, 10);

    int fd = open(argv[1], O_RDWR | O_CREAT, 0640);
    if (fd < 0) {
        return UNKNOWN_ERROR;
    }

    size_t file_size = n * n * w + n; // n * n - elems, each takes w bytes + n bytes for all '\n'
    int code = ftruncate(fd, file_size);
    if (code < 0) {
        goto close_file;
    }

    char* buff = mmap ( NULL,               // address will be selected by the system
                                            // (to determine the starting address of the mapping)
                        file_size,          // size of file
                        PROT_READ | PROT_WRITE, // read and write (able)
                        MAP_SHARED,         // work with file, without copy
                        fd,                 // descriptor
                        0                   // offet
                      );


    int* table = (int*)malloc(n * n * sizeof(table[0]));
    fill_table(table, n, w);

    int fpos = 0;
    for (int i = 0; i < n; ++i) {

        for (int j = 0; j < n; j++) {

            sprintf(buff + fpos, "%*d", w, table[i * n + j]);
            fpos += w;

        }

        *(buff + fpos) = '\n';
        ++fpos;

    }
    free(table);

    munmap(buff, file_size);

close_file:
    close(fd);

    return OK;
}