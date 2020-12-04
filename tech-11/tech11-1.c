#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define FUSE_USE_VERSION 30 // API version 3.0
#include <fuse.h>

char* concat(const char* s1, const char* s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);

    char* result = (char*)calloc(len1 + len2 + 1, sizeof(char));

    if (!result) {
        fprintf(stderr, "calloc() failed: insufficient memory!\n");
        return NULL;
    }

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);

    return result;
}

enum CONSTS {
    MAX_FOLDERS = 1024,
};

char* source = NULL;
int source_len = 0;
char* folders[MAX_FOLDERS];
int folders_count = 0;

typedef struct {
    char* src;
} my_options_t;

typedef struct {
    char name[FILENAME_MAX];
    int size;
    int offset;
} system_file_t;

struct {
    FILE* main_systemfile;
    system_file_t* files;
    int count;
    int main_offset;
} filesystem;

void open_filesystem(const char* src)
{
    source_len = strlen(src);
    source = (char*)calloc(source_len, 1);
    memcpy(source, src, source_len);

    char* duri = source;
    folders[folders_count++] = duri;

    while (NULL != (duri = strchr(duri, ':'))) {
        duri[0] = '\0';
        ++duri;
        folders[folders_count++] = duri;
    }

    for (int i = 0; i < folders_count; ++i) {
        char* tmp = (char*)calloc(PATH_MAX, 1);
        realpath(folders[i], tmp);
        folders[i] = tmp;
    }

    free(source);
}

int my_stat(const char* path, struct stat* st, struct fuse_file_info* fi)
{
    struct timespec last_time;
    memset(&last_time, 0, sizeof(last_time));

    char* last_path = NULL;

    for (int i = 0; i < folders_count; ++i) {
        char* curr_dir = concat(folders[i], path);

        struct stat st_;
        memset(&st_, 0, sizeof(st_));

        if (0 == stat(curr_dir, &st_)) {
            if (st_.st_mtime > last_time.tv_sec) {
                last_time.tv_sec = st_.st_mtime;
                if (last_path != NULL) {
                    free(last_path);
                }
                last_path = curr_dir;
            } else {
                free(curr_dir);
            }
        }
    }

    if (last_path == NULL) {
        return -ENOENT;
    }

    stat(last_path, st);

    if (S_ISREG(st->st_mode)) {
        st->st_mode = 0444 | S_IFREG;
    }

    if (S_ISDIR(st->st_mode)) {
        st->st_mode = 0555 | S_IFDIR;
    }

    return 0;
}

int my_readdir(
    const char* path,
    void* out,
    fuse_fill_dir_t filler,
    off_t off,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags)
{
    // two mandatory entries: the directory itself and its parent
    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);

    char* files[MAX_FOLDERS];
    int files_counter = 0;

    for (int i = 0; i < folders_count; ++i) {

        char* curr_dir = concat(folders[i], path);

        DIR* dir_fd = opendir(curr_dir);

        if (NULL != dir_fd) {
            struct dirent* dir_str = readdir(dir_fd);
            while (NULL != dir_str) {
                int dir_name_len = strlen(dir_str->d_name) + 1;
                files[files_counter] = (char*)calloc(dir_name_len, 1);
                memcpy(files[files_counter], dir_str->d_name, dir_name_len);
                ++files_counter;

                dir_str = readdir(dir_fd);
            }
            closedir(dir_fd);
        }

        free(curr_dir);
    }

    for (int i = 0; i < files_counter; ++i) {

        char* cur_file = files[i];

        for (int pos = i + 1; pos < files_counter; ++pos) {
            if (strcmp(files[i], files[pos]) == 0) {
                free(files[pos]);
                files[pos] = NULL;

                for (int to_swap = pos + 1; to_swap < files_counter;
                     ++to_swap) {
                    char* tmp = files[to_swap];
                    files[to_swap] = files[to_swap - 1];
                    files[to_swap - 1] = tmp;
                }

                files_counter--;
            }
        }
    }

    for (int i = 0; i < files_counter; ++i) {
        filler(out, files[i], NULL, 0, 0);
    }

    return 0;
}

int my_read(
    const char* path,
    char* out,
    size_t size,
    off_t off,
    struct fuse_file_info* fi)
{
    struct timespec last_time;
    memset(&last_time, 0, sizeof(last_time));

    char* last_path = NULL;

    for (int i = 0; i < folders_count; ++i) {
        char* curr_dir = concat(folders[i], path);

        struct stat st_;
        memset(&st_, 0, sizeof(st_));

        if (0 == stat(curr_dir, &st_)) {
            if (st_.st_mtime > last_time.tv_sec) {
                last_time.tv_sec = st_.st_mtime;
                if (last_path != NULL) {
                    free(last_path);
                }
                last_path = curr_dir;
            } else {
                free(curr_dir);
            }
        }
    }

    if (last_path == NULL) {
        return -ENOENT;
    }

    struct stat st;
    if (0 != stat(last_path, &st)) {
        return -ENOENT;
    }

    if (off >= st.st_size) {
        return 0;
    }

    int f_in = open(last_path, O_RDONLY);
    lseek(f_in, off, SEEK_SET);

    int read_all = 0;
    int read_ret = 0;
    while (read_all < size &&
           (read_ret = read(f_in, out + read_all, size - read_all)) > 0) {
        read_all += read_ret;
    }

    close(f_in);
    return read_all;
}

int my_open(const char* path, struct fuse_file_info* fi)
{
    struct timespec last_time;
    memset(&last_time, 0, sizeof(last_time));

    char* last_path = NULL;

    for (int i = 0; i < folders_count; ++i) {
        char* curr_dir = concat(folders[i], path);

        struct stat st_;
        memset(&st_, 0, sizeof(st_));

        if (0 == stat(curr_dir, &st_)) {
            if (st_.st_mtime > last_time.tv_sec) {
                last_time.tv_sec = st_.st_mtime;
                if (last_path != NULL) {
                    free(last_path);
                }
                last_path = curr_dir;
            } else {
                free(curr_dir);
            }
        }
    }

    if (last_path == NULL) {
        return -ENOENT;
    }

    if (O_RDONLY != (fi->flags & O_ACCMODE)) {
        return -EACCES; // file system is read-only, so can't write
    }
    return 0; // success!
}

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    struct fuse_operations operations = {
        .readdir = my_readdir,
        .getattr = my_stat,
        .open = my_open,
        .read = my_read,
    };

    my_options_t my_options;
    memset(&my_options, 0, sizeof(my_options));

    struct fuse_opt options[2];
    options[0].templ = "--src %s";
    options[0].offset = offsetof(my_options_t, src);
    options[0].value = 0;

    options[1].templ = NULL;
    options[1].offset = 0;
    options[1].value = 0;

    fuse_opt_parse(&args, &my_options, options, NULL);

    if (my_options.src) {
        open_filesystem(my_options.src);
    }

    int ret = fuse_main(
        args.argc,
        args.argv,   // arguments to be passed to /sbin/mount.fuse3
        &operations, // pointer to callback functions
        NULL         // optional pointer to user-defined data
    );

    fclose(filesystem.main_systemfile);
    free(filesystem.files);

    return ret;
}