; Copyright (c) 2021, Aleksey "introspec" Pichugin, Milos "baze" Bazelides, Pavel "Zilog" Cimbal
; This code is released under the terms of the BSD 2-Clause License.

; E1 decoder (28 bytes excluding initialization).

; The decoder assumes reverse order. There's a possibility to omit the end-of-stream
; marker if we let the output stream overwrite opcodes just after LDDR.

IF 1
		ld	a,%11000000	; Ideally, these values should be "reused".
		ld	bc,0		; Address alignment is one of the options.

		ld	hl,SrcAddr
		ld	de,DstAddr
EliasGamma	add	a,a
		rl	c
;		ret	c		; Option to include the end of stream marker.
NextBit		add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,EliasGamma
		rla			; Literal or phrase?
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
		inc	bc
CopyBytes	lddr
		inc	c		; Prepare the most-significant Elias-Gamma bit.
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
ELSE

; This alternative version contains "uglier" initialization of B inside the main loop.
; However, it increases the chance of being able to optimize the setup.

		ld	c,0		; Ideally, these values should be "reused"
		ld	a,%11000000	; e.g. by aligning the addresses.

		ld	hl,SrcAddr
		ld	de,DstAddr
EliasGamma	add	a,a
		rl	c
;		ret	c		; Option to include the end of stream marker.
NextBit		add	a,a
		jr	nz,NoFetch
		ld	b,a		; Initialize B to zero before the first use.
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,EliasGamma
		rla			; Literal or phrase?
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
		inc	bc
CopyBytes	lddr
		inc	c		; Prepare the most-significant Elias-Gamma bit.
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
ENDIF
