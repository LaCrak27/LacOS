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
    jmp $

disk_check:
    jmp $
