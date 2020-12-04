#include <arpa/inet.h>
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
    memset(&filesystem, 0, sizeof(filesystem));

    filesystem.main_systemfile = fopen(src, "r");
    fscanf(filesystem.main_systemfile, "%d\n", &filesystem.count);

    filesystem.files =
        (system_file_t*)calloc(filesystem.count, sizeof(system_file_t));
    int offset = 0;
    for (int i = 0; i < filesystem.count; ++i) {
        fscanf(
            filesystem.main_systemfile,
            "%s %d\n",
            filesystem.files[i].name,
            &filesystem.files[i].size);
        filesystem.files[i].offset = offset;
        offset += filesystem.files[i].size;
    }
    fscanf(filesystem.main_systemfile, "\n");

    filesystem.main_offset = ftell(filesystem.main_systemfile);
}

int my_stat(const char* path, struct stat* st, struct fuse_file_info* fi)
{
    // check if accessing root directory
    if (0 == strcmp("/", path)) {
        st->st_mode = 0444 | S_IFDIR; // file type - dir, access read only
        st->st_nlink = 2;             // at least 2 links: '.' and parent
        return 0;                     // success!
    }

    for (int i = 0; i < filesystem.count; ++i) {
        if (strncmp(path + 1, filesystem.files[i].name, FILENAME_MAX) == 0) {
            st->st_mode =
                S_IFREG | 0444; // file type - regular, access read only
            st->st_nlink = 1;   // one link to file
            st->st_size = filesystem.files[i].size; // bytes available
            return 0;                               // success!
        }
    }

    return -ENOENT;
}

int my_readdir(
    const char* path,
    void* out,
    fuse_fill_dir_t filler,
    off_t off,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags)
{
    if (0 != strcmp(path, "/")) {
        return -ENOENT; // we do not have subdirectories
    }

    // two mandatory entries: the directory itself and its parent
    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);

    // directory contents
    for (int i = 0; i < filesystem.count; ++i) {
        filler(out, filesystem.files[i].name, NULL, 0, 0);
    }

    return 0; // success
}

int my_read(
    const char* path,
    char* out,
    size_t size,
    off_t off,
    struct fuse_file_info* fi)
{
    if (path[0] == '/') {
        for (int i = 0; i < filesystem.count; ++i) {

            if (strncmp(path + 1, filesystem.files[i].name, FILENAME_MAX) ==
                0) {
                fseek(
                    filesystem.main_systemfile,
                    filesystem.main_offset + filesystem.files[i].offset,
                    SEEK_SET);

                if (filesystem.files[i].size < size) {
                    size = filesystem.files[i].size;
                }
                fread(out, 1, size, filesystem.main_systemfile);

                return size;
            }
        }
    }

    return -ENOENT;
}

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    struct fuse_operations operations = {
        .readdir = my_readdir,
        .getattr = my_stat,
        // .open    = my_open,
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