; Copyright (c) 2021, Aleksey "introspec" Pichugin, Milos "baze" Bazelides, Pavel "Zilog" Cimbal
; This code is licensed under the BSD 2-Clause License.

; Reverse EF8 decoder (39 bytes with setup, 30 bytes excluding setup).

; If the decoder is launched from ZX Spectrum BASIC, the setup can be optimized by using
; the initial values of BC and A:

;		ld	h,b
;		ld	l,b
;		ld	de,DstAddr

; This reduces the decoder to 34 bytes if these assumptions are met:

; 1) The compressed stream is placed immediately above the entry point.
; 2) The end-of-stream marker is omitted, and the final block overwrites opcodes after LDDR.
; 3) The start address is either #7F80 (BC = #7F80, A = #80) or #BFC0 (BC = #BFC0, A = #C0).
;    For the latter, the first literal must be at least 2 bytes long.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	a,128
		ld	c,a

EliasGamma	add	a,a
		rl	c
		ret	z		; Option to include the end-of-stream marker.
DecodeLoop	add	a,a
		jr	nz,NoFetch
		ld	b,a
		sbc	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,EliasGamma
		rla
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to extend the offset range.
		inc	bc
CopyBytes	lddr
		inc	c
		jr	c,DecodeLoop
		pop	hl
		dec	hl
		jr	DecodeLoop
