; Copyright (c) 2025, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Experimental ZX2 decoder (61 bytes with initialization, 55 bytes excluding initialization).
; This work is derived from Einar Saukas' ZX2 (https://github.com/einar-saukas/ZX2).

; The decoder assumes reverse order.

		ld	hl,SrcAddr
		ld	de,DstAddr

		ld	b,0
		ld	a,128

Literal		call	EliasGamma
		lddr
		add	a,a
		jr	nc,Match

		call	EliasGamma
CopyMatch	push	hl
		push	ix
		pop	hl
		add	hl,de
		lddr
		pop	hl
		add	a,a
		jr	c,Literal

Match		ld	c,(hl)
		inc	c
		ret	z
		dec	hl
		push	bc
		pop	ix
		call	EliasGamma
		inc	bc
		jr	CopyMatch

EliasGamma	ld	c,1
EliasLoop	add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		ret	nc
		add	a,a
		rl	c
		rl	b
		jr	EliasLoop
