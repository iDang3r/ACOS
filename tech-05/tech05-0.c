#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

    int cx = 0;
    pid_t pid = 0;
    while (pid == 0) {
        pid = fork();
        cx += (pid != -1);
    }

    if (pid == -1) {
        printf("%d\n", cx + 1);
    }

    return 0;
}