#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    if (argc < 2) {
        return UNKNOWN_ERROR;
    }

    int pipe_fds[2];
    pipe(pipe_fds);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) {

        dup2(pipe_fds[1], STDERR);
        close(pipe_fds[1]);

        execlp("gcc", "gcc", /*"-Wall",*/ argv[1], NULL);
        perror("exec_gcc");
        exit(1);
    }

    close(pipe_fds[1]);

    FILE* in = fdopen(pipe_fds[0], "r");
    char buff[BUFF_SIZE];

    char error_[] = "error";
    char warning_[] = "warning";

    size_t file_name_size = strnlen(argv[1], NAME_MAX);

    int error_cx = 0;
    int warning_cx = 0;

    int prev_error = -1;
    int prev_warning = -1;

    while (NULL != fgets(buff, sizeof(buff), in)) {

        if (strncmp(buff, argv[1], file_name_size) == 0) {
            char* offset = buff + file_name_size + 1; // +1 to skip ':'

            int tmp = 0, n = 0;
            sscanf(offset, "%d:%*d: %n", &tmp, &n);
            offset += n;

            if (strncmp(offset, error_, sizeof(error_) - 1) == 0) {

                if (tmp > prev_error) {
                    error_cx++;
                    prev_error = tmp;
                }

            } else if (strncmp(offset, warning_, sizeof(warning_) - 1) == 0) {

                if (tmp > prev_warning) {
                    warning_cx++;
                    prev_warning = tmp;
                }
            }
        }
    }

    wait(0);

    printf("%d %d\n", error_cx, warning_cx);

    return OK;
}