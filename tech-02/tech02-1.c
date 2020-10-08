#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

enum DESCRIPTORS {
    STDIN  = 0,
    STDOUT = 1,
};

enum FILE_ERROR {
    OK                      = 0,
    FAIL_OPEN_INPUT_FILE    = 1,
    FAIL_OPEN_WRITE_FILE    = 2,
    UNKNOWN_ERROR           = 3,
};

union value_t {
    int         value;
    char        buff[sizeof(int)];
};

union next_pointer_t {
    uint32_t    next_pointer;
    char        buff[sizeof(uint32_t)];
};

struct Item {
  union value_t         value;
  union next_pointer_t  next_pointer;
} item;

int main(int argc, char* argv[]) {
    int exit_code = OK;

    int input_file = open(argv[1], O_RDONLY);
    if (input_file < 0) {
        exit_code = FAIL_OPEN_INPUT_FILE;
        goto exit;
    }

    do {

        ssize_t read_code = read(input_file, &item, sizeof(item));
        if (read_code != sizeof(item)) {
            exit_code = UNKNOWN_ERROR;
            goto exit;
        }

        printf("%d ", item.value.value);

        lseek(input_file, item.next_pointer.next_pointer, SEEK_SET);

    } while(item.next_pointer.next_pointer);


exit:

    close(input_file);

    return exit_code;
}