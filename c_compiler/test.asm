.section .data
num: .quad 2456

.section .text
.globl _main
_main:
    movq num(%rip), %rax
    movq %rax, %rcx      # Use the value in RAX as the exit code
    call ExitProcess     # Call ExitProcess