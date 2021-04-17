#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum CONTANTS {
    MAX_SIZE_OF_STATIC_SORT = 20 * 1024 * 1024,
};

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

int cmp(const void* first, const void* second) {
	return (int)(*(int32_t*)first - *(int32_t*)second);
}

int merge_sort_by_file_main(const char* name_of_file, char* buff, int* file_counter) {

    int file = open(name_of_file, O_RDWR);
    if (file < 0) {
        return FAIL_OPEN_INPUT_FILE;
    }

    size_t size_of_file = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);

    if (size_of_file <= MAX_SIZE_OF_STATIC_SORT) {
        // ура, сортим без тацев с бубном перед файлами
        read(file, buff, size_of_file);

        qsort(buff, size_of_file / sizeof(int32_t), sizeof(int32_t), cmp);

        lseek(file, 0, SEEK_SET);
        write(file, buff, size_of_file);

        return OK;
    }

    char file_1_tmp_name[64],
         file_2_tmp_name[64];

    sprintf(file_1_tmp_name, "tmp_file_%d.txt", *file_counter);
    int file_1 = open(file_1_tmp_name, O_WRONLY | O_CREAT, 0640);
    if (file_1 < 0) {
        return FAIL_OPEN_WRITE_FILE;
    }

    sprintf(file_2_tmp_name, "tmp_file_%d.txt", *file_counter + 1);
    int file_2 = open(file_2_tmp_name, O_WRONLY | O_CREAT, 0640);
    if (file_2 < 0) {
        return FAIL_OPEN_WRITE_FILE;
    }

    *file_counter += 2;

    // раскидаем числа по 2-м файлам, чтобы отсортировать поотдельности
    // конечно, это лучше сделать ещё через буфер, чтобы не гонять системные
    // вызовы по 1 байту, но так тоже зашло
    int32_t scan_num = 0;
    size_t  size = 0;
    while (read(file, &scan_num, sizeof(scan_num)) > 0) {
        if (size & 1) {
            write(file_1, &scan_num, sizeof(scan_num));
        } else {
            write(file_2, &scan_num, sizeof(scan_num));
        }
    }
    // будем закрывать файлы, чтобы не переполнить таблицу дескрипторов
    close(file_1);
    close(file_2);

    // сортируем 1-й файл
    merge_sort_by_file_main(file_1_tmp_name, buff, file_counter);
    // сортируем 2-й файл
    merge_sort_by_file_main(file_2_tmp_name, buff, file_counter);

    // начинаем сливать

    file_1 = open(file_1_tmp_name, O_RDONLY);
    file_2 = open(file_2_tmp_name, O_RDONLY);

    lseek(file, 0, SEEK_SET);

    int32_t num_from_1_file = 0,
            num_from_2_file = 0;

    while (read(file_1, &num_from_1_file, sizeof(num_from_1_file)) > 0 &&
           read(file_2, &num_from_2_file, sizeof(num_from_2_file)) > 0) {

        if (num_from_1_file < num_from_2_file) {
            write(file, &num_from_1_file, sizeof(num_from_1_file));
            // считали 2 числа, но записали только одно, поэтому 
            // нужно откатить указатель на второе число, чтобы не 
            // потерять его впоследствии
            lseek(file_2, -sizeof(num_from_2_file), SEEK_CUR);
        } else {
            write(file, &num_from_2_file, sizeof(num_from_2_file));
            // аналогично
            lseek(file_1, -sizeof(num_from_1_file), SEEK_CUR);
        }

    }

    // добиваем файлы

    ssize_t read_symbols = 0;
    while ((read_symbols = read(file_1, buff, MAX_SIZE_OF_STATIC_SORT)) > 0) {
        write(file, buff, read_symbols);
    }

    while ((read_symbols = read(file_2, buff, MAX_SIZE_OF_STATIC_SORT)) > 0) {
        write(file, buff, read_symbols);
    }

    close(file);
    close(file_1);
    close(file_2);

    // мб стоит удалить эти файлы

    char system_command_buff[70] = "rm ";
    memcpy(system_command_buff + 3, file_1_tmp_name, strlen(file_1_tmp_name));
    system(system_command_buff);
    memcpy(system_command_buff + 3, file_2_tmp_name, strlen(file_2_tmp_name));
    system(system_command_buff);

    return OK;
}

int merge_sort_by_file(const char* name_of_file) {

    char* buff = (char*)malloc(MAX_SIZE_OF_STATIC_SORT);
    if (buff == NULL) {
        return -1;
    }
    
    int file_counter = 1;
    int code = merge_sort_by_file_main(name_of_file, buff, &file_counter);

    free(buff);

    return code;
}

int main(int argc, const char* argv[]) {
    int exit_code = OK;

    merge_sort_by_file(argv[1]);

exit:
    return exit_code;
}