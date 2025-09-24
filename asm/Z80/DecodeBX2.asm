; Copyright (c) 2025, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse BX2 decoder (57 bytes with setup, 48 bytes excluding setup).
; This work is inspired by Einar Saukas' ZX2 (https://github.com/einar-saukas/ZX2).

		xor	a
		ld	b,a
		ld	c,a
		ld	hl,SrcAddr
		ld	de,DstAddr

DecodeLoop	call	EliasGamma
		rla
		jr	nc,NewOffset

		lddr

		call	EliasGamma
		rla
		jr	c,RepOffset

NewOffset	ex	af,af'
		ld	a,(hl)
		or	a
		ret	z		; Option to include the end-of-stream marker.
		ex	af,af'
		dec	hl
		inc	bc

RepOffset	push	hl
		ex	af,af'
		ld	h,0
		ld	l,a
		ex	af,af'
		add	hl,de
		lddr
		pop	hl
		jr	DecodeLoop

EliasGamma	inc	c
EliasLoop	add	a,a
		jr	nz,NoFetch
		sbc	a,(hl)
		dec	hl
		rla
NoFetch		ret	nc
		add	a,a
		rl	c
		rl	b
		jr	EliasLoop
