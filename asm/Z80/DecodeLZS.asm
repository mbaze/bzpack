; Copyright (c) 2017, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; Byte-aligned LZSS decoder.

; The decoder assumes reverse order. We can omit the end of stream
; marker if we let the last literal overwrite the code after LDDR.

		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0		; Ideally this value should be "reused".

MainLoop	ld	c,(hl)
		dec	hl
		srl	c
;		ret	z		; Option to include the end of stream marker.
;		inc	c		; Option to increase length to 128.
		jr	c,Copy
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
