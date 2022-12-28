;
;
; Beginnings of a simple stdio library for xtal
; ----


CMD_PUT_REC 	= $09   	; Send until EOL
CMD_PUT_BYTES	= $0B		; Put n bytes

EOL				= $9B		; ATASCII end-of-line

CIOV 			= $E456     ;	CIO VECTOR

ICHID   		= $0340     ;   Set by OS. Handler Identifier. If not
        		            ;   in use, the value is 255 ($FF), which
        		            ;   is also the initialization value.
ICDNO   		= ICHID+1   ;   Set by OS. Device number (eg: D1: D2:).
ICCOM   		= ICHID+2   ;   Set by User. Command
ICSTA   		= ICHID+3   ;   Set by OS. May or may not be the same
        		            ;   value as CMD_STATUS returned
ICBAL   		= ICHID+4   ;   Set by User. Buffer address (low byte)
ICBAH   		= ICHID+5   ;   Set by User. buffer address (high byte)
ICPTL   		= ICHID+6   ;   Used by BASIC. Address of put byte routine.
ICPTH   		= ICHID+7   ;   Used by BASIC. Address of put byte routine.
ICBLL   		= ICHID+8   ;   buffer length (low byte) in put/get ops
ICBLH   		= ICHID+9   ;   buffer length (high byte)
ICAX1   		= ICHID+10  ;   auxiliary information.  Used by Open


; ---------------------------------------------------------------------------
; Print a line until we get the terminating EOL char, and append the newline
; at the end

.function printLine
.clobber x,a
@printLine:
	ldx #0					; just use the default output device
	lda f0					; Get the low byte of the address to print..
	sta ICBAL,x				; and tell CIO
	lda f0+1				; Get the high byte of the address..
	sta ICBAH,x				; and tell CIO
	lda #CMD_PUT_REC		; set command to "print to end of line"
	sta ICCOM,x				; and tell CIO
	lda #0					; set low byte of buffer length...
	sta ICBLL,x				; and tell CIO
	lda #$ff				; set high-byte of buffer length
	sta ICBLH,x				; and tell CIO
	jsr CIOV				; Jump to the print-to-EOL routine
	clc
	rts

.endfunction


; ---------------------------------------------------------------------------
; More general print routine that gets the length of the string and only
; prints those chars. This means we don't have to have the newline

.function printStr
.clobber x,a,y
@printStr:
	
	; Set up the IOCB parameters
	
	ldx #0					; just use the default output device
	lda f0					; Get the low byte of the address to print..
	sta ICBAL,x				; and tell CIO
	lda f0+1				; Get the high byte of the address..
	sta ICBAH,x				; and tell CIO
	lda #CMD_PUT_BYTES		; set command to "print to end of line"
	sta ICCOM,x				; and tell CIO
	lda #0					; Zero out the high-byte of ..
	sta ICBLH,x				; the string to print
	
	; Find the length of the string
	
	ldy #$0					; Start of the string
find0:
	lda (f0),y				; Read the indexed byte from memory
	beq found0				; If its a zero, we're done
	iny						; Increment Y
	bne find0				; Not, so go for the next character
	lda ICBLH,x				; Get the high-byte of the length
	clc						; Prepare to add
	adc #1					; Increment it
	sta ICBLH,x				; And store it
	inc f0+1				; Also increase the pointer
	clc						; Just to be sure
	bcc find0				; And branch directly to the next character
	
	; update the length and call CIOV
	
found0:
	tya 					; Transfer the size of the string to A
	sta ICBLL,x				; and tell CIO
	jsr CIOV				; Jump to the print-to-EOL routine
	clc
	rts

.endfunction
	


; ---------------------------------------------------------------------------
; Convert a 32-bit number to an ASCII string. Designed to be called as
; a function, so expects to see the argument in f0

.function printReg
.clobber a,x,y,f0,s0

@printReg:
		ldy #0					; Counter for
		
		bit f0+3				; check to see if the number is -ve
		bpl notNegative			; if not, skip showing the '-'
		lda #$2D				; a literal '-' sign
		sta ascii,y				; and place it at the start
		iny						; bump the store-index
		_neg32 f0,f0			; convert to a +ve number
		
	notNegative:
		sty s0					; preserve where we are
		ldy #0					; number-of-digits count
	
	nextPass:
		ldx #32					; number of bits to iterate over
		lda #0					; zero the accumulator
	
	doShift:
		_asl32 f0,f0			; shift the bits left...
		rol a					; into the accumulator
		cmp #10					; and see if we are < 10
		bcc lessThan10			; skip if so, else
		sbc #10					; subtract 10, and
		inc f0					; and set a result bit
	
	lessThan10:
		dex						; repeat..
		bne doShift				; 32 times
		pha						; save digit
		iny						; increment the number of digits
		lda f0+3				; check if..
		ora f0+2				; the number..
		ora f0+1				; is zero
		ora	f0					; and ..
		bne nextPass			; repeat if not
		
		tya						; transfer
		tax						; Y to X
		ldy s0					; fetch where we were in the string
	
	showDigit:
		pla						; pop a digit off the stack
		clc						; convert it ..
		adc #$30				; to ascii
		sta ascii,y				; and write to the string
		iny						; next position in the string
		dex						; get next ..
		bne showDigit			; digit, else
		lda #EOL				; add an ATASCII end-of-line
		sta ascii,y				; and write to the string
	
		_movi16 ascii,f0		; put the address of the string into f0
		exec printLine 			; and print the string
		rts
		
ascii:
.word 0,0,0,0,0,0,0,0,0,0

		
.endfunction
