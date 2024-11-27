; load DH sectors to ES:BX from drive DL 
; ngl this is hacky af but i've been working on it for more than 6 hours and it works so i'm not touching it
; TODO: Make this load 0x1ce40 bytes (don't hardcode stuff so much, just repeat it ig)
disk_load:
    pusha
    mov di, 0
    mov es, di ; Get ES ready
    mov bx, KERNEL_OFFSET
    shr bx, 4 ; FFF0 -> 0FFF
    mov ds, bx ; Get SS ready

    mov ch, 0 ; Cylinder 0
    call fd_load

    mov ax, 0x4800 ; Amount of bytes to move
    xchg bx, bx
    call memcpy ; Move from ss:bx (0x1500) to es:di (3rd cylinder pointer)
    add di, 0x4800 ; Move pointer down to make size for second block
    
    mov ch, 1 ; Cylinder 1
    call fd_load

    mov ax, 0x4800 ; Amount of bytes to move
    xchg bx, bx
    call memcpy ; Move from ss:bx (0x1500) to es:di (3rd cylinder pointer)
    add di, 0x4800 ; Move pointer down to make size for second block
    xchg bx, bx

    mov bx, 0
    mov ds, bx ; Reset ds for gdt
    popa
    ret

fd_load: ; Loads cylinder ch into 0x1500
    mov bx, 0x1500 ; Load into 0x1500
    mov ah, 0x02 ; Read sector function 
    mov al, 36 ; Read whole cylinder
    mov dh, 0x00 ; Head 0
    mov cl, 0x01 ; Sector 1 (Read whole cylinder)

    int 0x13
    jc disk_error
    ret

disk_error:
    mov bx, 0xffff
    xchg bx, bx
    jmp $
