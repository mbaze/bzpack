; Copyright (c) 2017, 2022, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; LZS decoder (18 bytes excluding initialization).

; The decoder assumes reverse order. There's a possibility to omit the end-of-stream
; marker if we let the output stream overwrite opcodes just after LDDR.

IF 1
		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0		; Ideally some values should be "reused".
MainLoop	ld	c,(hl)
		dec	hl
		srl	c
;		ret	z		; Option to include the end-of-stream marker.
;		inc	c		; Option to increase length to 128.
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
CopyBytes	lddr
		jr	c,MainLoop
		pop	hl
		dec	hl
		jr	MainLoop
ELSE

; On Sinclair ZX Spectrum we can make a couple of assumptions. The USR function places
; its argument to BC therefore aligning the address to 256 bytes initializes C to zero.
; Also HL can be initialized cheaply if we assume that the compressed stream is located
; just above the routine's entry point. Using pre-decrement rather than post-decrement
; makes it even possible to reuse POP HL. The decompressor is just 23 bytes long.

		ld	de,DstAddr
		push	bc
		ld	b,c
MainLoop1	pop	hl
		dec	hl
MainLoop2	ld	c,(hl)
		dec	hl
		srl	c
;		ret	z		; Option to include the end-of-stream marker.
;		inc	c		; Option to increase length to 128.
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
CopyBytes	lddr
		jr	nc,MainLoop1
		jr	MainLoop2
ENDIF
