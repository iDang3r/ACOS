#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int n = strtol(argv[1], NULL, 10);

    pid_t pid = 0;

    for (int i = 1; i <= n; ++i) {
        pid = fork();

        if (pid == 0) {
            if (i < n)
                printf("%d ", i);
            else
                printf("%d", i);

            exit(0);
        }

        int status = 0;
        waitpid(pid, &status, 0);
    }

    printf("\n");

    return 0;
}