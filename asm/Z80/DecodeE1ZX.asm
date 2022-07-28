; Copyright (c) 2022, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; E1ZX decoder (28 bytes excluding initialization, usually 35 bytes including initialization, 39 bytes max).

; The decoder assumes reverse order. End of stream marker is not supported.
; The last literal is expected to overwrite opcodes after LDDR.

IF 1
		ld	a,128		; Note that the ZX Spectrum's USR function sets A = C upon entry.
		ld	c,a		; If the start address is #XX80 both registers already equal 128.

		ld	hl,SrcAddr
		ld	de,DstAddr
EliasLength	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	b,a
		sub	(hl)		; Fetch and (except for very rare situations) set carry.
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
		inc	c		; Prepare the most-significant Elias bit.
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
ELSE

; If we use the USR #XX80 trick described above we can go one step further and place
; the compressed stream just before the entry point. This saves another byte because
; BC already contains the startup address. However, in this case we must pre-decrement
; rather than post-decrement the stream pointer. The decoder size is 34 bytes.

		ld	h,b
		ld	l,c
		ld	de,DstAddr
EliasLength	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	b,a
		dec	hl
		sub	(hl)		; Fetch and (except for very rare situations) set carry.
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
		inc	c		; Prepare the most-significant Elias bit.
		jr	c,NextBit
		pop	hl
		jr	NextBit
ENDIF
