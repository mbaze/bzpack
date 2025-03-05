; Copyright (c) 2022, Milos "baze" Bazelides
; This code is licensed under the BSD 2-Clause License.

; Reverse LZS "hardcore" decoder (23 byte with setup, 18 bytes excluding setup).

; This decoder is optimized for the Sinclair ZX Spectrum and operates under the following
; assumptions:

; 1) The program is launched from BASIC using USR, with a start address of #XX00,  
;    ensuring that register C is set to 0.  
; 2) The compressed stream is located immediately above the entry point.  
; 3) There's no end-of-stream marker. The last block overwrites opcodes after LDDR.  

		ld	de,DstAddr
		push	bc
		ld	b,c
MainLoop1	pop	hl
		dec	hl
MainLoop2	ld	c,(hl)
		dec	hl
		srl	c
		jr	c,CopyBytes
		push	hl
		ld	l,(hl)
		ld	h,b
		add	hl,de
CopyBytes	lddr
		jr	nc,MainLoop1
		jr	MainLoop2
