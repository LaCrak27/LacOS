[bits 16]
[org 0x7c00]
KERNEL_OFFSET equ 0xFFF0 ; Where the kernel gets loaded (must end in 0)
MAP_OFFSET equ 0x1000 ; Where to place detected memory map

    mov bp, 0xF000 ; Stack set-up
    mov sp, bp 
    mov ax, 0
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Enable A20 line (GOD that was a hard bug to fix)
    in al, 0x92
    test al, 2
    jnz a20_on
    or al, 2
    and al, 0xFE
    out 0x92, al
a20_on:
    call get_map

    call load_kernel

    call switch_to_pm ; We will never return from this

    jmp $ ; Since we should never get here, hang so we don't get the CPU randomly executing code in memory


; Including some nice ass routines i made
%include "boot/memory/get_map.asm"
%include "boot/memory/memcpy.asm"
%include "boot/disk/disk_load.asm"
%include "boot/pm/gdt.asm"
%include "boot/pm/switch_to_pm.asm"

[bits 16]
; load kernel
load_kernel:
    mov dl, 0 ; Drive selection, select main floppy
    call disk_load ; Will perform 2 load operations to load 100 sectors (7.5KB) into memory [ES:BX]
    ret

[bits 32]
; We arrive here after switching to PM
begin_pm:
    jmp KERNEL_OFFSET + 0x200 ; Jump to where we loaded the kernel (skip boot sect) :)
hang:
    jmp $ ; In case it doenst work

; Padding and magic number ($-$$ is current line)
times 510-($-$$) db 0   
dw 0xaa55