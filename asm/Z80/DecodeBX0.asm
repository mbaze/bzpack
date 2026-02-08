; Copyright (c) 2025, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse BX0 decoder (68 bytes with setup, 62 bytes excluding setup).
; This work is inspired by Einar Saukas' ZX0 (https://github.com/einar-saukas/ZX0).

		xor	a
		push	af		; Push dummy value onto the stack.
		ld	hl,SrcAddr
		ld	de,DstAddr

DecodeLoop	call	EliasGamma
		lddr
		rla
		jr	c,NewOffset

		call	EliasGamma

RepOffset	ex	(sp),hl
		push	hl
		add	hl,de
		lddr
		pop	hl
		ex	(sp),hl
		rla
		jr	nc,DecodeLoop

NewOffset	pop	bc
		or	a
		call	EliasGamma
		dec	c
		ret	m		; Option to include the end-of-stream marker.
		ld	b,c
		ld	c,(hl)
		dec	hl
		rr	b
		rr	c
		rra
;		inc	bc		; Option to extend the offset range.
		push	bc
		call	EliasGamma
		inc	bc
		jr	RepOffset

EliasGamma	ld	bc,1
EliasLoop	adc	a,a
		jr	nz,NoFetch
		sbc	a,(hl)
		dec	hl
		rla
NoFetch		ret	nc
		add	a,a
		rl	c
		rl	b
		jr	EliasLoop
