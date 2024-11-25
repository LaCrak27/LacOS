; load DH sectors to ES:BX from drive DL
disk_load:
  
    ;pusha
    mov ah, 0x02 ; Read sector function 
    mov al, 35 ; Read dh sectors
    mov ch, 0x00 ; Cylinder 0
    mov dh, 0x00 ; Head 0
    mov cl, 0x02 ; Start from the second sector (right after this)  

    int 0x13 ; Bios interrupt

    jc disk_error
    
    xchg bx, bx
    ;add bx, 0x4600; Move pointer on ram to account for already read sectors
    %ifdef COMMENT

    mov ah, 0x02 ; Read sector function 
    mov al, 35 ; Read dh sectors
    mov ch, 0x00
    mov dh, 0x00 ; Head 0
    mov cl, 0b01000001 ; Sector 1, Cylinder 1*/
    ;       cyl | sec
    %endif

    mov ah, 0x02 ; Read sector function 
    mov al, 35 ; Read dh sectors
    mov ch, 0x00 ; Cylinder 0
    mov dh, 0x00 ; Head 0
    mov cl, 0x41 ; Start from the second sector (right after this)  

    int 0x13

    jc disk_error
    xchg bx, bx
    ;add bx, 0x4600; Move pointer on ram to account for already read sectors

    mov ah, 0x02 ; Read sector function 
    mov al, 35 ; Read dh sectors
    mov ch, 0x00
    mov dh, 0x00 ; Head 0
    mov cl, 0b10000001 ; Sector 1, Cylinder 2

    int 0x13

    jc disk_error
    ;popa
    ret

disk_error:
    mov bx, 0xffff
    xchg bx, bx
    jmp $
