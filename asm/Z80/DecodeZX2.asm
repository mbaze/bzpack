; Copyright (c) 2025, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Experimental ZX2 decoder (61 bytes with initialization, 55 bytes excluding initialization).
; This work is derived from Einar Saukas' ZX2 (https://github.com/einar-saukas/ZX2).

; The decoder assumes reverse order.

		ld	hl,SrcAddr
		ld	de,DstAddr

		ld	a,128
Literal		call	EliasGamma
		lddr
		rla
		jr	nc,NewOffset

		call	EliasGamma
RepOffset	push	hl
		ex	af,af'
		ld	h,0
		ld	l,a
		ex	af,af'
		add	hl,de
		lddr
		pop	hl
		rla
		jr	c,Literal

NewOffset	ex	af,af'
		ld	a,(hl)
		inc	a
		ret	z
		dec	hl
		ex	af,af'
		call	EliasGamma
		inc	bc
		jr	RepOffset

EliasGamma	ld	bc,1
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
