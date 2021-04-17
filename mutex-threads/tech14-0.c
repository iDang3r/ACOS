#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

struct thread_argument {
    int iterates;
    int pos;

    double* data;
    size_t data_size;

    pthread_mutex_t* mutex;
};

void* work(void* arg_v)
{
    struct thread_argument* arg = arg_v;

    while (arg->iterates --> 0)
    {
        pthread_mutex_lock(arg->mutex);

        arg->data[arg->pos] += 1.0;

        if (arg->pos == 0) {
            arg->data[arg->data_size - 1] += 0.99;
        } else {
            arg->data[arg->pos - 1] += 0.99;
        }

        if (arg->pos == arg->data_size - 1) {
            arg->data[0] += 1.01;
        } else {
            arg->data[arg->pos + 1] += 1.01;
        }

        pthread_mutex_unlock(arg->mutex);
    }

    return NULL;
}

int main(int argc, char* argv[])
{
    int iterates = strtol(argv[1], NULL, 10);
    int threads_counter = strtol(argv[2], NULL, 10);

    double data[threads_counter];
    memset(data, 0, sizeof(data));

    pthread_t threads[threads_counter];
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    struct thread_argument thread_arguments[threads_counter];
    for (int i = 0; i < threads_counter; ++i) {
        thread_arguments[i].iterates = iterates;
        thread_arguments[i].pos = i;

        thread_arguments[i].data = data;
        thread_arguments[i].data_size = threads_counter;

        thread_arguments[i].mutex = &mutex;
    }

    for (int i = 0; i < threads_counter; ++i) {
        pthread_create(threads + i, NULL, work, thread_arguments + i);
    }

    for (int i = 0; i < threads_counter; ++i) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < threads_counter; ++i) {
        printf("%.10g ", data[i]);
    }

    pthread_mutex_destroy(&mutex);

    return 0;
}