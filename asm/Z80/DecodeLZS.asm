; Copyright (c) 2017, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse LZS decoder (26 bytes with setup, 18 bytes excluding setup).

; The end-of-stream marker can be omitted if the output stream overwrites opcodes  
; immediately after LDDR.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0		; Ideally, some values should be "reused".
MainLoop	ld	c,(hl)
		dec	hl
		srl	c
;		ret	z		; Option to include the end-of-stream marker.
;		inc	c		; Option to extend the block length.
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to extend the offset range.
CopyBytes	lddr
		jr	c,MainLoop
		pop	hl
		dec	hl
		jr	MainLoop
