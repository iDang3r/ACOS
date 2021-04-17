#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

enum FILE_ERROR {
    OK                      = 0,
    FAIL_OPEN_INPUT_FILE    = 1,
    FAIL_OPEN_WRITE_FILE    = 2,
    UNKNOWN_ERROR           = 3,
};

enum CONSTANTS {
    BUFF_SIZE = 1000 * 1000,
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        return UNKNOWN_ERROR;
    }

    int exit_code = OK;

    int input_file = open(argv[1], O_RDONLY);
    if (input_file < 0) {
        exit_code = FAIL_OPEN_INPUT_FILE;
        goto exit;
    }

    int output_file_digits = open(argv[2], O_WRONLY | O_CREAT, 0640),
        output_file_trash  = open(argv[3], O_WRONLY | O_CREAT, 0640);
    if (output_file_digits < 0 || output_file_trash < 0) {
        exit_code = FAIL_OPEN_WRITE_FILE;
        goto exit;
    }

    char buff[BUFF_SIZE];
    ssize_t read_code = 0;
    do {
        read_code = read(input_file, buff, sizeof(buff));

        if (read_code < 0) {
            exit_code = UNKNOWN_ERROR;
            goto exit;
        }

        for (int pos = 0; pos < read_code; ++pos) {

            ssize_t write_code = 0;

            if (isdigit(buff[pos])) {
                write_code = write(output_file_digits, (buff + pos), sizeof(buff[0]));
            } else {
                write_code = write(output_file_trash,  (buff + pos), sizeof(buff[0]));
            }

            if (write_code < 0) {
                exit_code = UNKNOWN_ERROR;
                goto exit;
            }
        }

    } while (read_code == sizeof(buff));
    
exit:
    close(input_file);
    close(output_file_digits);
    close(output_file_trash);

    return exit_code;
}