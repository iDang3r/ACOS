#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

enum {
    QUEUE_SIZE = 1024,
    BUFF_SIZE = 4096,
};

bool is_term = false;

struct Queue {
    int data_[QUEUE_SIZE];

    int start_;
    int end_;

} queue;

bool queue_not_empty()
{
    return queue.start_ < queue.end_;
}

int queue_get()
{
    if (queue.start_ >= queue.end_) {
        abort();
    }
    return queue.data_[queue.start_++];
}

void queue_push(int value)
{
    if (queue.end_ == QUEUE_SIZE) {
        abort();
    }
    queue.data_[queue.end_++] = value;
}

void handler(int num)
{
    queue_push(num);
}

void handler_term(int num)
{
    is_term = true;
}

void printer(FILE* file)
{
    static char buff[BUFF_SIZE];
    fgets(buff, sizeof(buff), file);
    fputs(buff, stdout);
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    queue.start_ = 0;
    queue.end_ = 0;

    FILE* files[argc];
    for (int i = 1; i < argc; ++i) {
        files[i] = fopen(argv[i], "r");
    }

    struct sigaction sig_ignore;
    sig_ignore.sa_handler = SIG_IGN;
    sig_ignore.sa_flags = SA_RESTART;
    for (int i = 1; i < SIGRTMIN; ++i) {
        if (i == SIGSTOP || i == SIGKILL) {
            continue;
        }
        sigaction(i, &sig_ignore, NULL);
    }

    struct sigaction sig_act;
    sig_act.sa_handler = handler_term;
    sig_act.sa_flags = SA_RESTART;
    sigaction(SIGRTMIN, &sig_act, NULL);

    sig_act.sa_handler = handler;
    for (int i = SIGRTMIN + 1; i < SIGRTMAX; ++i) {
        sigaction(i, &sig_act, NULL);
    }

    // start logic
    while (!is_term) {
        pause();
        if (is_term) {
            for (int i = 1; i < argc; ++i) {
                fclose(files[i]);
            }
            return 0;
        }
        while (queue_not_empty()) {
            int sig_num = queue_get() - SIGRTMIN;
            printer(files[sig_num]);
        }
    }

    return 0;
}