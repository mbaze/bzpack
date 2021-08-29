
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
		inc	b		; B = 0 after phrase, B = 1 after literal
		jr	z,WasPhrase	; after phrase there can only be a literal or short offset
		jr	c,LongOffset	; after literal there can only be a phrase with long / short offset
		dec	b		; short offset, set B = 0
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
		dec	b		; B = 255 means "it was a phrase"
		jr	NextBit
