#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    pid_t pid = 0;
    int cx = 0;
    int status = 0;
    int ret_code = 1;

    while (ret_code) {

        pid = fork();

        if (pid == 0) {
            return scanf("%*s") != EOF;
        } else {
            waitpid(pid, &status, 0);
            ret_code = WEXITSTATUS(status);
            cx += ret_code;
        }

    }

    printf("%d\n", cx);

    return 0;
}