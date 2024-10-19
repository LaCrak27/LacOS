print_string:
    pusha
    mov ah, 0x0e ; Sets the interrupt to BIOS teletype mode.
    jmp print_char
    
print_char:
    mov dx, [bx]
    cmp dl, 0 ; Check if the last character is null
    je end_print_string
    mov al, dl ; Move the character into the a register
    int 0x10 ; Trigger the interrupt
    inc bx ; Offset the memory adress by one
    jmp print_char
    
end_print_string:   
    popa
    ret