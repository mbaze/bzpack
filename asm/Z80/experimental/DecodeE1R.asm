; Copyright (c) 2021, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; E1R decoder (40 bytes excluding initialization).

; The decoder assumes reverse order. The end of stream marker can be omitted
; if we let the last literal overwrite opcodes after LDDR and let the code continue.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	bc,#FF00	; Ideally, these values should be "reused"
		ld	a,%11000000	; e.g. by aligning the addresses.

EliasGamma	add	a,a
		rl	c
;		ret	c		; Option to include the end of stream marker.
NextBit		add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,EliasGamma
		rla
		jr	nc,LoadOffset
		inc	b		; Was it a phrase?
		jr	z,CopyBytes
		push	hl
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
