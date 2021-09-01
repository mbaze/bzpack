; Copyright (c) 2021, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; E1X1 decoder.

; The decoder assumes reverse order. We can omit the end of stream
; marker if we let the last literal overwrite opcodes after LDDR.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	bc,0		; Ideally, these values should be "reused"
		ld	a,%11000000	; e.g. by aligning the addresses.

EliasLength	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,EliasLength
		rla
		dec	b		; Test "was phrase" flag.
		jr	z,WasPhrase
		inc	b
		rl	b		; B = 0 / 1 for regular / extended offset.
WasPhrase	jr	c,CopyBytes
LongOffset	push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
		inc	c
CopyBytes	lddr
		inc	c
		jr	c,NextBit
		pop	hl
		dec	hl
		inc	b		; Set "was phrase" flag.
		jr	NextBit
