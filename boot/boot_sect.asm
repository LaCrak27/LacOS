[org 0x7c00]
KERNEL_OFFSET equ 0x1000 ; Where the kernel gets loaded

    mov [BOOT_DRIVE], dl ; Store boot drive in DL

    mov bp, 0x9000 ; Stack set-up
    mov sp, bp 

    mov bx, MSG_REAL_MODE ; Tell the user that the system actually is starting
    call print_string 

    call load_kernel

    call switch_to_pm ; We will never return from this

    jmp $ ; Since we should never get here, hang so we don't get the CPU randomly executing code in memory


; Including some nice ass routines i made
%include "boot/print/print_helper.asm"
%include "boot/disk/disk_load.asm"
%include "boot/pm/gdt.asm"
%include "boot/pm/switch_to_pm.asm"
%include "boot/pm/print_string_pm.asm"

[bits 16]

; looad kernel
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print_string

    mov bx, KERNEL_OFFSET ; Place in memory to load data
    mov dh, 15 ; Number of sectors
    mov dl, [BOOT_DRIVE] ; Drive selection
    call disk_load

    ret

[bits 32]
; We arrive here after switching to PM
begin_pm:
    mov ebx, MSG_PROT_MODE
    call print_string_pm

    call KERNEL_OFFSET ; Lots of shit being assumed here, very badd, but will work for now

    jmp $ ; In case it doenst work

; Constants
BOOT_DRIVE db 0
MSG_REAL_MODE db "Boot sector loaded in 16-bit real mode, switching to 32-bit mode... \n", 0
MSG_PROT_MODE db "We are now executing 32-bit instructions \n", 0
MSG_LOAD_KERNEL db "Loading C kernel...\n", 0

; Padding and magic number
times 510-($-$$) db 0
dw 0xaa55