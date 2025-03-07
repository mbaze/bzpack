; Copyright (c) 2021, Aleksey "introspec" Pichugin, Milos "baze" Bazelides, Pavel "Zilog" Cimbal
; This code is licensed under the BSD 2-Clause License.

; Reverse E1 "hardcore" decoder (34 bytes with setup, 29 bytes excluding setup).

; This decoder is optimized for the Sinclair ZX Spectrum and operates under the following
; assumptions:

; 1) The program is launched from BASIC using USR, with a start address of #BFC0.
;    This ensures that A and C are set to %11000000, and B is set to %11000000 - 1.
; 2) The first block is a literal of at least two bytes in length.
; 3) The compressed stream is placed immediately above the entry point.
; 4) There's no end-of-stream marker. The last block overwrites opcodes after LDDR.

		ld	de,DstAddr
		ld	h,b
		ld	l,b

EliasGamma	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	b,a
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
		inc	bc
CopyBytes	lddr
		inc	c
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
