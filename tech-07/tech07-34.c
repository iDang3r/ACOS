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

pid_t launch(char* command, int pipe_1[2], int pipe_2[2])
{
    dup2(pipe_1[0], STDIN);
    close(pipe_1[0]);

    pipe(pipe_2);

    pid_t pid = fork();
    if (0 == pid) {
        execlp(command, command, NULL);
        perror("fork");
    }

    return pid;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        return UNKNOWN_ERROR;
    }
    
    int pipes[2][2] = {{0, 0}, {0, 1}};

    launch(argv[1], pipes[1], pipes[0]);
    for (int i = 2; i < argc - 1; ++i) {
        launch(argv[i], pipes[i & 1], pipes[1 - (i & 1)]);
    }
    launch(argv[argc - 1], pipes[1 - (argc & 1)], pipes[argc & 1]);

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            close(pipes[i][j]);
        }
    }

    return OK;
}