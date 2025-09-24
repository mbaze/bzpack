; Copyright (c) 2017, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse LZM decoder (27 bytes with setup, 19 bytes excluding setup).

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0

DecodeLoop	ld	c,(hl)
		dec	hl
		srl	c
		ret	z		; Option to include the end-of-stream marker.
;		inc	c		; Option to extend the block length.
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to extend the offset range.
CopyBytes	lddr
		jr	c,DecodeLoop
		pop	hl
		dec	hl
		jr	DecodeLoop
