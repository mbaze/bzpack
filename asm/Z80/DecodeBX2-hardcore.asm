; Copyright (c) 2025, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse BX2 "hardcore" decoder (49 bytes with setup, 44 bytes excluding setup).

; This decoder is optimized for the ZX Spectrum and operates under the following assumptions,
; which are easily met in minimalist demoscene programs:

; 1) The compressed stream is placed immediately above the entry point.
; 2) The end-of-stream marker is omitted, and the final block overwrites opcodes after LDDR.
; 3) No literal exceeds 255 bytes, and no match exceeds 254 bytes.
; 4) The first block is at least 2 bytes long.
; 5) The program is launched from BASIC with a start address of #7F80, ensuring BC = #7F80,
;    and A = #80.

		ld	h,b
		ld	l,c
		ld	de,DestAddr

DecodeLoop	call	EliasGamma
		rla
		jr	nc,NewOffset

		lddr

		call	EliasGamma
		rla
		jr	c,RepOffset

NewOffset	dec	hl
		ex	af,af'
		ld	a,(hl)
		ex	af,af'
		inc	c

RepOffset	push	hl
		ex	af,af'
		ld	h,b
		ld	l,a
		ex	af,af'
		add	hl,de
		lddr
		pop	hl
		jr	DecodeLoop

EliasGamma	inc	c
EliasLoop	add	a,a
		jr	nz,NoFetch
		ld	b,a
		dec	hl
		sbc	a,(hl)
		rla
NoFetch		ret	nc
		add	a,a
		rl	c
		jr	EliasLoop
