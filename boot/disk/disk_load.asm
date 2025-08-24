; load 7 sectors to ES:BX from drive DL 
disk_load:
    mov di, 0
    mov es, di ; Get ES ready
    mov bx, KERNEL_OFFSET
    shr bx, 4 ; FFF0 -> 0FFF
    mov ds, bx ; Get SS ready

    mov ch, 0 ; Start on cylinder 0
    mov dx, 15 ; Load 7 cylinders    
    jmp load_loop

load_end:
    mov bx, 0
    mov ds, bx ; Reset ds for gdt
    ret

; Loads dx cylinders into disk
load_loop:
    call fd_load ; Load cyl 0 into 0x1500
    call move_chunk
    inc ch
    dec dx
    test dx, dx
    jz load_end
    jmp load_loop

fd_load: ; Loads cylinder ch into 0x1500
    push dx
    mov dl, 0 ; Select drive again
    mov bx, 0x1500 ; Load into 0x1500
    mov ah, 0x02 ; Read sector function 
    mov al, 36 ; Read whole cylinder
    mov dh, 0x00 ; Head 0
    mov cl, 0x01 ; Sector 1 (Read whole cylinder)
    int 0x13
    jc disk_error
    pop dx
    ret

move_chunk:
    mov ax, 0x4800 ; Amount of bytes to move
    call memcpy ; Move from ss:bx (0x1500) to es:di (3rd cylinder pointer)
    mov ax, ds
    add ax, 0x0480 ; Move pointer to point to next block
    mov ds, ax
    ret

disk_error:
    mov bx, 0xffff
    xchg bx, bx
    jmp $
