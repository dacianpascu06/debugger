section .data
  number_data dq 10

section .bss
    number_bss resb 8

section .rodata
    number_rodata dq 20

section .text
    global main

main:
    mov rcx, 6
    mov rax, 2
    add rax, rcx
    mov rax, 60
    xor rdi, rdi
    syscall

