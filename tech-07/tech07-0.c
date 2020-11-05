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

    int in_fd = open(argv[2], O_RDONLY);
    dup2(in_fd, STDIN);
    close(in_fd);

    int pipe_fds[2];
    pipe(pipe_fds);

    pid_t pid = fork();
    if (0 == pid) {

        dup2(pipe_fds[1], STDOUT);
        close(pipe_fds[1]);

        execlp(argv[1], argv[1], NULL);

    } else {

        close(pipe_fds[1]);

        char buff[BUFF_SIZE];
        size_t counter = 0;
        ssize_t read_ret = 1;

        while (read_ret > 0) {
            read_ret = read(pipe_fds[0], buff, sizeof(buff));

            if (read_ret < 0) {
                return UNKNOWN_ERROR;
            }

            counter += read_ret;
        }

        printf("%zu\n", counter);
    }

    return OK;
}