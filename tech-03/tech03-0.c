#include <stdio.h>

#define w_char(x) printf("%s: %c\n", #x, x)
#define w_int(x)  printf("%s: %d\n", #x, x)

void normalize_path(char* path) {

    int offset = 0;
    int pos    = 0;

    while (path[pos + offset] != '\0') {

        // w_char(path[pos + offset]);

        if (path[pos + offset] != '.' && path[pos + offset] != '/') {
            path[pos] = path[pos + offset];
            pos++;
            continue;
        }

        if ((path[pos + offset] == '/' || path[pos + offset] == '.') && path[pos + offset + 1] == '/') {
            offset += 2;
            continue;
        }

        if (path[pos + offset] == '.' && path[pos + offset + 1] == '.') {

            int cx = 2;
            while (path[pos + offset - cx] != '/') {
                cx++;
            }

            pos -= cx;
            offset += cx + 2;

            continue;
        }

        path[pos] = path[pos + offset];
        pos++;

    }

    path[pos] = '\0';

}


// int main() {

//     char s[256];

//     scanf("%s", s);
//     normalize_path(s);
//     printf("%s\n", s);

//     return 0;
// }