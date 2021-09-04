; Copyright (c) 2021, Aleksey "introspec" Pichugin, Milos "baze" Bazelides, Pavel "Zilog" Cimbal
; This code is released under the terms of the BSD 2-Clause License.

; E1E1 decoder (28 bytes excluding initialization).

; The decoder assumes reverse order. We can omit the end of stream
; marker if we let the last literal overwrite opcodes after LDDR.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	bc,0		; Ideally, these values should be "reused"
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
		rla			; Literal or phrase?
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
		inc	c
CopyBytes	lddr
		inc	c		; Prepare the most-significant bit.
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
