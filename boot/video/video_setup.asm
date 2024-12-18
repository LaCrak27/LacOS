[bits 16]
; Copies ax bytes from es:bx to ds:di
video_setup:
    push ax
    mov ax, 0x0013 ; Set video mode function, mode 13h (linear 256 color)
    int 0x10 ; BIOS interrupt, we switch modes and hopefully load up the palette into the registers
    ; Todo: copy palette into memory
    mov ax, 0x0003 ; Go back to our trusty friend text mode
    int 0x10 ; Actually switch
    ; Todo: same thing
    pop ax
    ret