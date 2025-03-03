; Copyright (c) 2025, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; BX2 decoder (59 bytes with initialization, 53 bytes excluding initialization).
; This work is inspired by Einar Saukas' ZX2 (https://github.com/einar-saukas/ZX2).

; The decoder assumes reverse order.

		ld	hl,SrcAddr
		ld	de,DstAddr

		ld	a,128
		push	hl
MainLoop1	pop	hl
MainLoop2	call	EliasGamma
		rla
		jr	c,CopyBytes

NewOffset	ex	af,af'
		ld	a,(hl)
		inc	a
		ret	z
		dec	hl
		ex	af,af'
		inc	bc

RepOffset	push	hl
		ex	af,af'
		ld	h,0
		ld	l,a
		ex	af,af'
		add	hl,de
CopyBytes	lddr
		jr	nc,MainLoop1

		call	EliasGamma
		rla
		jr	nc,NewOffset
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
