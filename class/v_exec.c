#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>

void set_stack_limit(uint64_t new_stack_size) {
    
    struct rlimit lim;
    getrlimit(RLIMIT_STACK, &lim);
    lim.rlim_cur = new_stack_size;

    if (-1 == setrlimit(RLIMIT_STACK, &lim)) {
        perror("setrlimit");
        exit(1);
    }
}

// launch PROG ARGS...
int main(int argc, char* argv[]) {

    char* prog = argv[1];
    char* args[argc];

    for (int i = 1; i < argc; ++i) {
        args[i - 1] = argv[i];
    }
    args[argc - 1] = NULL;

    pid_t pid = fork();
    if (0 == pid) {

        execvp(prog, args);

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