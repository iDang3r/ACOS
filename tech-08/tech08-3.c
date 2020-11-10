#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t sender_pid = 0;
int num = 0;

bool is_term = false;

void handler(int sig_num, siginfo_t* siginfo, void* code)
{
    num = siginfo->si_value.sival_int;
    sender_pid = siginfo->si_pid;
}

int main(int argc, char* argv[])
{
    // set SIG_RTMIN
    struct sigaction sig_act;
    sig_act.sa_sigaction = handler;
    sig_act.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGRTMIN, &sig_act, NULL);

    // start logic

    while (!is_term) {
        pause();
        if (num == 0) {
            return 0;
        }
        union sigval sig_val;
        sig_val.sival_int = num - 1;
        sigqueue(sender_pid, SIGRTMIN, sig_val);
    }

    return 0;
}