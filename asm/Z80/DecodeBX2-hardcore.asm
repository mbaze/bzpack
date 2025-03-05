; Copyright (c) 2025, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse BX2 "hardcore" decoder (49 bytes with setup, 44 bytes excluding setup).

; This decoder is optimized for the Sinclair ZX Spectrum and operates under the following
; assumptions, which are easily met in minimalist demoscene programs:

; 1) The program is launched from BASIC using USR, with a start address of #7F80.
;    This ensures both A and C are set to 128, and B is initialized to 127.
; 2) The first block is a literal of at least two bytes in length.
; 3) No literal exceeds 255 bytes, and no match exceeds 254 bytes.
; 4) The compressed stream is placed immediately above the entry point.
; 5) There's no end-of-stream marker. The last block overwrites opcodes after LDDR.

		ld	de,DestAddr
		ld	h,b
		ld	l,b

DecodeLoop	call	EliasGamma
		rla
		jr	nc,NewOffset

		lddr

		call	EliasGamma
		rla
		jr	c,RepOffset

NewOffset	ex	af,af'
		ld	a,(hl)
		ex	af,af'
		dec	hl
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
		ld	a,(hl)
		dec	hl
		rla
NoFetch		ret	nc
		add	a,a
		rl	c
		jr	EliasLoop
