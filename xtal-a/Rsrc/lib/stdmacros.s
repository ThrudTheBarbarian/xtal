;
; Most of this is taken from Andrew Jacobs' 6502 standard macro library
; Sadly, Andrew passed away in January 2021. I hope using this is not
; a problem for whoever owns the copyright now.
;

;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Implement CLC,ADC as _add
;|*
;|* Clobbers: A
;|* Arguments: operand to add
;\*************************************************************************/
.macro _add
	clc
	adc %1
.endmacro

;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Clear 2 bytes of memory, the address pointed to by the argument, and the
;|* subsequent address
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : Address of first memory location
;\*************************************************************************/
.macro _clr16
	LDA #0
	STA %1
	STA %1+1
.endmacro


;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Clear 4 bytes of memory, the address pointed to by the argument, and the
;|* subsequent address
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : Address of first memory location
;\*************************************************************************/
.macro _clr32
	LDA #0
	STA %1
	STA %1+1
	STA %1+2
	STA %1+3
.endmacro


;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Transfer two bytes from one memory location to another. The order in
;|* which they are moved depends on the relative positions of src and dst.
;|* If src and dst are the same, then no code is generated
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address
;|*    %2 : destination address
;\*************************************************************************/
.macro _xfer16
	.if (%1 != %2)
		.if (%1 > %2)
			lda %1
			sta %2
			lda %1+1
			sta %2+1
		.else
			lda %1+1
			sta %2+1
			lda %1
			sta %2
		.endif
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Transfer four bytes from one memory location to another. The order in
;|* which they are moved depends on the relative positions of src and dst.
;|* If src and dst are the same, then no code is generated
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address
;|*    %2 : destination address
;\*************************************************************************/
.macro _xfer32
	.if (%1 != %2)
		.if (%1 > %2)
			lda %1
			sta %2
			lda %1+1
			sta %2+1
			lda %1+2
			sta %2+2
			lda %1+3
			sta %2+3
		.else
			lda %1+3
			sta %2+3
			lda %1+2
			sta %2+2
			lda %1+1
			sta %2+1
			lda %1
			sta %2
		.endif
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Set the value of a 16-bit location dst with the given constant value
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : constant 16-bit value
;|*    %2 : destination address
;\*************************************************************************/
.macro _set16i
	.if (%1 != 0)
		lda #<%1
		sta %2
		lda #>%1
		sta %2+1
	.else
		_clr32 %2
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical NOT of the 16-bit value at location %1 and
;|* store it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : src address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _not16
	lda %1
	eor #$FF
	sta %2
	lda %1+1
	eor #$FF
	sta %2+1
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical NOT of the 16-bit value at location %1 and
;|* store it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : src address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _not32
	lda %1
	eor #$FF
	sta %2
	lda %1+1
	eor #$FF
	sta %2+1
	lda %1+2
	eor #$FF
	sta %2+2
	lda %1+3
	eor #$FF
	sta %2+3
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical OR of the 16-bit value at location %1 and %2
;|* storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _or16
	.if (%1 != %2)
		lda %1
		ora %2
		sta %3
		lda %1+1
		ora %2+1
		sta %3+1
	.else
		_xfer16 %1, %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical OR of the 16-bit value at location %1 with an
;|* immediate value %2, storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : immediate value to or with
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _or16i
	lda %1
	ora #<%2
	sta %3
	lda %1+1
	ora #>%2
	sta %3+1
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical OR of the 32-bit value at location %1 and %2
;|* storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _or32
	.if (%1 != %2)
		lda %1
		ora %2
		sta %3
		lda %1+1
		ora %2+1
		sta %3+1
		lda %1+2
		ora %2+2
		sta %3+2
		lda %1+3
		ora %2+3
		sta %3+3
	.else
		_xfer32 %1, %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical AND of the 16-bit value at location %1 and %2
;|* storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _and16
	.if (%1 != %2)
		lda %1
		and %2
		sta %3
		lda %1+1
		and %2+1
		sta %3+1
	.else
		_xfer16 %1, %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical AND of the 16-bit value at location %1 with an
;|* immediate value %2, storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : immediate value to or with
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _and16i
	lda %1
	and #<%2
	sta %3
	lda %1+1
	and #>%2
	sta %3+1
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical AND of the 32-bit value at location %1 and %2
;|* storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _and32
	.if (%1 != %2)
		lda %1
		and %2
		sta %3
		lda %1+1
		and %2+1
		sta %3+1
		lda %1+2
		and %2+2
		sta %3+2
		lda %1+3
		and %2+3
		sta %3+3
	.else
		_xfer32 %1, %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical EOR of the 16-bit value at location %1 and %2
;|* storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _eor16
	.if (%1 != %2)
		lda %1
		eor %2
		sta %3
		lda %1+1
		eor %2+1
		sta %3+1
	.else
		_clr16 %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical AND of the 16-bit value at location %1 with an
;|* immediate value %2, storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : immediate value to or with
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _eor16i
	lda %1
	eor #<%2
	sta %3
	lda %1+1
	eor #>%2
	sta %3+1
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical AND of the 32-bit value at location %1 and %2
;|* storing it at %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from
;|*    %3 : destination address to write to
;\*************************************************************************/
.macro _eor32
	.if (%1 != %2)
		lda %1
		eor %2
		sta %3
		lda %1+1
		eor %2+1
		sta %3+1
		lda %1+2
		eor %2+2
		sta %3+2
		lda %1+3
		eor %2+3
		sta %3+3
	.else
		_clr32 %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Perform an arithmetic shift left on the 16 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _asl16
	.if (%1 != %2)
		lda %1
		asl a
		sta %2
		lda %1+1
		rol a
		sta %2+1
	.else
		asl %1
		rol %1+1
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Perform an arithmetic shift left on the 32 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _asl32
	.if (%1 != %2)
		lda %1
		asl a
		sta %2
		lda %1+1
		rol a
		sta %2+1
		lda %1+2
		rol a
		sta %2+2
		lda %1+3
		rol a
		sta %2+3
	.else
		asl %1
		rol %1+1
		rol %1+2
		rol %1+3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Perform a left rotation on the 16 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _rol16
	.if (%1 != %2)
		lda %1
		rol a
		sta %2
		lda %1+1
		rol a
		sta %2+1
	.else
		rol %1
		rol %1+1
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Perform a left rotation on the 32 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _rol32
	.if (%1 != %2)
		lda %1
		rol a
		sta %2
		lda %1+1
		rol a
		sta %2+1
		lda %1+2
		rol a
		sta %2+2
		lda %1+3
		rol a
		sta %2+3
	.else
		rol %1
		rol %1+1
		rol %1+2
		rol %1+3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Perform a logical shift right on the 16 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _lsr16
	.if (%1 != %2)
		lda %1+1
		lsr a
		sta %2+1
		lda %1
		ror a
		sta %2
	.else
		lsr %1+1
		ror %1
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Perform a logical shift right on the 32 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _lsr32
	.if (%1 != %2)
		lda %1+3
		lsr a
		sta %2+3
		lda %1+2
		ror a
		sta %2+2
		lda %1+1
		ror a
		sta %2+1
		lda %1
		ror a
		sta %2
	.else
		lsr %1+3
		ror %1+2
		ror %1+1
		ror %1
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Perform a rotate-right on the 16 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _ror16
	.if (%1 != %2)
		lda %1+1
		ror a
		sta %2+1
		lda %1
		ror a
		sta %2
	.else
		ror %1+1
		ror %1
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Increment (by one) the 16-bit value at location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : source address to increment
;\*************************************************************************/
.macro _ror32
	.if (%1 != %2)
		lda %1+3
		ror a
		sta %2+3
		lda %1+2
		ror a
		sta %2+2
		lda %1+1
		ror a
		sta %2+1
		lda %1
		ror a
		sta %2
	.else
		ror %1+3
		ror %1+2
		ror %1+1
		ror %1
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Increment (by one) the 16-bit value at location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : source address to increment
;\*************************************************************************/
.macro _inc16
	inc %1
	bne done_?
	inc %1+1
done_?:
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Increment (by one) the 32-bit value at location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : source address to increment
;\*************************************************************************/
.macro _inc32
	inc %1
	bne done_?
	inc %1+1
	bne done_?
	inc %1+2
	bne done_?
	inc %1+3
done_?:
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Decrement (by one) the 16-bit value at location %1
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to increment
;\*************************************************************************/
.macro _dec16
	lda %1
	bne done_?
	dec %1+1
done_?:
	dec %1
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Decrement (by one) the 32-bit value at location %1
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to increment
;\*************************************************************************/
.macro _dec32
	lda %1
	bne dec0_?
	lda %1+1
	bne dec1_?
	lda %1+2
	bne dec2_?
	dec %1+3
dec2_?:
	dec %1+2
dec1_?:
	dec %1+1
dec0_?:
	dec %1
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* add the 16-bit value at location %1 to the 16-bit value at %2, storing
;|* the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _add16
	.if %1 != %2
		clc
        lda %1
       	adc %2
       	sta %3
       	lda %1+1
       	adc %2+1
       	sta %3+1
	.else
       _asl16 %1, %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* add the 32-bit value at location %1 to the 32-bit value at %2, storing
;|* the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _add32
	.if %1 != %2
		clc
        lda %1
       	adc %2
       	sta %3
       	lda %1+1
       	adc %2+1
       	sta %3+1
       	lda %1+2
       	adc %2+2
       	sta %3+2
       	lda %1+3
       	adc %2+3
       	sta %3+3
	.else
       _asl32 %1, %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* subtract the 16-bit value at location %1 to the 16-bit value at %2,
;|* storing the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _sub16
		sec
        lda %1
       	sbc %2
       	sta %3
       	lda %1+1
       	sbc %2+1
       	sta %3+1
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* subtract the 16-bit value at location %1 to the 16-bit value at %2,
;|* storing the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _sub32
		sec
        lda %1
       	sbc %2
       	sta %3
       	lda %1+1
       	sbc %2+1
       	sta %3+1
       	lda %1+2
       	sbc %2+2
       	sta %3+2
       	lda %1+3
       	sbc %2+3
       	sta %3+3
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* negate the signed 16-bit number at location %1 and store the result at
;|* location %2. Both %1 and %2 can be the same
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of destination operand. Can be same as %1
;\*************************************************************************/
.macro _neg16
		sec
        lda #0
        sbc %1
       	sta %2
       	lda #0
       	sbc %1+1
       	sta %2+1
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* negate the signed 32-bit number at location %1 and store the result at
;|* location %2. Both %1 and %2 can be the same
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of destination operand. Can be same as %1
;\*************************************************************************/
.macro _neg32
		sec
        lda #0
        sbc %1
       	sta %2
       	lda #0
       	sbc %1+1
       	sta %2+1
       	lda #0
       	sbc %1+2
       	sta %2+2
       	lda #0
       	sbc %1+3
       	sta %2+3
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* calculate the absolute value of a signed 16-bit number at location %1
;|* and stores it in %2. Less code uis generated if %1 == %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand
;|*    %2 : address of destination operand. Can be same as %1
;\*************************************************************************/
.macro _abs16
		bit %1
        .if (%1 != %2)
			bpl move_?
			_neg16 %1, %2
			jmp done_?
	move_?:
			_xfer16 %1, %2
		.else
			bpl done_?
			_neg16 %1, %2
		.endif
	done_?:
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* calculate the absolute value of a signed 32-bit number at location %1
;|* and stores it in %2. Less code is generated if %1 == %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand
;|*    %2 : address of destination operand. Can be same as %1
;\*************************************************************************/
.macro _abs32
		bit %1
        .if (%1 != %2)
			bpl move_?
			_neg32 %1, %2
			jmp done_?
	move_?:
			_xfer32 %1, %2
		.else
			bpl done_?
			_neg32 %1, %2
		.endif
	done_?:
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* calculate the 16 bit product of two 16 bit unsigned numbers. Any
;|* overflow during calculation is lost. The value at %1 is destroyed
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand.
;\*************************************************************************/
.macro _mulu16
		_clr16 %3
		ldx #16
	loop_?:
		_asl16 %3,%3
		_asl16 %1,%1
		bcc next_?
		_add16 %2, %3, %3
	next_?:
		dex
		bpl loop_?
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Multiply two 16-bit signed numbers. Do this by checking the sign of the
;|* high bytes, and if they differ, then remember to convert the result back
;|* to negative. Then convery both operands to +ve if necessary, and run
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand.
;\*************************************************************************/
.macro _mul16
		lda %1				; compute the EOR of high-bits of both numbers
		eor %2
		php					; and store for later
		
		lda %1				; is num1 negative ?
		bpl check2_?		; +ve, so nothing to do here
		_neg16 %1,%1		; convert to +ve
	
	check2_?:
		lda %2				; is num2 negative
		bpl doMul16_?		; +ve, so nothing to do here
		_neg16 %2,%2
	
	doMul16_?
		_mulu16 %1, %2, %3	; Do an unsigned multiply
	
		plp					; Pull the saved state
		bpl done_?			; don't need to convert back to -ve
		_neg16 %3, %3
	
	done_?:
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* calculate the 32 bit product of two 16 bit unsigned numbers. The
;|* value at %1 is destroyed
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand.
;\*************************************************************************/
.macro _mul16x
		_clr32 %3
		ldx #16
	loop_?:
		_asl32 %3,%3
		_asl32 %1,%1
		bcc next_?
		_add16 %2, %3, %3
		bcc next_?
		_inc16 %3+2
	next_?:
		dex
		bpl loop_?
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* calculate the 32 bit product of two 32 bit unsigned numbers. The
;|* value at %1 is destroyed. Any overflow is lost
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand.
;\*************************************************************************/
.macro _mulu32
		_clr32 %3
		ldx #32
	loop_?:
		_asl32 %3,%3
		_asl32 %1,%1
		bcc next_?
		_add32 %2, %3, %3
		bcc next_?
		_inc16 %3+2
	next_?:
		dex
		bpl loop_?
.endmacro



;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Multiply two 32-bit signed numbers. Do this by checking the sign of the
;|* high bytes, and if they differ, then remember to convert the result back
;|* to negative. Then convery both operands to +ve if necessary, and run
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand.
;\*************************************************************************/
.macro _mul16
		lda %1				; compute the EOR of high-bits of both numbers
		eor %2
		php					; and store for later
		
		lda %1				; is num1 negative ?
		bpl check2_?		; +ve, so nothing to do here
		_neg32 %1,%1		; convert to +ve
	
	check2_?:
		lda %2				; is num2 negative
		bpl doMul16_?		; +ve, so nothing to do here
		_neg32 %2,%2
	
	doMul16_?
		_mulu32 %1, %2, %3	; Do an unsigned multiply
	
		plp					; Pull the saved state
		bpl done_?			; don't need to convert back to -ve
		_neg32 %3, %3
	
	done_?:
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Multiply a 16-bit value at location %1 by a 16-bit constant %2 and
;|* store in location %3
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : address of source operand
;|*    %2 : address of destination operand. Can be same as %1
;\*************************************************************************/
.macro _mul16i
	.if (%2 == 1)
		_xfer16 %1, %3
	.else
		_clr16 %3
		__mul16i %1, %2, %3
	.endif
.endmacro

.macro __mul16i
	.if (%2 & $FFFE)
		__mul16i %1, (%2/2), %3
		_asl16 %3, %3
	.endif
	.if (%2 & $0001)
		_add16 %1, %3, %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide a 16-bit value at location %1 by a 16-bit value %2 and
;|* store 16-bit result in location %3 and the 16-bit remainder in %4
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : location of dividend
;|*    %2 : location of divisor
;|*    %3 : location of quotient
;|*	   %4 : location of remainder
;\*************************************************************************/
.macro _div16
		_clr16 %4
		ldx #16
	loop_?:
		_asl16 %1, %1
		_rol16 %4, %4
		_sub16 %4, %2, %4
		bcs next_?
		_add16 %4, %2, %4
	next_?:
		_rol16 %3, %3
		dex
		bpl loop_?
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide a 32-bit value at location %1 by a 16-bit value %2 and
;|* store 16-bit result in location %3 and the 16-bit remainder in %4
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : location of dividend
;|*    %2 : location of divisor
;|*    %3 : location of quotient
;|*	   %4 : location of remainder
;\*************************************************************************/
.macro _div16x
		_clr16 %4
		ldx #32
	loop_?:
		_asl32 %1, %1
		_rol16 %4, %4
		_sub16 %4, %2, %4
		bcs next_?
		_add16 %4, %2, %4
	next_?:
		_rol16 %3, %3
		dex
		bpl loop_?
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide a 32-bit value at location %1 by a 32-bit value %2 and
;|* store 32-bit result in location %3 and the 32-bit remainder in %4
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : location of dividend
;|*    %2 : location of divisor
;|*    %3 : location of quotient
;|*	   %4 : location of remainder
;\*************************************************************************/
.macro _div32
		_clr32 %4
		ldx #32
	loop_?:
		_asl32 %1, %1
		_rol32 %4, %4
		_sub32 %4, %2, %4
		bcs next_?
		_add32 %4, %2, %4
	next_?:
		_rol32 %3, %3
		dex
		bpl loop_?
.endmacro



;/*************************************************************************\
;|* Type: Comparison operation
;|*
;|* Compare a 16-bit quantity at %1 to another at %2. Returns as soon as a
;|* difference is found
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : location of value 1
;|*    %2 : location of value 2
;\*************************************************************************/
.macro _cmp16
		lda %1+1
		cmp %2+1
		bne done_?
		lda %1
		cmp %2
	done_?:
.endmacro


;/*************************************************************************\
;|* Type: Comparison operation
;|*
;|* Compare a 32-bit quantity at %1 to another at %2. Returns as soon as a
;|* difference is found
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : location of value 1
;|*    %2 : location of value 2
;\*************************************************************************/
.macro _cmp32
		lda %1+3
		cmp %2+3
		bne done_?
		lda %1+2
		cmp %2+2
		bne done_?
		lda %1+1
		cmp %2+1
		bne done_?
		lda %1
		cmp %2
	done_?:
.endmacro




;/*************************************************************************\
;|* Type: Memory operation
;|*
;|* Copy %3 bytes from src (%1) to dst (%2) starting at the beginning of
;|* the block of bytes
;|*
;|* Clobbers: A,X,Y
;|* Arguments:
;|*    %1 : location of the block source-address, 2-byte ZP vector
;|*    %2 : location of the block destination address, 2-byte ZP vector
;|*	   %3 : number of bytes to copy
;\*************************************************************************/
.macro _memcpyUp
		ldy #0
		ldx %3+1
		beq frag_?
	page_?:
		lda (%1),Y
		sta (%2),Y
		iny
		bne page_?
		inc %1+1
		inc %2+1
		dex
		bne page_?
	frag_?:
		beq done_?
		lda (%1),Y
		sta (%1),Y
		iny
		bne frag_?
	done_?:
.endmacro


;/*************************************************************************\
;|* Type: Memory operation
;|*
;|* Copy %3 bytes from src (%1) to dst (%2) starting at the end of the
;|* block of bytes
;|*
;|* Clobbers: A,X,Y
;|* Arguments:
;|*    %1 : location of the block source-address, 2-byte ZP vector
;|*    %2 : location of the block destination address, 2-byte ZP vector
;|*	   %3 : number of bytes to copy
;\*************************************************************************/
.macro _memcpyDn
		
		lda %1+1				; Point to the last value to copy, because
		_add %3+1				; we're copying downwards not upwards
		sta %1+1
		
		lda %2+1
		_add %3+1
		sta %2+1
		
		ldy %3					; Handle fractions of a page-size first
		bne entry_?				; Got to copy the fragment
		beq	copyPage_?			; Start copying pages
		
	copyByte_?:
		lda (%1),y
		sta (%2),y
	
	entry_?:
		dey
		bne copyByte_?
		lda (%1),y				; copy the remaining byte
		sta (%2),y
	
	copyPage_?:
		ldx %3+1
		beq done_?
	
	initBase_?:
		dec %1+1
		dec %2+1
		dey
	
	copyBytes_?:
		lda (%1),y				; unroll the loop to make it a little
		sta (%1),y				; faster. Note that we unroll by 3
		dey						; because 255/3 = 85 remainder 0
		lda (%1),y
		sta (%1),y
		dey
		lda (%1),y
		sta (%1),y
		dey

	copyEntry_?:
		bne copyBytes_?
		lda (%1),y
		sta (%1),y
		dex
		bne initBase_?
.endmacro


;/*************************************************************************\
;|* Type: Memory operation
;|*
;|* Handle memory copy. Period.
;|*
;|* Clobbers: A,X,Y
;|* Arguments:
;|*    %1 : location of the block source-address, 2-byte ZP vector
;|*    %2 : location of the block destination address, 2-byte ZP vector
;|*	   %3 : number of bytes to copy
;\*************************************************************************/
.macro _memcpy
		_cmp16 %1, %2
		bcc safe_?
		_memcpyUp %1, %2, %3
		jmp done_?
	safe_?:
		_memcpyDn %1, %2, %3
	done_?:
.endmacro

