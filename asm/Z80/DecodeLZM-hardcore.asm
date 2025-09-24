; Copyright (c) 2022, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse LZM "hardcore" decoder (23 bytes with setup, 18 bytes excluding setup).

; This decoder is optimized for the ZX Spectrum and operates under the following assumptions:

; 1) The compressed stream is placed immediately above the entry point.
; 2) The end-of-stream marker is omitted, and the final block overwrites opcodes after LDDR.
; 3) The program is launched from BASIC with a start address of #XX80, ensuring C = 0.

		ld	de,DstAddr
		push	bc
		ld	b,c

DecodeLoop1	pop	hl
		dec	hl
DecodeLoop2	ld	c,(hl)
		dec	hl
		srl	c
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
CopyBytes	lddr
		jr	nc,DecodeLoop1
		jr	DecodeLoop2
