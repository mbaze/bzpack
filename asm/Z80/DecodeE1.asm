; Copyright (c) 2021, Aleksey "introspec" Pichugin, Milos "baze" Bazelides, Pavel "Zilog" Cimbal
; This code is licensed under the BSD 2-Clause License.

; Reverse E1 decoder (39 bytes with setup, 28 bytes excluding setup).

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	bc,0
		ld	a,%11000000

EliasGamma	add	a,a
		rl	c
;		ret	c		; Option to include the end-of-stream marker.
NextBit		add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,EliasGamma
		rla
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to extend the offset range.
		inc	bc
CopyBytes	lddr
		inc	c
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
