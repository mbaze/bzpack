
; This routine adds only 7 bytes and zero additional bits to the stream.
; Assuming that there are no two literals in sequence we can interpret the flag as follows:

; 1) After literal there's a phrase either with standard offset 1..255 or with extended offset 1..511.
; 2) After phrase there can only be a literal or another phrase with standard offset.

DecodeElias	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,DecodeElias
		rla			; prepare flag
		dec	b		; B = 0 after phrase, B = 255 after literal
		jr	z,WasPhrase	; after phrase there can only be a literal or short offset
		inc	b		; reset B back to zero
		rl	b		; set B = 0 / 1 for short / extended offset
WasPhrase	jr	c,Copy		; it's a literal, jump with B = 0
LongOffset	push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
		inc	c
Copy		lddr
		inc	c
		jr	c,NextBit
		pop	hl
		dec	hl
		inc	b		; B = 1 means "it was a phrase"
		jr	NextBit

; Alternative shorter version, just +6 bytes. The meaning of short/long offset flag is flipped.

DecodeElias	add	a,a
		rl	c
NextBit		add	a,a
		jr	nz,NoFetch
		ld	a,(hl)
		dec	hl
		rla
NoFetch		jr	c,DecodeElias
		rla			; prepare flag
		djnz	WasLiteral
		jr	c,ShortOffset	; was phrase, carry means short offset, we have B = 0 at this point
WasLiteral	inc	b		; after literal set B back to 0, after long offset B = 1
		jr	c,Copy		; carry means another literal, no carry means short offset
ShortOffset	push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
		inc	c
Copy		lddr
		inc	c
		jr	c,NextBit
		pop	hl
		dec	hl
		inc	b		; B = 1 means "it was a phrase"
		jr	NextBit
