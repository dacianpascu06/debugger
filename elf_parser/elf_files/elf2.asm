
section .data
  number_data dd 10

section .bss
  number_bss resb 4

section .rodata
  number_rodata dd 20

section .text
    global main

main:
    mov ecx, 6                      
    mov eax, 2
    add eax,ecx
    mov eax,1
    int 0x80

