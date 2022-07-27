; Copyright (c) 2022, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; E1ZX decoder (28 bytes excluding initialization, usually 35 bytes including initialization, 39 bytes max).

; The decoder assumes reverse order. End of stream marker is not supported.
; The last literal is expected to overwrite opcodes after LDDR.

		ld	a,128		; Note that the ZX Spectrum's USR function sets A = C upon entry.
		ld	c,a		; If the start address is #xx80 both registers already equal 128.

		ld	hl,SrcAddr
		ld	de,DstAddr
EliasLength	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	b,a
		sub	(hl)		; Fetch and (except for very rare situations) set carry.
		dec	hl
;		scf			; Only use in case of warning.
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
		inc	c		; Prepare the most-significant Elias bit.
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
