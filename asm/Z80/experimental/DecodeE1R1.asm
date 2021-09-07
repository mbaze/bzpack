; Copyright (c) 2021, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; E1R1 decoder (39 bytes excluding initialization).

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
		jr	nc,LoadOffset
		inc	b		; Was it a phrase?
		jr	z,CopyBytes
		ex	af,af'
		jr	ReuseOffset
LoadOffset	push	hl
		ex	af,af'
		ld	a,(hl)
ReuseOffset	ld	l,a
		ex	af,af'
		ld	b,0
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
		inc	c
CopyBytes	lddr
		inc	c
		jr	c,NextBit
		pop	hl
		dec	hl
		djnz	NextBit		; Set B = 255 to indicate phrase.

; Shorter version (38 bytes excluding initialization) but it doesn't preserve SP.

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
		jr	nc,LoadOffset
		inc	b		; Was it a phrase?
		jr	z,CopyBytes
		ex	(sp),hl
		jr	ReuseOffset
LoadOffset	push	hl
		ld	l,(hl)
		ld	b,0
		ld	h,b
		push	hl
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
ReuseOffset	inc	c
CopyBytes	lddr
		inc	c
		jr	c,NextBit
		pop	hl
		ex	(sp),hl		; (SP) = last offset
		dec	hl
		djnz	NextBit		; Set B = 255 to indicate phrase.
