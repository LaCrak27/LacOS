[bits 16]
; Switch to PM
switch_to_pm:
    cli ; Disable interrupts

    lgdt [gdt_descriptor] ; Load GDT

    mov eax, cr0 ; Make the 1st bit of cr0 to be a one
    or al, 0x1
    mov cr0, eax ; We are in PM

    jmp CODE_SEG:init_pm ; Far jump to 32-bit code, setting CS

[bits 32]
; Initialize registers and stack once in PM
init_pm:
    mov ax, DATA_SEG ; Point segment registers to the data segment
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000 ; Update stack position
    mov esp, ebp

    call begin_pm