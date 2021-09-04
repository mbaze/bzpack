; Copyright (c) 2021, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; E1X1 decoder (34 bytes excluding initialization).

; The decoder assumes reverse order. We can omit the end of stream
; marker if we let the last literal overwrite opcodes after LDDR.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	bc,#FF00	; Ideally, these values should be "reused"
		ld	a,%11000000	; e.g. by aligning the addresses.

EliasLength	add	a,a
		rl	c
;		ret	c		; Option to include the end of stream marker.
NextBit		add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,EliasLength
		rla
		inc	b
		jr	nc,LoadOffset
		jr	z,CopyBytes	; Was it a phrase?
		dec	b
LoadOffset	push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256 / 512.
		ld	b,0
		inc	c
CopyBytes	lddr
		inc	c
		jr	c,NextBit
		pop	hl
		dec	hl
		djnz	NextBit		; Set B = 255 to indicate phrase.
