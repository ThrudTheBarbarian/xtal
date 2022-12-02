;
; Convert a 32-bit number to an ASCII string. Designed to be called as
; a function, so expects to see the argument in f0
;

.include stdio.s

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

