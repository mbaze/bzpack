; Copyright (c) 2021, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; UE2 decoder (35 bytes excluding initialization).

; The decoder assumes reverse order. There's a possibility to omit the end-of-stream
; marker if we let the output stream overwrite opcodes just after LDDR.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0		; Ideally, these values should be "reused"
		ld	a,%10000000	; e.g. by aligning the addresses.

MainLoop	ld	c,1
		call	ReadBit		; Literal?
		jr	c,CopyBytes

EliasGamma	call	ReadBit
		rl	c
;		ret	c		; Option to include the end of stream marker.
		call	ReadBit
		jr	c,EliasGamma

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

ReadBit		add	a,a
		ret	nz
		ld	a,(hl)
		dec	hl
		rla
		ret
