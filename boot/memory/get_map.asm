; SOURCE: https://wiki.osdev.org/Detecting_Memory_(x86)#Getting_an_E820_Memory_Map
; use the INT 0x15, eax= 0xE820 BIOS function to get a memory map
; note: initially di is 0, be sure to set it to a value so that the BIOS code will not be overwritten. 
;       The consequence of overwriting the BIOS code will lead to problems like getting stuck in `int 0x15`
; inputs: es:di -> destination buffer for 24 byte entries
; outputs: bp = entry count, trashes all registers except esi
[bits 16]
mmap_ent equ 0x1001             ; the number of entries will be stored at 0x1001
get_map:
    pusha
    mov ax, 0
    mov es, ax
    mov di, 0x1002          ; Set di to 0x1002. This is where we'll store the first entry
	xor ebx, ebx		; ebx must be 0 to start
	xor bp, bp		; keep an entry count in bp (clear it here)
	mov edx, 0x0534D4150	; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes
	int 0x15
	jc failed	; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150	; Some BIOSes apparently trash this register?
	cmp eax, edx		; on success, eax must have been reset to "SMAP"
	jne failed
	test ebx, ebx		; ebx = 0 implies list is only 1 entry long (worthless)
	je failed
	jmp jmpin
e820lp:
	mov eax, 0xe820		; eax, ecx get trashed on every int 0x15 call
	mov ecx, 24		; ask for 24 bytes again
	int 0x15
	jc e820f		; carry set means "end of list already reached"
	mov edx, 0x0534D4150	; repair potentially trashed register
jmpin:
	jcxz skipent		; skip any 0 length entries
	cmp cl, 20		; got a 24 byte ACPI 3.X response?
	jbe notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je skipent
notext:
	mov ecx, [es:di + 8]	; get lower uint32_t of memory region length
	or ecx, [es:di + 12]	; "or" it with upper uint32_t to test for zero
	jz skipent		; if length uint64_t is 0, skip entry
	inc bp			; got a good entry: ++count, move to next storage spot, if bad entry overwrite
	add di, 24
skipent:
	test ebx, ebx		; if ebx resets to 0, list is complete
	jne e820lp
e820f:
	mov [es:mmap_ent], bp	; store the entry count
    mov [es:mmap_ent-1], byte 0x69	; set magic number for kernel
	clc			; there is "jc" on end of list to this point, so the carry must be cleared
    popa
	ret
failed:
	stc			; "function unsupported" error exit
    popa
	ret