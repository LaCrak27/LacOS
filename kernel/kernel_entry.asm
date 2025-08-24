; Jumps to the kernel's entry function
[bits 32]
extern kmain
call kmain
jmp $ ; Again, we should never get here