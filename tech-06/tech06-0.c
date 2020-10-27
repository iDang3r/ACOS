#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum {

    PROG_MAX_SIZE = 4 * 4096,

};

int main(int argc, char* argv[])
{
    char program[PROG_MAX_SIZE] = "print(";
    size_t print_sz = strnlen(program, sizeof(program));

    fgets(program + print_sz, sizeof(program) - print_sz, stdin);
    if (strnlen(program, sizeof(program)) == print_sz) {
        return 0;
    }

    char* n_ = strchr(program + print_sz, '\0');
    if (n_ != NULL) {
        n_[0] = ')';
        n_[1] = '\0';
    }

    execlp("python3", "python3", "-c", program, NULL);

    return 0;
}