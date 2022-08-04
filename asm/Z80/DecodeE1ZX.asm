; Copyright (c) 2022, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; E1ZX decoder (usually 34..36 bytes including initialization).

; The end of stream marker is not supported by this format. The program is expected
; to continue after the last literal overwrites opcodes just after LDDR.

IF 1

; On Sinclair ZX Spectrum the USR function places its argument to BC and sets A = C.
; Aligning the start address to #XX80 sets both A and C to 128 and simplifies setup.
; Normally we would need C = 0 but the most-significant bit gets shifted out anyway.
; We also no longer need A = 192 because the second ADD A,A is not expected to set
; Carry. This is now done by SUB (HL). We also "sneakily" initialize B during fetch.

		ld	hl,SrcAddr
		ld	de,DstAddr
EliasLength	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	b,a
		sub	(hl)		; Fetch and (except for rare situations) set carry.
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
		inc	c		; Prepare the most-significant Elias-Gamma bit.
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
ELSE

; If we use the USR #XX80 trick described above we can go one step further and place
; the compressed stream just before the entry point. This saves another byte because
; BC already contains the start address and we can initialize HL cheaply. However,
; we must pre-decrement rather than post-decrement HL. The routine is 34 bytes long.

		ld	h,b
		ld	l,c
		ld	de,DstAddr
EliasLength	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	b,a
		dec	hl
		sub	(hl)		; Fetch and (except for rare situations) set carry.
;		scf			; Only use in case of warning.
		rla
NoFetch		jr	c,EliasLength
		rla			; Literal or phrase?
		jr	c,CopyBytes
		dec	hl
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
		inc	c
CopyBytes	lddr
		inc	c		; Prepare the most-significant Elias-Gamma bit.
		jr	c,NextBit
		pop	hl
		jr	NextBit
ENDIF
