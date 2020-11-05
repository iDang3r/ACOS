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

int main(int argc, char* argv[])
{
    if (argc < 3) {
        return UNKNOWN_ERROR;
    }

    int pipe_fds[2];
    pipe(pipe_fds);

    pid_t pid = fork();
    if (0 == pid) {

        dup2(pipe_fds[1], STDOUT);
        close(pipe_fds[1]);

        execlp(argv[1], argv[1], NULL);
    }

    pid = fork();
    if (0 == pid) {

        close(pipe_fds[1]);

        dup2(pipe_fds[0], STDIN);
        close(pipe_fds[0]);

        execlp(argv[2], argv[2], NULL);
    }

    close(pipe_fds[0]);
    close(pipe_fds[1]);

    wait(0);
    wait(0);

    return OK;
}