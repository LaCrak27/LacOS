; Jumps to the kernel's entry function
[bits 32]
extern kmain
xor ebp, ebp ; Set ebp to NULL
call kmain
jmp $ ; Again, we should never get here