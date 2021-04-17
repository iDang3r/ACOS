#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int cx = 0;
bool is_term = false;

void handler_int(int num)
{
    ++cx;
}

void handler_term(int num)
{
    is_term = true;
}

int main(int argc, char* argv[])
{
    // set SIG_INT
    struct sigaction sig_int;
    sig_int.sa_handler = handler_int;
    sig_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sig_int, NULL);

    // set SIG_TERM
    struct sigaction sig_term;
    sig_term.sa_handler = handler_term;
    sigaction(SIGTERM, &sig_term, NULL);

    // start logic
    printf("%d\n", getpid());
    fflush(stdout);

    while (!is_term) {
        pause();
    }
    printf("%d\n", cx);

    return 0;
}