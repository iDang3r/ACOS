#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

enum {

    PROG_MAX_SIZE = 4 * 4096,

};

int main(int argc, char* argv[])
{
    char buff[PROG_MAX_SIZE];

    fgets(buff, sizeof(buff), stdin);
    if (strnlen(buff, sizeof(buff)) == 0) {
        return 0;
    }

    char program[PROG_MAX_SIZE];
    snprintf(
        program,
        sizeof(program),
        "#include <stdio.h>\n"
        "int main()\n"
        ""
        "{\n"
        "   printf(\"%%d\", (%s));\n"
        "   return 0;\n"
        "}\n",
        buff);

    const char file_name[] = "file.c";

    int fd = open(file_name, O_WRONLY | O_CREAT, 0640);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (write(fd, program, strnlen(program, sizeof(program))) < 0) {
        perror("write");
        goto close_file;
    }

close_file:
    close(fd);

    pid_t pid = fork();
    if (pid == 0) {
        execlp("gcc", "gcc", file_name, NULL);
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }

    pid = fork();
    if (pid == 0) {
        execlp("./a.out", "./a.out", NULL);
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }

    unlink(file_name);
    unlink("a.out");

    return 0;
}