;    xchg bx, bx ; Magic breakpoint.
[org 0x7c00]
KERNEL_OFFSET equ 0xFFF0 ; Where the kernel gets loaded
MAP_OFFSET equ 0x1000 ; Where the kernel gets loaded

    mov [BOOT_DRIVE], dl ; Store boot drive in DL

    mov bp, 0xF000 ; Stack set-up
    mov sp, bp 

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

; looad kernel
load_kernel:
    mov dl, [BOOT_DRIVE] ; Drive selection
    call disk_load ; Will perform 2 load operations to load 100 sectors (7.5KB) into memory [ES:BX]
    ret

[bits 32]
; We arrive here after switching to PM
begin_pm:
    xchg bx, bx
    call KERNEL_OFFSET + 0x200 ; Jump to where we loaded the kernel (skip boot sect) :)

    jmp $ ; In case it doenst work

; Constants
BOOT_DRIVE db 0

; Padding and magic number ($-$$ is current line)
times 510-($-$$) db 0   
dw 0xaa55