#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// #define WITH_BUSY
// #define DEBUG

enum ERRORS {
    OK = 0,
    UNKNOWN_ERROR = -1,
};

#pragma pack(push, 1)
struct Node {

    struct Node* next_;
    size_t size_;
    bool busy_;
};
#pragma pack(pop)

struct List {

    struct Node* head_;
    size_t size_;

} list;

void myalloc_initialize(int fd)
{

    if (fd < 0) {
        return;
    }

    struct stat file_stat;
    fstat(fd, &file_stat);

    list.head_ = mmap(
        NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    list.size_ = file_stat.st_size;

    list.head_->next_ = NULL;
    list.head_->size_ = list.size_ - sizeof(struct Node);
    list.head_->busy_ = false;
}

void* my_malloc(size_t size)
{

    struct Node* tmp = list.head_;
    struct Node* best = NULL;

    while (tmp != NULL) {
        if (!tmp->busy_ && size <= tmp->size_ &&
            (best == NULL || tmp->size_ < best->size_)) {

            best = tmp;
        }
        tmp = tmp->next_;
    }

#ifdef DEBUG
    printf("Best block: %p, size: %zu\n", best, best->size_);
#endif

    if (best == NULL) {

#ifdef DEBUG
        printf("Can't find block");
#endif

        return NULL;
    }

    struct Node* new_node =
        (struct Node*)((char*)best + sizeof(struct Node) + size);

#ifdef DEBUG
    printf("NEW block: %p, size: %zu\n", new_node, new_node->size_);
#endif

    if (sizeof(struct Node) + size < best->size_) {
        new_node->next_ = best->next_;
        new_node->size_ = best->size_ - size - sizeof(struct Node);
        new_node->busy_ = false;

        best->next_ = new_node;
        best->size_ = size;
    }

    best->busy_ = true;

    return (char*)(best) + sizeof(struct Node);
}

void* my_malloc_1(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    struct Node* check = list.head_;

    while (check != NULL && size != check->size_) {
        check = check->next_;
    }

    if (check != NULL) {
        check->size_ = 0;
        return (char*)check + sizeof(struct Node);
    }

    size += sizeof(struct Node);

    struct Node* tmp = list.head_;
    struct Node* best = tmp;

    // find first free block
    while (tmp != NULL) {

        if (size <= tmp->size_) {
            best = tmp;
            break;
        }

        tmp = tmp->next_;
    }

    if (best->size_ < size) {
        printf("can't find any buff\n");
        return NULL;
    }

    // find the best block

    while (tmp != NULL) {

        if (size <= tmp->size_ && tmp->size_ < best->size_) {
            best = tmp;
        }

        tmp = tmp->next_;
    }

    if (best->size_ < size) {
        printf("find block error!\n");
        return NULL;
    }

    if (best->size_ == size) {
        best->size_ = 0;
        return (char*)(best) + sizeof(struct Node);
    }

    printf("Best block: %p, size: %zu\n", best, best->size_);

    struct Node* new_node = (struct Node*)((char*)best + size);

    new_node->next_ = best->next_;
    new_node->size_ = best->size_ - size;

    best->next_ = new_node;
    best->size_ = 0; // means busy

    return (char*)(best) + sizeof(struct Node);
}

void dump()
{

    struct Node* tmp = list.head_;

    while (tmp != NULL) {

        printf(
            "Ptr: %p, size: %zu, count_sz: %zu",
            tmp,
            tmp->size_,
            (char*)(tmp->next_) - (char*)(tmp) - sizeof(struct Node));

#ifdef WITH_BUSY
        printf(", busy: %d", (int)tmp->busy_);
#endif

        printf("\n");

        tmp = tmp->next_;
    }
}

void my_free(void* ptr)
{

    struct Node* to_free = (struct Node*)((char*)ptr - sizeof(struct Node));
    struct Node* prev = list.head_;

    if (to_free == list.head_) {
        prev = NULL;
    }

    // find our block
    while (prev != NULL && prev->next_ != to_free) {
        prev = prev->next_;
    }

    struct Node* next = to_free->next_;

    // merge prev block
    if (prev != NULL && !prev->busy_) {
        prev->size_ += to_free->size_ + sizeof(struct Node);
        prev->next_ = next;
        to_free = prev;
    }

    // merge next block
    if (next && !next->busy_) {
        to_free->size_ += next->size_ + sizeof(struct Node);
        to_free->next_ = next->next_;
    }

    to_free->busy_ = false;
}

void my_free_1(void* ptr)
{

    ptr = (char*)ptr - sizeof(struct Node);

    if (list.head_->next_ == NULL) {
        if (list.head_ == ptr) {

            list.head_->size_ = list.size_ - sizeof(struct Node);
        }
        return;
    }

    if (list.head_ == ptr) {

        if (list.head_->next_->size_ != 0) {

            list.head_->size_ += list.head_->next_->size_ + sizeof(struct Node);
            list.head_->next_ = list.head_->next_->next_;

        } else {

            list.head_->size_ = (char*)(list.head_->next_) -
                                (char*)(list.head_) - sizeof(struct Node);
        }

        return;
    }

    printf("try to find: %p\n", ptr);

    struct Node* prev = list.head_;
    struct Node* duri = prev->next_;

    while (duri != NULL && duri != ptr) {
        prev = duri;
        duri = duri->next_;
    }

    if (duri == NULL) {
        printf("free: not allocated ptr\n");
        return;
    }

    printf("OK, found\n");

    if (prev->size_ != 0) { // prev is free
        prev->next_ = duri->next_;

        prev->size_ =
            (char*)(prev->next_) - (char*)(prev) - sizeof(struct Node);
    } else {

        prev = duri;
    }

    if (prev->next_ != NULL && prev->next_->size_ != 0) {
        prev->size_ += prev->next_->size_ + sizeof(struct Node);
        prev->next_ = prev->next_->next_;
        return;
    }

    if (prev->next_ == NULL) {

        prev->size_ =
            (char*)list.head_ + list.size_ - (char*)prev - sizeof(struct Node);

    } else {
        prev->size_ =
            (char*)(prev->next_) - (char*)(prev) - sizeof(struct Node);
    }
}

void myalloc_finalize()
{

    munmap(list.head_, list.size_);
}

#ifdef DEBUG

int main(int argc, const char* argv[])
{

    int fd = open("fd.txt", O_RDWR | O_CREAT, 0640);
    if (fd < 0) {
        printf("Can't open/create swap-file\n");
        return 0;
    }

    myalloc_initialize(fd);

    for (int i = 0; i < 10; ++i) {

        int x = 0;

        printf("print x(1, 2): ");
        scanf("%d", &x);

        if (x == 1) { // malloc

            int sz = 0;
            scanf("%d", &sz);

            void* code = my_malloc(sz);
            printf("malloc: %p\n", code);

        } else if (x == 2) { // free

            void* p = NULL;

            scanf("%p", &p);

            my_free(p);
        }

        dump();
    }

    myalloc_finalize();

    close(fd);

    return 0;
}

#endif