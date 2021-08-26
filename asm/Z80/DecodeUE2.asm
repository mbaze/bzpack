; Copyright (c) 2021, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; Unary decoder with Elias-Gamma langths (2..N).

; The decoder assumes reverse order. We can omit the end of stream
; marker if we let the last literal overwrite the code after LDDR.

DecodeUE1	ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0		; Note: Ideally, these values should be "reused"
		ld	a,%10000000	; e.g. by aligning the addresses.

MainLoop	ld	c,1
		call	ReadBit		; Literal?
		jr	c,Copy

DecodeLength	call	ReadBit
		rl	c
;		ret	c		; Option to include the end of stream marker.
		call	ReadBit
		jr	c,DecodeLength

		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
Copy		lddr
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
