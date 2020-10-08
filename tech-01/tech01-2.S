#include <sys/syscall.h>    

    .intel_syntax noprefix

    .global _start

    .text

#define BUFF_SIZE 1024

_start:

read_loop:

    mov rax, SYS_read
    mov rdi, 0
    lea rsi, [buff]
    mov rdx, BUFF_SIZE
    syscall 

    push rax

    cmp rax, 0
    je end

    mov rax, SYS_write
    mov rdi, 1
    lea rsi, [buff]
    pop rdx
    syscall

    jmp read_loop

end:

    mov rax, SYS_exit
    mov rdi, 0
    syscall

// DATA START

    .data

buff:
    .skip BUFF_SIZE
