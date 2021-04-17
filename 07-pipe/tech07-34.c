#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

enum ERRORS {
    OK = 0,
    UNKNOWN_ERROR = 1,
};

enum CONSTANTS {
    BUFF_SIZE = 4096,
};

enum desriptors {
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2,
};

pid_t launch(char* command, int in_fd, int out_fd)
{
    pid_t pid = fork();
    if (pid == 0) {
        if (in_fd != STDIN) {
            dup2(in_fd, STDIN);
            close(in_fd);
        }

        if (out_fd != STDOUT) {
            dup2(out_fd, STDOUT);
            close(out_fd);
        }

        execlp(command, command, NULL);
        perror("execlp");
        exit(1);
    }

    return pid;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        return UNKNOWN_ERROR;
    }
    pid_t pid = -1;
    int status = 0;

    int pipes[2][2];
    pipes[1][0] = STDIN;

    for (int i = 1; i < argc - 1; ++i) {
        pipe(pipes[1 - (i & 1)]);

        pid = launch(argv[i], pipes[i & 1][0], pipes[1 - (i & 1)][1]);
        close(pipes[1 - (i & 1)][1]);

        waitpid(pid, &status, 0);

        close(pipes[i & 1][0]);
        close(pipes[i & 1][1]);
    }
    pipes[argc & 1][1] = 1;
    pid = launch(argv[argc - 1], pipes[1 - (argc & 1)][0], STDOUT);
    waitpid(pid, &status, 0);

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            close(pipes[i][j]);
        }
    }

    return OK;
}