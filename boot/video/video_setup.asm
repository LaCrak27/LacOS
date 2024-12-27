[bits 16]
; Gets VGA font in memory to ES:DI
video_setup:
	push ds
	push es
	mov	ax, 1130h ; Bios function to return font
	mov	bh, 6
	int	10h
    ; Copy the thing
	push es
	pop	ds
	pop	es
	mov	si, bp
	mov	cx, 256*16/4
	rep	movsd
	pop	ds
    ret