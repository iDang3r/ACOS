#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int num = 0;
bool is_usr_1 = false;
bool is_usr_2 = false;
bool is_term = false;

void handler_usr_1(int num)
{
    is_usr_1 = true;
}

void handler_usr_2(int num)
{
    is_usr_2 = true;
}

void handler_term(int num)
{
    is_term = true;
}

int main(int argc, char* argv[])
{
    // set SIG_USR_1
    struct sigaction sig_usr_1;
    sig_usr_1.sa_handler = handler_usr_1;
    sig_usr_1.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sig_usr_1, NULL);

    // set SIG_USR_2
    struct sigaction sig_usr_2;
    sig_usr_2.sa_handler = handler_usr_2;
    sig_usr_2.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sig_usr_2, NULL);

    // set SIG_TERM
    struct sigaction sig_term;
    sig_term.sa_handler = handler_term;
    sigaction(SIGTERM, &sig_term, NULL);

    // start logic
    printf("%d\n", getpid());
    fflush(stdout);
    scanf("%d", &num);

    while (!is_term) {
        pause();
        if (is_usr_1) {
            num++;
            is_usr_1 = false;
        } else if (is_usr_2) {
            num *= -1;
            is_usr_2 = false;
        } else if (is_term) {
            return 0;
        }
        printf("%d ", num);
        fflush(stdout);
    }

    return 0;
}