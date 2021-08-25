; (c) 2021 Aleksey "introspec" Pichugin, Milos "baze" Bazelides and Pavel "Zilog" Cimbal.

; Block decoder with Elias-Gamma langths (1..N).

; The decoder assumes reverse order. If the last literal overwrites
; instructions after LDDR we can omit the end of stream marker.

DecodeUE1	ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0		; Note: Ideally, these values should be "reused"
		ld	a,%11000000	; e.g. by aligning the addresses.

DecodeLength	add	a,a
		rl	c
;		ret	c		; Option to include the end of stream marker.
NextBit		add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,DecodeLength

		rla			; Literal or phrase?
		jr	c,Copy

		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
		inc	c
Copy		lddr
		inc	c		; Prepare the most-significant bit.
		jr	c,NextBit
		pop	hl
		dec	hl
		jr	NextBit
