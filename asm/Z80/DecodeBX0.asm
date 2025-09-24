; Copyright (c) 2025, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse BX0 decoder (68 bytes with setup, 62 bytes excluding setup).
; This work is inspired by Einar Saukas' ZX0 (https://github.com/einar-saukas/ZX0).

		xor	a
		push	af		; Push dummy value onto the stack.
		ld	hl,SrcAddr
		ld	de,DstAddr

DecodeLoop	call	EliasGamma1
		lddr
		rla
		jr	nc,NewOffset

		call	EliasGamma1

RepOffset	ex	(sp),hl
		push	hl
		add	hl,de
		lddr
		pop	hl
		ex	(sp),hl
		rla
		jr	c,DecodeLoop

NewOffset	pop	bc
		call	EliasGamma2
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
		call	EliasGamma2
		inc	bc
		jr	RepOffset

EliasGamma1	or	a
EliasGamma2	ld	bc,1
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
