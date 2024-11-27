[bits 16]
; Copies ax bytes from es:bx to ds:di
memcpy:
    pusha
memcpy_lp:
    mov dl, [es:bx]
    mov [ds:di], dl ; Copy the value pointed to by bx, to the adress stored in cx
    inc bx
    inc di
    dec ax
    test ax, ax ; Smaller binary output than cmp ax, 0
    je memcpy_end ; if ax = 0 -> all bytes have been transfered, return
    jmp memcpy_lp ; else, loop again
memcpy_end:
    popa
    ret