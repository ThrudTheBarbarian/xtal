;
; Convert a 32-bit number to an ASCII string. Uses:
;
; This code is heavily (like 95%) based on the code at
; http://www.6502.org/source/strings/32bit-to-ascii.html which carries the
; below copyright and distribution terms.
;
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;*                                                                             *
;*                CONVERT 32-BIT BINARY TO ASCII NUMBER STRING                 *
;*                                                                             *
;*                             by BigDumbDinosaur                              *
;*                                                                             *
;* This 6502 assembly language program converts a 32-bit unsigned binary value *
;* into a null-terminated ASCII string whose format may be in  binary,  octal, *
;* decimal or hexadecimal.                                                     *
;*                                                                             *
;* --------------------------------------------------------------------------- *
;*                                                                             *
;* Copyright (C)1985 by BCS Technology Limited.  All rights reserved.          *
;*                                                                             *
;* Permission is hereby granted to copy and redistribute this software,  prov- *
;* ided this copyright notice remains in the source code & proper  attribution *
;* is given.  Any redistribution, regardless of form, must be at no charge  to *
;* the end user.  This code MAY NOT be incorporated into any package  intended *
;* for sale unless written permission has been given by the copyright holder.  *
;*                                                                             *
;* THERE IS NO WARRANTY OF ANY KIND WITH THIS SOFTWARE.  It's free, so no mat- *
;* ter what, you're getting a great deal.                                      *
;*                                                                             *
;* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; Calling parameters
; ------------------
;
;	FN0.4 	: value to be converted to text
;	FN1.1	: radix character ($=hex, @=octal, %=bin, other=dec)
; 	FN2.2	: address of buffer to place string. Lengths (inc \0) required are:
;				Binary 	: 33 bytes
;				Octal	: 12 bytes
;				Decimal	: 11 bytes
;				Hex		: 9  bytes
;
; Return parameters
; -----------------
;
; 	FN0.1	: string length
;
;
;
; Constants
; ---------
;
a_hexdec	= 'A' - '9' - 2				; hex->decimal offset
m_bits		= #32						; operand bit size
m_cbits		= #48						; workspace bit size
m_strlen	= m_bits + 1				; max printable string length
n_radix		= #4						; number of supported radices
s_pfac		= m_bits / 8				; primary accumulator size
s_ptr		= #2						; pointer size
s_wrkspc	= m_cbits / 8				; conversion workspace size

;
; Zero-page assignments
; ---------------------
;
; Uses all of $B0..$BF + $AE,$AF
;

ptr01		= #$AA						; pointer to string buffer area
pfac		= #$B0						; primary accumulator
wrkspc01	= #$B4						; conversion workspace
wrkspc02	= #$BA						; workspace
formflag	= #$A7						; flag for which format to use
radix		= #$AF						; radix index
stridx		= #$AE						; string buffer index
		
;
; Entry point, and define a function if we are paging memory

.function printReg
.clobber a,x,y

	;
	; Set up the starting parameters
	;
	movm.4 fn0,	pfac					; copy value to workspace
	movm.1 #0,	stridx					; initialise string index
	
	;
	; evaluate the radix
	
	lda fn1.1							; fetch the radix from the parameter
	ldx	n_radix-1						; X = total number of radices

checkRadix:
	cmp radxtab, x						; Do we recognise this radix ?
	beq radixOk							; Yes, we are done
	dex									; Try the next one
	bne checkRadix

	stx radix							; Store the radix for later (0=decimal)
	txa									; Are we converting to decimal ?
	beq notDecimal						; Jump to the right place, or continue
	
	jsr convertToBCD					; Convert operand to BCD
	lda #0
	beq setupConversion					; And start converting
	
notDecimal:
	bit formflag						; Test if high-bit is set (= no symbol)
	bmi noSymbol
	
	lda radxtab, x						; Get the symbol to use
	ldy #0
	sta (FN2.2), y						; Prepend to the string
	inc stridx							; Increment position of pointer

noSymbol:
	ldx #0								; operand index
	ldy #s_wrkspc-1						; workspace index
	
copyBigEndian:
	lda pfac, x 						; copy operand to ...
	sta wrkspc01, y 					; workspace in ...
	dey									; big-endian order
	inx
	cpx #s_pfac
	bne copyBigEndian

	lda #0

padWorkspace:
	sta wrkspc01,y						; write 0 to the rest ...
	dey									; of the workspace
	bpl padWorkspace
	
	
setupConversion:
	sta wrkspc02         				; initialize byte counter
	ldy radix            				; radix index
	lda numstab,y        				; numerals in string
	sta wrkspc02+1       				; set remaining numeral count
	lda bitstab,y        				; bits per numeral
	sta wrkspc02+2       				; set
	lda lzsttab,y        				; leading zero threshold
	sta wrkspc02+3       				; set

generateConverion:
	lda #0
	ldy wrkspc02+2        				; bits per numeral

moreBits:
	ldx #s_wrkspc-1      				; workspace size
	clc                  				; avoid starting carry

shiftBit:
	rol wrkspc01,x        				; shift out a bit...
	dex                   				; from the operand or...
	bpl shiftBit          				; BCD conversion result
				  
    rol                  				; bit to .A
    dey
    bne moreBits         				; more bits to grab
				 
    tay                  				; if numeral isn't zero...
    bne checkRange         				; skip leading zero tests
				 
    ldx wrkspc02+1       				; remaining numerals
    cpx wrkspc02+3       				; leading zero threshold
    bcc checkRange         				; below it, must convert
				 
    ldx wrkspc02         				; processed byte count
    beq discardZeros       				; discard leading zero
						 
checkRange:
	cmp #10              				; check range
	bcc inRange         				; is 0-9
 
	adc #a_hexdec        				; apply hex adjust

inRange:
	adc #'0'              				; change to ASCII
    ldy stridx            				; string index
    sta (FN2.2), y						; save numeral in buffer
    inc stridx            				; next buffer position
    inc wrkspc02          				; bytes=bytes+1

discardZeros:
	dec wrkspc02+1       				; numerals=numerals-1
	bne generateConversion     			; not done

	;
	; Terminate the string with a \0
	;
    lda #0
    ldy stridx            				; printable string length
    sta (FN2.2), y          			; terminate string
    stx (FN0.1)							; Write string length to return reg
    clc                   				; all okay
    rts

;
;==============================================================================
;
;CONVERT PFAC INTO BCD
;
;	---------------------------------------------------------------
;	Uncomment noted instructions if this code is to be used  on  an
;	NMOS system whose interrupt handlers do not clear decimal mode.
;	---------------------------------------------------------------
;
convertToBCD:
		ldx #s_pfac-1         			;primary accumulator size -1

placeOnStack:
		lda pfac,x           			; value to be converted
		pha                  			; protect
		dex
		bpl placeOnStack				; next

		lda #0
		ldx #s_wrkspc-1       			; workspace size

clearWorkspaces:
		sta wrkspc01,x        			; clear final result
		sta wrkspc02,x        			; clear scratchpad
		dex
		bpl clearWorkspaces

    	inc wrkspc02+s_wrkspc-1
    	php                   			; NMOS doesn't respect decimal
    	sei                   			; mode through an interrupt
    	sed                  			; select decimal mode
    	ldy #m_bits-1        			; bits to convert -1

nextOperandBit:
		ldx #s_pfac-1         			; operand size
		clc                   			; no carry at start

nextLSBit:
		ror pfac,x            			; grab LS bit in operand
		dex
		bpl nextLSBit

		bcc LSBitClear					; LS bit clear

		clc
		ldx #s_wrkspc-1

partialResult:
		lda wrkspc01,x        			; partial result
		adc wrkspc02,x        			; scratchpad
		sta wrkspc01,x        			; new partial result
		dex
		bpl partialResult

		clc

LSBitClear:
		ldx #s_wrkspc-1

Scratchpad:
		lda wrkspc02,x        			; scratchpad
		adc wrkspc02,x        			; double &...
		sta wrkspc02,x        			; save
		dex
		bpl Scratchpad

		dey
		bpl nextOperandBit          	; next operand bit

		plp                   			; NMOS IRQ protection
		ldx #0

nextOperand:
		pla                   			; operand
         sta pfac,x            			; restore
         inx
         cpx #s_pfac
         bne nextOperand          		; next

         rts
			
;
; Data tables
;
radxtab:	.byte 0,'%','@','$'			; recognized radixes
numstab:	.byte 12, 48, 16, 12		; maximum numerals
bitstab  	.byte 4,1,3,4         		;bits per numeral
lzsttab  	.byte 2,9,2,3         		;leading zero suppression thresholds


