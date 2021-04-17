#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int scanner()
{
    int ch = 0;

    do {
        ch = getchar();
    } while (ch == '\n' || ch == '\t' || ch == ' ');

    bool good_symb = false;
    while (!(ch == '\n' || ch == '\t' || ch == ' ' || ch == EOF)) {
        ch = getchar();
        good_symb = true;
    }

    return good_symb;
}

int main(int argc, char* argv[])
{
    pid_t pid = 0;
    int cx = 0;
    int status = 0;
    int ret_code = 1;

    while (ret_code) {

        pid = fork();

        if (pid == 0) {
            return scanner();
        } else {
            waitpid(pid, &status, 0);
            ret_code = WEXITSTATUS(status);
            cx += ret_code;
        }
    }

    printf("%d\n", cx);

    return 0;
}