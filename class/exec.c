#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {

    pid_t pid = fork();
    if (0 == pid) {
        // ls -l -a
        execlp("ls", "ls", "-l", "-a", NULL);
        perror("exec");
        exit(1);

    } else if (-1 == pid) {
        perror("fork");
        exit(1);

    } else if (pid > 0) {
        int status = -1;
        waitpid(pid, &status, 0);


    }
    

    return 0;
}