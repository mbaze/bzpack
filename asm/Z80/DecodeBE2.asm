; Copyright (c) 2021, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; Block decoder with Elias-Gamma langths (2..N).

; The decoder assumes reverse order. If the last literal overwrites
; instructions after LDDR we can omit the end of stream marker.

DecodeUE1	ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0		; Note: Ideally, these values should be "reused"
		ld	a,%10000000	; e.g. by aligning the addresses.

DecodeLength	ld	c,1
DecodeLoop	call	ReadBit
		rl	c
;		ret	c		; Option to include the end of stream marker.
		call	ReadBit
		jr	c,DecodeLoop

		dec	c
		call	ReadBit		; Literal or phrase?
		jr	c,Copy

		push	hl
		ld	l,(hl)
		ld	h,b
;		inc	hl		; Option to increase offset to 256.
		add	hl,de
		inc	c
Copy		lddr
		jr	c,DecodeLength
		pop	hl
		dec	hl
		jr	DecodeLength

ReadBit		add	a,a
		ret	nz
		ld	a,(hl)
		dec	hl
		rla
		ret
