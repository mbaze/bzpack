; Copyright (c) 2021, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse UE2 decoder (44 bytes with setup, 36 bytes excluding setup).

; The end-of-stream marker can be omitted if the output stream overwrites opcodes  
; immediately after LDDR.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	a,%10000000

MainLoop	ld	c,1
		call	ReadBit
		jr	c,CopyBytes

EliasGamma	call	ReadBit
		rl	c
;		ret	c		; Option to include the end-of-stream marker.
		call	ReadBit
		jr	c,EliasGamma

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

ReadBit		add	a,a
		ret	nz
		ld	b,a
		ld	a,(hl)
		dec	hl
		rla
		ret
