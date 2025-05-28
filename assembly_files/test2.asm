  section .data
      global_variable dd 30
  section .text
    global _start
_start:
    mov rax, 1          
    mov rax, 2
    mov rax, 3
    mov rax, 4

    mov rax, 60         ; sys_exit
    xor rdi, rdi
    syscall


