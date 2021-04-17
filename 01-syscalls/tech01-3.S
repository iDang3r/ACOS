#include <sys/syscall.h>    

    .intel_syntax noprefix

    .global _start

    .text

#define BUFF_SIZE (50 * 1024 * 1024)

_start:

    mov rax, SYS_brk
    mov rdi, 0
    syscall

    push rax
    push rax

    mov rax, SYS_brk
    pop rdi
    add rdi, BUFF_SIZE
    syscall

    mov rax, SYS_read
    mov rdi, 0
    pop rsi
    push rsi
    mov rdx, BUFF_SIZE
    syscall

    cmp rax, 0
    je exit

    pop rsi

    mov r11, rax
    dec r11
    mov al, [rsi + r11]
    cmp al, '\n'

    mov rdx, 0

    je print_loop

    inc r11
    mov byte ptr [rsi + r11], '\n'

print_loop:

    cmp r11, 0
    je end

    inc rdx
    dec r11

    mov al, [rsi + r11]
    cmp al, '\n'
    
    jne print_loop

    push r11
    push rsi

    mov rax, SYS_write
    mov rdi, 1
    add rsi, r11
    inc rsi
    syscall

    pop rsi
    pop r11

    mov rdx, 0

    cmp r11, 0
    jne print_loop

end:

    mov rax, SYS_write
    mov rdi, 1
    add rsi, r11
    inc rdx
    syscall

exit:
    mov rax, SYS_exit
    mov rsi, 0
    syscall

