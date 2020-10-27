#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

pid_t launch(char* cmd, char* arg, int in_fd, int out_fd) 
{
    pid_t pid = fork();
    if (pid == 0) {

        if (0 != out_fd) dup2(in_fd,  0);
        if (1 != out_fd) dup2(out_fd, 1);

        close(in_fd);
        close(out_fd);

        execlp(cmd, cmd, arg, NULL);
        _exit(1);
    }
    return pid;
}

int main(int argc, char* argv[]) 
{
    int pipe_fds[2];
    pipe(pipe_fds);

    pid_t ls   = launch("ls",   "-a",  0, pipe_fds[1]);
    pid_t grep = launch("grep", "out", pipe_fds[0], 1);

    close(pipe_fds[0]); close(pipe_fds[1]);

    wait(0); wait(0); // ждём в произвольном порядке

    return 0;
}