#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum CONSTS {
    MAX_STACK_SIZE = 4 * 4096,
};

struct thread_argument {
    pthread_mutex_t* mutex;
};

void* counter(void* arg_ptr)
{
    struct thread_argument* arg = (struct thread_argument*)arg_ptr;
    int sum = 0;
    int x = 0;

    bool can_read = true;

    while (can_read) {
        pthread_mutex_lock(arg->mutex);
        if (scanf("%d", &x) != EOF) {
            sum += x;
        } else { // EOF
            can_read = false;
        }
        pthread_mutex_unlock(arg->mutex);
    }
    return (void*)(long long)sum;
}

int main(int argc, char* argv[])
{
    int threads_counter = strtol(argv[1], NULL, 10);

    pthread_t threads[threads_counter];
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, MAX_STACK_SIZE);

    struct thread_argument thread_arguments[threads_counter];

    for (int i = 0; i < threads_counter; ++i) {
        thread_arguments[i].mutex = &mutex;
        pthread_create(threads + i, &attr, &counter, thread_arguments + i);
    }

    long long result = 0;
    for (int i = 0; i < threads_counter; ++i) {
        void* ret = NULL;
        pthread_join(threads[i], &ret);
        result += (long long)ret;
    }

    printf("%lld", result);

    return 0;
}