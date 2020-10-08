#include <windows.h>
#include <stdio.h>

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
    DWORD    value;
    BYTE     buff[sizeof(DWORD)];
};

union next_pointer_t {
    DWORD    next_pointer;
    BYTE     buff[sizeof(DWORD)];
};

struct Item {
  union value_t         value;
  union next_pointer_t  next_pointer;
} item;

int main(int argc, char* argv[]) {
    DWORD exit_code = OK;

    // int input_file = open(argv[1], O_RDONLY);
    HANDLE input_file = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (input_file == INVALID_HANDLE_VALUE) {
        exit_code = FAIL_OPEN_INPUT_FILE;
        goto exit;
    }

    do {

        // ssize_t read_code = read(input_file, &item, sizeof(item));
        DWORD bytes_read = 0;
        ReadFile(input_file, &item, sizeof(item), &bytes_read, NULL);
        
        if (bytes_read != sizeof(item)) {
            exit_code = UNKNOWN_ERROR;
            goto exit;
        }

        printf("%d ", item.value.value);

        // lseek(input_file, item.next_pointer.next_pointer, SEEK_SET);
        SetFilePointer(input_file, item.next_pointer.next_pointer, NULL, FILE_BEGIN);

    } while(item.next_pointer.next_pointer);


exit:

    DeleteFileA(argv[1]);

    return exit_code;
}