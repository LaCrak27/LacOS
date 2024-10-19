; load DH sectors to ES:BX from drive DL
disk_load:
    push dx
    mov ah, 0x02 ; Read sector function 
    mov al, dh ; Read dh sectors
    mov ch, 0x00 ; Cylinder 0
    mov dh, 0x00 ; Head 0
    mov cl, 0x02 ; Start from the second sector (right after this)  

    int 0x13 ; Bios interrupt

    jc disk_error

    pop dx
    cmp dh, al ; Compare sectors read (al) with sectors requested
    jne disk_check ; Jump if not equal
    ret

disk_error:
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $

disk_check:
    mov bx, DISK_CHECK_MSG
    call print_string
    jmp $


DISK_ERROR_MSG: 
    db "System halted, disk read error, please restart and try again", 0

DISK_CHECK_MSG: 
    db "System halted, disk read error (number of bytes different), please restart and try again", 0