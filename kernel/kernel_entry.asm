; Jumps to the kernel's entry function
[bits 32]
[extern main]
call main
jmp $ ; Again, we should never get here