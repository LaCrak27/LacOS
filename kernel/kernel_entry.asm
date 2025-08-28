; Jumps to the kernel's entry function
[bits 32]
extern kmain
mov ebp, 0x9FC00 ; Set stack again
mov esp, ebp
xor ebp, ebp ; Set ebp to NULL for stack traces
call kmain
jmp $ ; Again, we should never get here