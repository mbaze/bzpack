; Copyright (c) 2017, Milos "baze" Bazelides
; This code is released under the terms of the BSD 2-Clause License.

; LZS decoder (18 bytes excluding initialization).

; The decoder assumes reverse order. The end of stream marker can be omitted
; if we let the last literal overwrite opcodes after LDDR and let the code continue.

IF 0
		ld	hl,SrcAddr
		ld	de,DstAddr
		ld	b,0		; Ideally this value should be "reused".

MainLoop	ld	c,(hl)
		dec	hl
		srl	c
;		ret	z		; Option to include the end of stream marker.
;		inc	c		; Option to increase length to 128.
		jr	c,CopyBytes
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
ELSE

; On Sinclair ZX Spectrum we can make a couple of assumptions. If the start address is #XX00
; the USR function will set BC accordingly. Also, if the input stream is placed directly
; before the entry point we can make the decompressor only 24 bytes long using pre-decrement.

		ld	h,b
		ld	l,c
		ld	b,c
		ld	de,DstAddr

MainLoop	dec	hl
		ld	c,(hl)
		srl	c
;		ret	z		; Option to include the end of stream marker.
;		inc	c		; Option to increase length to 128.
		jr	c,CopyBytes
		dec	hl
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
;		inc	hl		; Option to increase offset to 256.
CopyBytes	lddr
		jr	c,MainLoop
		pop	hl
		jr	MainLoop
ENDIF
