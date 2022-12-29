;
; Most of this is taken from Andrew Jacobs' 6502 standard macro library
; Sadly, Andrew passed away in January 2021. I hope using this is not
; a problem for whoever owns the copyright now.
;

;/*************************************************************************\
;|* Type: Register defines
;|*
;|* Let "normal" 6502 opcodes access the register variables by using
;|* r0 .. r63. This isn't a perfect solution because it won't handle
;|* automatically mapping in the correct range of values if r>16, but
;|* that can be done manually
;\*************************************************************************/

f0	= $a0
f1	= $a1
f2	= $a2
f3	= $a3
f4	= $a4
f5	= $a5
f6	= $a6
f7 	= $a7
f8	= $a8
f9	= $a9
f10	= $aa
f11	= $ab
f12	= $ac
f13	= $ad
f14	= $ae
f15	= $af

s0	= $b0
s1	= $b1
s2	= $b2
s3	= $b3
s4	= $b4
s5	= $b5
s6	= $b6
s7 	= $b7
s8	= $b8
s9	= $b9
s10	= $ba
s11	= $bb
s12	= $bc
s13	= $bd
s14	= $be
s15	= $bf

r0	= $c0
r1	= $c1
r2	= $c2
r3	= $c3
r4	= $c4
r5	= $c5
r6	= $c6
r7 	= $c7
r8	= $c8
r9	= $c9
r10	= $ca
r11	= $cb
r12	= $cc
r13	= $cd
r14	= $ce
r15	= $cf

r16	= $d0
r17	= $d1
r18 = $d2
r19	= $d3
r20	= $d4
r21	= $d5
r22	= $d6
r23	= $d7
r24	= $d8
r25	= $d9
r26	= $da
r27	= $db
r28	= $dc
r29	= $dd
r30	= $de
r31	= $df

r32	= $e0
r33	= $e1
r34 = $e2
r35	= $e3
r36	= $e4
r37	= $e5
r38	= $e6
r39	= $e7
r40	= $e8
r41	= $e9
r42	= $ea
r43	= $eb
r44	= $ec
r45	= $ed
r46	= $ee
r47	= $ef

r48	= $f0
r49	= $f1
r50 = $f2
r51	= $f3
r52	= $f4
r53	= $f5
r54	= $f6
r55	= $f7
r56	= $f8
r57	= $f9
r58	= $fa
r59	= $fb
r60	= $fc
r61	= $fd
r62	= $fe
r63	= $ff

sp	= $90		; also $91

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
;|* Write a 16-bit number to a memory location in little-endian form
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 immediate value to store
;|*    %2 address to store it at
;\*************************************************************************/
.macro _movi16
	lda #>%1
	sta %2+1
	lda #<%1
	sta %2
.endmacro

;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Write a 32-bit number to a memory location in little-endian form
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 immediate value to store
;|*    %2 address to store it at
;\*************************************************************************/
.macro _movi32
	lda #>>>%1
	sta %2+3
	lda #>>%1
	sta %2+2
	lda #>%1
	sta %2+1
	lda #<%1
	sta %2
.endmacro


;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Clear 1 bytes of memory, the address pointed to by the argument
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : Address of first memory location
;\*************************************************************************/
.macro _clr8
	LDA #0
	STA %1
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
;|* Transfer a byte from one memory location to another.
;|* If src and dst are the same, then no code is generated
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address
;|*    %2 : destination address
;\*************************************************************************/
.macro _xfer8
	.if (%1 != %2)
		lda %1
		sta %2
	.endif
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
;|* Transfer a byte from one memory location pointer to a memory location.
;|*
;|* Clobbers: A,Y
;|* Arguments:
;|*    %1 : source address
;|*    %2 : destination address
;\*************************************************************************/
.macro _xferp8
		lda %1
		ldy #0
		sta (%2),y
.endmacro

;
; Variant - accept a third parameter as non-zero Y
.macro _xferpn8
		lda %1
		ldy #%3
		sta (%2),y
.endmacro

;
; Variant - transfer from the indirected vector to an absolute address
.macro _xferbn8
		ldy #%3
		lda (%2),y
		sta %1
.endmacro

;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Transfer 2 bytes from one memory location pointer to a memory location.
;|*
;|* Clobbers: A,Y
;|* Arguments:
;|*    %1 : source address
;|*    %2 : destination address
;\*************************************************************************/
.macro _xferp16
		lda %1
		ldy #0
		sta (%2),y
		lda %1+1
		iny
		sta (%2),y
.endmacro

;
; Variant - accept a third parameter as non-zero Y
.macro _xferpn16
		lda %1
		ldy #%3
		sta (%2),y
		lda %1+1
		iny
		sta (%2),y
.endmacro

;
; Variant - transfer from the indirected vector to an absolute address
.macro _xferbn16
		ldy #%3
		lda (%2),y
		sta %1
		iny
		lda (%2),y
		sta %1+1
.endmacro

;/*************************************************************************\
;|* Type: Basic operation
;|*
;|* Transfer 2 bytes from one memory location pointer to a memory location.
;|*
;|* Clobbers: A,Y
;|* Arguments:
;|*    %1 : source address
;|*    %2 : destination address
;\*************************************************************************/
.macro _xferp32
		lda %1
		ldy #0
		sta (%2),y
		lda %1+1
		iny
		sta (%2),y
		lda %1+2
		iny
		sta (%2),y
		lda %1+3
		iny
		sta (%2),y
.endmacro

;
; Variant - accept a third parameter as non-zero Y
.macro _xferpn32
		lda %1
		ldy #%3
		sta (%2),y
		lda %1+1
		iny
		sta (%2),y
		lda %1+2
		iny
		sta (%2),y
		lda %1+3
		iny
		sta (%2),y
.endmacro

;
; Variant - transfer from the indirected vector to an absolute address
.macro _xferbn32
		ldy #%3
		lda (%2),y
		sta %1
		iny
		lda (%2),y
		sta %1+1
		iny
		lda (%2),y
		sta %1+2
		iny
		lda (%2),y
		sta %1+3
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
;|* Calculate the logical OR of the 8-bit value at location %1 and %2
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, also destination
;\*************************************************************************/
.macro _or8
	.if (%1 != %2)
		lda %1
		ora %2
		sta %2
	.else
		_xfer8 %1, %2
	.endif
.endmacro

;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical OR of the 16-bit value at location %1 and %2
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, also destination
;\*************************************************************************/
.macro _or16
	.if (%1 != %2)
		lda %1
		ora %2
		sta %2
		lda %1+1
		ora %2+1
		sta %2+1
	.else
		_xfer16 %1, %2
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
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, also destination
;\*************************************************************************/
.macro _or32
	.if (%1 != %2)
		lda %1
		ora %2
		sta %2
		lda %1+1
		ora %2+1
		sta %2+1
		lda %1+2
		ora %2+2
		sta %2+2
		lda %1+3
		ora %2+3
		sta %2+3
	.else
		_xfer32 %1, %2
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical AND of the 16-bit value at location %1 and %2
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, also destination
;\*************************************************************************/
.macro _and8
	.if (%1 != %2)
		lda %1
		and %2
		sta %2
	.else
		_xfer8 %1, %2
	.endif
.endmacro

;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical AND of the 16-bit value at location %1 and %2
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, also destination
;\*************************************************************************/
.macro _and16
	.if (%1 != %2)
		lda %1
		and %2
		sta %2
		lda %1+1
		and %2+1
		sta %2+1
	.else
		_xfer16 %1, %2
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
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, also destination
;\*************************************************************************/
.macro _and32
	.if (%1 != %2)
		lda %1
		and %2
		sta %2
		lda %1+1
		and %2+1
		sta %2+1
		lda %1+2
		and %2+2
		sta %2+2
		lda %1+3
		and %2+3
		sta %2+3
	.else
		_xfer32 %1, %2
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical EOR of the 8-bit value at location %1 and %2
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, and destination
;\*************************************************************************/
.macro _eor8
	.if (%1 != %2)
		lda %1
		eor %2
		sta %2
	.else
		_clr8 %2
	.endif
.endmacro

;/*************************************************************************\
;|* Type: Logic operation
;|*
;|* Calculate the logical EOR of the 16-bit value at location %1 and %2
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, and destination
;\*************************************************************************/
.macro _eor16
	.if (%1 != %2)
		lda %1
		eor %2
		sta %2
		lda %1+1
		eor %2+1
		sta %2+1
	.else
		_clr16 %2
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
;|* Calculate the logical EOR of the 32-bit value at location %1 and %2
;|* storing it at %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : first source address to read from
;|*    %2 : second source address to read from, and destination
;\*************************************************************************/
.macro _eor32
	.if (%1 != %2)
		lda %1
		eor %2
		sta %2
		lda %1+1
		eor %2+1
		sta %2+1
		lda %1+2
		eor %2+2
		sta %2+2
		lda %1+3
		eor %2+3
		sta %2+3
	.else
		_clr32 %2
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Shift operation
;|*
;|* Perform an arithmetic shift left on the 8 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _asl8
	.if (%1 != %2)
		lda %1
		asl a
		sta %2
	.else
		asl %1
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
;|* Perform a left rotation on the 8 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _rol8
	.if (%1 != %2)
		lda %1
		rol a
		sta %2
	.else
		rol %1
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
;|* Perform a logical shift right on the 8 bit number at location %1
;|* and store the result at location %2. If %1 and %2 are the same then the
;|* operation is applied directly to the memory otherwise it is done in the
;|* accumulator
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : source address to read from
;|*    %2 : destination address to write to
;\*************************************************************************/
.macro _lsr8
	.if (%1 != %2)
		lda %1
		lsr a
		sta %2
	.else
		lsr %1
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
;|* Type: Arithmetic operation
;|*
;|* Increment (by one) the 16-bit value at location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : source address to increment
;\*************************************************************************/
.macro _inc16
	inc %1
	bne done
	inc %1+1
done:
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Increment (by one) the 32-bit value at location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : source address to increment
;\*************************************************************************/
.macro _inc32
	inc %1
	bne done
	inc %1+1
	bne done
	inc %1+2
	bne done
	inc %1+3
done:
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Increment the 8-bit value pointed to by location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : vector address in ZP that points to source address to increment
;|*    %2 : value by which to increment
;\*************************************************************************/
.macro _inc8p
	ldy #$0
	lda (%1),y
	clc
	adc #%2
	sta (%1),y
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Increment the 16-bit value pointed to by location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : vector address in ZP that points to source address to increment
;|*    %2 : value by which to increment
;\*************************************************************************/
.macro _inc16p
	ldy #$0
	clc
	lda (%1),y
	adc #<%2
	sta (%1),y
	iny
	lda (%1),y
	adc #>%2
	sta (%1),y
.endmacro

;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Increment the 32-bit value pointed to by location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : vector address in ZP that points to source address to increment
;|*    %2 : value by which to increment
;\*************************************************************************/
.macro _inc32p
	ldy #$0
	clc
	lda (%1),y
	adc #<%2
	sta (%1),y
	iny
	lda (%1),y
	adc #>%2
	sta (%1),y
	iny
	lda (%1),y
	adc #>%2
	sta (%1),y
	iny
	lda (%1),y
	adc #>%2
	sta (%1),y
.endmacro




;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Increment the 8-bit value pointed to by location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : vector address in ZP that points to source address to increment
;|*    %2 : value by which to increment
;\*************************************************************************/
.macro _dec8p
	ldy #$0
	lda (%1),y
	sec
	sbc #%2
	sta (%1),y
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Decrement the 32-bit value pointed to by location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : vector address in ZP that points to source address to increment
;|*    %2 : value by which to decrement
;\*************************************************************************/
.macro _dec32p
	ldy #$0
	sec
	lda (%1),y
	sbc #<%2
	sta (%1),y
	iny
	lda (%1),y
	sbc #>%2
	sta (%1),y
	iny
	lda (%1),y
	sbc #>%2
	sta (%1),y
	iny
	lda (%1),y
	sbc #>%2
	sta (%1),y
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Decrement the 16-bit value pointed to by location %1
;|*
;|* Clobbers: <none>
;|* Arguments:
;|*    %1 : vector address in ZP that points to source address to increment
;|*    %2 : value by which to decrement
;\*************************************************************************/
.macro _dec16p
	ldy #$0
	sec
	lda (%1),y
	sbc #<%2
	sta (%1),y
	iny
	lda (%1),y
	sbc #>%2
	sta (%1),y
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
	bne done
	dec %1+1
done:
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
	bne dec0
	lda %1+1
	bne dec1
	lda %1+2
	bne dec2
	dec %1+3
dec2:
	dec %1+2
dec1:
	dec %1+1
dec0:
	dec %1
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* add the 8-bit unsigned value at location %1 to the 8-bit unsigned value
;|* at %2, storing the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _add8
		clc
        lda %1
       	adc %2
       	sta %3
.endmacro

;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* add the 8-bit unsigned value at location %1 to the 8-bit unsigned value
;|* at %2, storing the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _add8u
	.if %1 != %2
		clc
        lda %1
       	adc %2
       	sta %3
	.else
        lda %1
       	asl a
       	sta %3
	.endif
.endmacro

;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* add the 16-bit value %1 to the 16-bit value at %2, storing
;|* the result in %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : value of operand #1
;|*    %2 : address of source operand #2 and destination
;\*************************************************************************/
.macro _add16i
	.if %1 != 0
		clc
        lda %2
       	adc #<%1
       	sta %2
       	lda %2+1
       	adc #>%1
       	sta %2+1
	.endif
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
		clc
        lda %1
       	adc %2
       	sta %3
       	lda %1+1
       	adc %2+1
       	sta %3+1
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
.macro _add16u
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
.macro _add32u
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
;|* subtract the 8-bit signed value at location %1 from the 8-bit signed
;|* value at %2, storing the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _sub8
		sec
        lda %1
       	sbc %2
       	sta %3
.endmacro

;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* subtract the 8-bit unsigned value at location %1 from the 8-bit unsigned
;|* value at %2, storing the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _sub8u
	.if %1 != %2
		sec
        lda %1
       	sbc %2
       	sta %3
	.else
		lda #0
		sta %3
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
	.if %1 != %2
		sec
        lda %1
       	sbc %2
       	sta %3
       	lda %1+1
       	sbc %2+1
       	sta %3+1
	.else
		_clr16 %3
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
.macro _sub16u
	.if %1 != %2
		sec
        lda %1
       	sbc %2
       	sta %3
       	lda %1+1
       	sbc %2+1
       	sta %3+1
	.else
		_clr16 %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* subtract the 16-bit value of %1 from the 16-bit value at %2, storing
;|* the result in %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : value of operand #1
;|*    %2 : address of source operand #2 and destination
;\*************************************************************************/
.macro _sub16i
	.if %1 != 0
		sec
        lda %2
       	sbc #<%1
       	sta %2
       	lda %2+1
       	sbc #>%1
       	sta %2+1
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
;|* subtract the 16-bit value at location %1 to the 16-bit value at %2,
;|* storing the result in %3
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand
;\*************************************************************************/
.macro _sub32u
	.if %1 != %2
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
	.else
		_clr32 %3
	.endif
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* negate the signed 8-bit number at location %1 and store the result at
;|* location %2. Both %1 and %2 can be the same
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of destination operand. Can be same as %1
;\*************************************************************************/
.macro _neg8
		sec
        lda #0
        sbc %1
       	sta %2
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
		bit %1+1
        .if (%1 != %2)
			bpl move
			_neg16 %1, %2
			jmp done
	move:
			_xfer16 %1, %2
		.else
			bpl done
			_neg16 %1, %2
		.endif
	done:
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
		bit %1+3
        .if (%1 != %2)
			bpl move
			_neg32 %1, %2
			jmp done
	move:
			_xfer32 %1, %2
		.else
			bpl done
			_neg32 %1, %2
		.endif
	done:
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
.macro _mul8u
		_clr8 %3
		ldx #8
	loop:
		_lsr8 %1,%1
		bcc next
		_add8u %2, %3, %3
	next:
		_asl8 %2, %2
		dex
		bne loop
.endmacro

;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Multiply two 8-bit possibly-signed numbers. Do this by checking the
;|* sign of the high bytes, and if they differ, then remember to convert
;|* the result back to negative. Then convert both operands to +ve if
;|* necessary, and run
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand.
;\*************************************************************************/
.macro _mul8
		lda %1				; compute the EOR of high-bits of both numbers
		eor %2
		php					; and store for later
		
		lda %1				; is num1 negative ?
		bpl check2			; +ve, so nothing to do here
		_neg8 %1,%1			; convert to +ve
	
	check2:
		lda %2				; is num2 negative
		bpl doMul8			; +ve, so nothing to do here
		_neg8 %2,%2
	
	doMul8:
		_mul8u %1, %2, %3	; Do an unsigned multiply
	
		plp					; Pull the saved state
		bpl done			; don't need to convert back to -ve
		_neg8 %3, %3
	
	done:
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
.macro _mul16u
		_clr16 %3
		ldx #16
	loop:
		_lsr16 %1,%1
		bcc next
		_add16u %2, %3, %3
	next:
		_asl16 %2, %2
		dex
		bne loop
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Multiply two 16-bit possibly-signed numbers. Do this by checking the
;|* sign of the high bytes, and if they differ, then remember to convert
;|* the result back to negative. Then convert both operands to +ve if
;|* necessary, and run
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
		bpl check2			; +ve, so nothing to do here
		_neg16 %1,%1		; convert to +ve
	
	check2:
		lda %2				; is num2 negative
		bpl doMul16			; +ve, so nothing to do here
		_neg16 %2,%2
	
	doMul16:
		_mul16u %1, %2, %3	; Do an unsigned multiply
	
		plp					; Pull the saved state
		bpl done			; don't need to convert back to -ve
		_neg16 %3, %3
	
	done:
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
	loop:
		_asl32 %3,%3
		_asl32 %1,%1
		bcc next
		_add16u %2, %3, %3
		bcc next
		_inc16 %3+2
	next:
		dex
		bpl loop
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
		_add16u %1, %3, %3
	.endif
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
.macro _mul32u
		_clr32 %3
		ldx #32
	loop:
		_lsr32 %1,%1
		bcc next
		_add32 %2,%3,%3
	next:
		_asl32 %2,%2
		dex
		bpl loop
.endmacro



;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Multiply two 32-bit signed numbers. Do this by checking the sign of the
;|* high bytes, and if they differ, then remember to convert the result back
;|* to negative. Then convert both operands to +ve if necessary, and run
;|*
;|* Clobbers: A, X
;|* Arguments:
;|*    %1 : address of source operand #1
;|*    %2 : address of source operand #2
;|*    %3 : address of destination operand.
;\*************************************************************************/
.macro _mul32
		lda %1				; compute the EOR of high-bits of both numbers
		eor %2
		php					; and store for later
		
		lda %1				; is num1 negative ?
		bpl check2		; +ve, so nothing to do here
		_neg32 %1,%1		; convert to +ve
	
	check2:
		lda %2				; is num2 negative
		bpl doMul32		; +ve, so nothing to do here
		_neg32 %2,%2
	
	doMul32:
		_mul32u %1, %2, %3	; Do an unsigned multiply
	
		plp					; Pull the saved state
		bpl done			; don't need to convert back to -ve
		_neg32 %3, %3
	
	done:
.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide a signed 8-bit value at location %1 by a signed 8-bit value %2
;|* and store the 8-bit remainder in location %3, surface the result in %1
;|*
;|* Clobbers: A, X, Y
;|* Arguments:
;|*    %1 : location of dividend and final result
;|*    %2 : location of divisor
;|*    %3 : location of remainder
;\*************************************************************************/
.macro _div8
		lda %2  			; Get the upper byte of the divisor
		pha					; and store in the scratch registor
		eor %1  			; EOR with sign of dividend
		pha					; and store for later

		lda %1				; Check if the dividend needs to be negated
		bpl skp1			; If it's +ve, skip the negation
		_neg8 %1,%1			; otherwise negate

skp1:
		lda %2  			; Check if the divisor needs to be negated
		bpl skp2			; If it's +ve, skip the negation
		_neg8 %2,%2

skp2:
		_clr8 %3			; Clear the accumulator
		
		lda %2				; Check for divide-by-zero error
		bne ok
		pla
		pla
		sec					; set error status
		bcs error
ok:
		ldy #$8		  	    ; Number of bits to rotate through

sdv_loop:
		_asl8 %1,%1			; left-shift dividend
		_rol8 %3,%3			; and rotate into 'accumulator'
		
		_sub8u %3,%2,%3		; subtract divisor from accumulator
		bcs sdv4			; if carry is set, r0 is +ve, skip add-back
		
		_add8u %3,%2,%3		; get r0 +ve again by adding back r2
		clc					; then always branch to next-bit routine
		bcc sdv5

sdv4:
		inc %1				; increment the results bit
		bcs sdv5			; and allow space for a longer bcs for error

error:
		bcs done
	
sdv5:
		dey					; Go to the next bit
		bne sdv_loop		; ... for 32 times

		pla					; check remainder sign,
		bpl sdv6			; skip negation if +ve
		_neg8 %1,%1
		
sdv6:
		pla					; check quotient sign
		bpl sdv8			; skip negation if +ve
		_neg8 %3,%3

sdv8:
		clc					; set status = no error
done:

.endmacro

;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide a (possibly signed) 8-bit value at location %1 by a (possibly
;|* signed) 8-bit value %2 and store the 8-bit remainder in location %3
;|* and surface the result in %1
;|*
;|* Clobbers: A, X, Y
;|* Arguments:
;|*    %1 : location of dividend and final result
;|*    %2 : location of divisor
;|*    %3 : location of remainder
;\*************************************************************************/
.macro _div8u
		_clr8 %3			; Clear the accumulator
		
		lda %2				; Check for divide-by-zero error
		bne ok
		sec					; set error status
		bcs error
ok:
		ldy #$8		  	    ; Number of bits to rotate through

sdv_loop:
		_asl8 %1,%1			; left-shift dividend
		_rol8 %3,%3			; and rotate into 'accumulator'
		
		_sub8u %3,%2,%3		; subtract divisor from accumulator
		bcs sdv4			; if carry is set, r0 is +ve, skip add-back
		
		_add8u %3,%2,%3		; get r0 +ve again by adding back r2
		clc					; then always branch to next-bit routine
		bcc sdv5

sdv4:
		inc %1				; increment the results bit
		bcs sdv5			; and allow space for a longer bcs for error

error:
		bcs done
	
sdv5:
		dey					; Go to the next bit
		bne sdv_loop		; ... for 32 times
		clc					; set status = no error
done:

.endmacro



;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide a signed 16-bit value at location %1 by a signed 16-bit value %2
;|* and store the 16-bit remainder in location %3, surface the result in %1
;|*
;|* Clobbers: A, X, Y
;|* Arguments:
;|*    %1 : location of dividend and final result
;|*    %2 : location of divisor
;|*    %3 : location of remainder
;\*************************************************************************/
.macro _div16
		lda %2+1			; Get the upper byte of the divisor
		pha					; and store in the scratch registor
		eor %1+1			; EOR with sign of dividend
		pha					; and store for later

		lda %1+1			; Check if the dividend needs to be negated
		bpl skp1			; If it's +ve, skip the negation
		_neg16 %1,%1		; otherwise negate

skp1:
		lda %2+1			; Check if the divisor needs to be negated
		bpl skp2			; If it's +ve, skip the negation
		_neg16 %2,%2

skp2:
		_clr16 %3			; Clear the accumulator
		
		lda %2				; Check for divide-by-zero error
		ora %2+1
		bne ok
		pla
		pla
		sec					; set error status
		bcs error
ok:
		ldy #$10			; Number of bits to rotate through

sdv_loop:
		_asl16 %1,%1		; left-shift dividend
		_rol16 %3,%3		; and rotate into 'accumulator'
		
		_sub16 %3,%2,%3		; subtract divisor from accumulator
		bcs sdv4			; if carry is set, r0 is +ve, skip add-back
		
		_add16 %3,%2,%3		; get r0 +ve again by adding back r2
		clc					; then always branch to next-bit routine
		bcc sdv5

sdv4:
		inc %1				; increment the results bit
		bcs sdv5			; and allow space for a longer bcs for error

error:
		bcs done
	
sdv5:
		dey					; Go to the next bit
		bne sdv_loop		; ... for 16 times

		pla					; check remainder sign,
		bpl sdv6			; skip negation if +ve
		_neg16 %1,%1
		
sdv6:
		pla					; check quotient sign
		bpl sdv8			; skip negation if +ve
		_neg16 %3,%3

sdv8:
		clc					; set status = no error
done:

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
.macro _div16x
		_clr16 %4
		ldx #32
	loop:
		_asl32 %1, %1
		_rol16 %4, %4
		_sub16u %4, %2, %4
		bcs next
		_add16u %4, %2, %4
	next:
		_rol16 %3, %3
		dex
		bpl loop
.endmacro

;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide an unsigned 16-bit value at location %1 by an unsigned 8-bit
;|* value %2 and store the 8-bit remainder in location %3 and surface the
;|* result in %1
;|*
;|* Clobbers: A, X, Y
;|* Arguments:
;|*    %1 : location of dividend and final result
;|*    %2 : location of divisor
;|*    %3 : location of remainder
;\*************************************************************************/
.macro _div16u
		_clr16 %3			; Clear the accumulator
		
		lda %2				; Check for divide-by-zero error
		bne ok
		sec					; set error status
		bcs error
ok:
		ldy #16		  	    ; Number of bits to rotate through

sdv_loop:
		_asl16 %1,%1		; left-shift dividend
		_rol16 %3,%3		; and rotate into 'accumulator'
		
		_sub16u %3,%2,%3	; subtract divisor from accumulator
		bcs sdv4			; if carry is set, r0 is +ve, skip add-back
		
		_add16u %3,%2,%3	; get r0 +ve again by adding back r2
		clc					; then always branch to next-bit routine
		bcc sdv5

sdv4:
		inc %1				; increment the results bit
		bcs sdv5			; and allow space for a longer bcs for error

error:
		bcs done
	
sdv5:
		dey					; Go to the next bit
		bne sdv_loop		; ... for 16 times
		clc					; set status = no error
done:

.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide a (possibly signed) 32-bit value at location %1 by a (possibly
;|* signed) 32-bit value %2 and store the 32-bit remainder in location %3
;|* and surface the result in %1
;|*
;|* Clobbers: A, X, Y
;|* Arguments:
;|*    %1 : location of dividend and final result
;|*    %2 : location of divisor
;|*    %3 : location of remainder
;\*************************************************************************/
.macro _div32
		lda %2+3			; Get the upper byte of the divisor
		pha					; and store in the scratch registor
		eor %1+3			; EOR with sign of dividend
		pha					; and store for later

		lda %1+3			; Check if the dividend needs to be negated
		bpl skp1			; If it's +ve, skip the negation
		_neg32 %1,%1		; otherwise negate

skp1:
		lda %2+3			; Check if the divisor needs to be negated
		bpl skp2			; If it's +ve, skip the negation
		_neg32 %2,%2

skp2:
		_clr32 %3			; Clear the accumulator
		
		lda %2				; Check for divide-by-zero error
		ora %2+1
		ora %2+2
		ora %2+3
		bne ok
		pla
		pla
		sec					; set error status
		bcs error
ok:
		ldy #$20			; Number of bits to rotate through

sdv_loop:
		_asl32 %1,%1		; left-shift dividend
		_rol32 %3,%3		; and rotate into 'accumulator'
		
		_sub32 %3,%2,%3		; subtract divisor from accumulator
		bcs sdv4			; if carry is set, r0 is +ve, skip add-back
		
		_add32 %3,%2,%3		; get r0 +ve again by adding back r2
		clc					; then always branch to next-bit routine
		bcc sdv5

sdv4:
		inc %1				; increment the results bit
		bcs sdv5			; and allow space for a longer bcs for error

error:
		bcs done
	
sdv5:
		dey					; Go to the next bit
		bne sdv_loop		; ... for 32 times

		pla					; check remainder sign,
		bpl sdv6			; skip negation if +ve
		_neg32 %1,%1
		
sdv6:
		pla					; check quotient sign
		bpl sdv8			; skip negation if +ve
		_neg32 %3,%3

sdv8:
		clc					; set status = no error
done:

.endmacro


;/*************************************************************************\
;|* Type: Arithmetic operation
;|*
;|* Divide an unsigned 32-bit value at location %1 by an unsigned 32-bit
;|* value %2 and store the 32-bit remainder in location %3
;|* and surface the result in %1
;|*
;|* Clobbers: A, X, Y
;|* Arguments:
;|*    %1 : location of dividend and final result
;|*    %2 : location of divisor
;|*    %3 : location of remainder
;\*************************************************************************/
.macro _div32u
		_clr32 %3			; Clear the accumulator
		
		lda %2				; Check for divide-by-zero error
		ora %2+1
		ora %2+2
		ora %2+3
		bne ok
		sec					; set error status
		bcs error
ok:
		ldy #$20			; Number of bits to rotate through

sdv_loop:
		_asl32 %1,%1		; left-shift dividend
		_rol32 %3,%3		; and rotate into 'accumulator'
		
		_sub32 %3,%2,%3		; subtract divisor from accumulator
		bcs sdv4			; if carry is set, r0 is +ve, skip add-back
		
		_add32 %3,%2,%3		; get r0 +ve again by adding back r2
		clc					; then always branch to next-bit routine
		bcc sdv5

sdv4:
		inc %1				; increment the results bit
		bcs sdv5			; and allow space for a longer bcs for error

error:
		bcs done
	
sdv5:
		dey					; Go to the next bit
		bne sdv_loop		; ... for 32 times
		clc					; set status = no error
done:

.endmacro

;/*************************************************************************\
;|* Type: Comparison operation
;|*
;|* Compare an 8-bit quantity at %1 to an immediate value %2
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : location of value 1
;|*    %2 : value to compare against
;\*************************************************************************/
.macro _cmpi8
		lda %1
		cmp #<%2
	done:
.endmacro

;/*************************************************************************\
;|* Type: Comparison operation
;|*
;|* Compare a 16-bit quantity at %1 to an immediate value %2. Returns as
;|* soon as a difference is found
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : location of value 1
;|*    %2 : value to compare against
;\*************************************************************************/
.macro _cmpi16
		lda %1+1
		cmp #>%2
		bne done
		lda %1
		cmp #<%2
	done:
.endmacro


;/*************************************************************************\
;|* Type: Comparison operation
;|*
;|* Compare a 32-bit quantity at %1 to an immediate value %2. Returns as
;|* soon as a difference is found
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : location of value 1
;|*    %2 : value to compare against
;\*************************************************************************/
.macro _cmpi32
		lda %1+3
		cmp #>>>%2
		bne done
		lda %1+2
		cmp #>>%2
		bne done
		lda %1+1
		cmp #>%2
		bne done
		lda %1
		cmp #<%2
	done:
.endmacro


;/*************************************************************************\
;|* Type: Comparison operation
;|*
;|* Compare an 8-bit quantity at %1 to another at %2.
;|*
;|* Clobbers: A
;|* Arguments:
;|*    %1 : location of value 1
;|*    %2 : location of value 2
;\*************************************************************************/
.macro _cmp8
		lda %1
		cmp %2
	done:
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
		bne done
		lda %1
		cmp %2
	done:
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
		bne done
		lda %1+2
		cmp %2+2
		bne done
		lda %1+1
		cmp %2+1
		bne done
		lda %1
		cmp %2
	done:
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
		beq frag
	page:
		lda (%1),Y
		sta (%2),Y
		iny
		bne page
		inc %1+1
		inc %2+1
		dex
		bne page
	frag:
		beq done
		lda (%1),Y
		sta (%1),Y
		iny
		bne frag
	done:
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
		bne entry				; Got to copy the fragment
		beq	copyPage			; Start copying pages
		
	copyByte:
		lda (%1),y
		sta (%2),y
	
	entry:
		dey
		bne copyByte
		lda (%1),y				; copy the remaining byte
		sta (%2),y
	
	copyPage:
		ldx %3+1
		beq done
	
	initBase:
		dec %1+1
		dec %2+1
		dey
	
	copyBytes:
		lda (%1),y				; unroll the loop to make it a little
		sta (%1),y				; faster. Note that we unroll by 3
		dey						; because 255/3 = 85 remainder 0
		lda (%1),y
		sta (%1),y
		dey
		lda (%1),y
		sta (%1),y
		dey

	copyEntry:
		bne copyBytes
		lda (%1),y
		sta (%1),y
		dex
		bne initBase
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
		bcc safe
		_memcpyUp %1, %2, %3
		jmp done
	safe:
		_memcpyDn %1, %2, %3
	done:
.endmacro


