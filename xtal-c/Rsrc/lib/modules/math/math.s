;
; Floating math support for xtal based on Rankin / Woz code
;

.include stdmacros.s
.include modules/stdio/stdio.s

; Register definitions in zero-page
;
flt_x2			= $80			; Exponent for floating reg #1
flt_m2			= $81			; Mantissa for floating reg #1 [3 bytes]
flt_x1			= $84			; Exponent for floating reg #2
flt_m1			= $85			; Mantissa for floating reg #2 [3 bytes]
flt_e			= $88			; scratch workspace adjacent to F2

flt_z			= $f0			; scratch workspace
flt_t			= $f4			; scratch workspace
flt_save		= $f8			; scratch workspace, save original exp/mant

; aliases to make the code clearer

; for f_fromString
flt_dpc			= $f0			; decimal-point-count.
flt_numExp		= $f1			; value of the exponent
flt_sign		= $f2			; Whether we have seen a sign character
flt_int			= $f3			; integer part of the floating point value
flt_signComp	= $f4			; Sign-comparison

; for f_toString
flt_len			= $f0			; Current index into string
flt_o1			= $f2			; Overflow for floating reg 0
flt_expCnt		= $f3			; Exponent adjustment
flt_currVar		= $f4			; Current var address, low byte

; for f_compare
flt_vecl		= $f5			; low  byte of vector to float to compare to
flt_vech		= $f6			; high byte of vector to float to compare to

; flt_vec is used as a general purpose vector when calling into routines
; which don't use the external hardware
flt_vec			= $92			; and $93

; Error codes from the floating point routines. Error is stored in flt_err
; and ought to be checked by calling routines
FLT_BAD_ARG		= $1
FLT_EXPONENT	= $2
FLT_OVERFLOW	= $3
FLT_DEC_POINTS	= $4

;
; Defines for constants in the code
FLT_MAX_INT_EXP	= $98			; maximum integer exponent

PLUS_0			= $00			; X or Y plus 0
PLUS_1			= $01			; X or Y plus 1
PLUS_2			= $02			; X or Y plus 2
PLUS_3			= $03			; X or Y plus 3

; ---------------------------------------------------------------------------

init:

	ldx #3
setup:
	lda two,x
	sta flt_x1,x
	dex
	bpl setup
	
	_movi16 str,flt_vec
	jsr fp_toString
	brk
	
	_movi16 str,f0
	;call printLine
	brk

one:
	.byte $80, $40, $00, $00
two:
	.byte $81, $40, $00, $00

str:
	.byte "                     "
	
; ---------------------------------------------------------------------------
; ASCII to float. Expects the address of the source buffer to be
; in flt_vec and the floating point number will end up in F1

fp_fromString:
	_clr32 flt_x1;				; clear the value in F1
	sta flt_dpc					; zero the decimal point count
	sta flt_numExp				; zero the exponent value
	
	jsr f_getCh					; Get a character from the string
	bcc f_start					; If the character is 0-9, start processing

	cmp #'-'					; Check to see if if this chare is a '-'
	bne f_plus

	ldx #ff						; modify the sign flag
	stx flt_sign
	jsr f_incGetCh				; Increment the string index and get next char
	jmp f_start

f_plus:
	cmp #'+'					; Check to see if this chare is a '+'
	bne f_haveDp
	
	jsr f_incGetCh				; Increment the string index and get next char
	jmp f_start

f_haveDp:
	cmp #'.'					; Check to see if this chare is a '.'
	bne f_start
	
	inc flt_dpc					; Make sure we only have 1 decimal point
	ldx flt_dpc
	cpx #1
	beq f_start
	
	lda #FLT_DEC_POINTS
	sta f_err;
	rts
	
	
f_start:
	pha							; store off the current char
	cmp flt_dpc					; check to see if we have a decimal
	beq	f_noDpoint				; skip increment if we have no .
	inc flt_numExp

f_noDpoint:
	jsr fp_mult10				; multiply F0 by 10
	; FIXME: needs completion
	rts

; ---------------------------------------------------------------------------
; float to ASCII buffer. Expects the address of the destination buffer to be
; in flt_vec and the floating point number to emit ascii for to be in F1

fp_toString:
	ldy #0						; Set the character index
	lda #' '					; By default we're +ve
	bit flt_m1					; Test for -ve
	bpl f2s_plus				; skip if Y
	lda flt_m1					; Fetch the byte-with-sign-bit
	and  #$7f					; clear sign bit
	sta flt_m1					; and store
	lda #'-'

f2s_plus:
	sta (flt_vec),y
	sty flt_len					; Save the character index
	iny							; increment index
	ldx flt_x1					; load the exponent
	bne f2s_not0				; if not zero, skip
	
	lda #'0';					; Short-circuit it to just "0"
	iny							; increment index
	sta (flt_vec),y				; Save to output string
	jmp f2s_terminate

f2s_not0:
	lda #0						; Zero the expected exponent
	cpx #$81					; exponent >= $81 (1.00000)
	bcs f2s_moreThan1			; Y? : skip if so
	
	; if we're here it means the float is < 1.00000
	
	_xfer32 f_million,flt_x2	; put 1,000,000 into F2
	jsr fp_mul					; F1 = F1 * 1,000,000
	lda #$FA					; set number exponent = -6

f2s_moreThan1:
	sta flt_numExp				; store off the exponent

f2s_loop1:
	lda #<f_maxNonSci			; point to low byte of 999999.4375
	ldy #>f_maxNonSci			; (max value before scientific notation)
	jsr fp_compare
	beq f2s_digits				; equal, just do digits
	
	bpl f2s_div10				; do /10 if FP1 > (AY)

f2s_loop2:
	lda #<f_maxDec				; point to low byte of 99999.9375
	ldy #>f_maxDec				; (max value with at least one decimal place)
	jsr fp_compare
	beq f2s_digits				; >= .. just do digits
	bpl f2s_digits
	
	jsr fp_mult10				; do *10 since FP1 < (AY)
	dec flt_numExp				; decrement exponent count
	bne f2s_loop2				; and repeat

f2s_div10:
	jsr fp_div10				; do /10
	inc flt_numExp				; increment exponent count
	bne f2s_loop1

f2s_digits:
	jsr fp_tofixed				; convert F1 to fixed
	ldx #$01					; set default digits before '.' to 1
	lda flt_numExp				; get number exponent count
	clc							; prepare to add
	adc #$7						; up to six digits before point
	bmi f2s_1digit				; just one digit before the .
	
	cmp #$8						; A>=8 if value >=1e6
	bcs f2s_edigit				; branch if >8
	
								; carry is clear...
	adc #$ff					; subtract 1 from digit count
	tax							; copy A
	lda #$2						; set exponent adjust

f2s_1digit:
	sec							; prepare for subtract

f2s_edigit:
	sbc #$2						; subtract 2
	sta flt_expCnt				; save exponent adjustment
	stx flt_numExp				; save digits-before-dp count
	txa							; copy to A
	beq f2s_nodigits			; branch if no digits before dp
	bpl f2s_ifdigits			; branch if there are digits before dp

f2s_nodigits:
	ldy flt_len					; Get output string index
	lda #'.'					; decimal point
	iny							; increment index
	sta (flt_vec),y				; Save to output string

	txa
	beq f2s_noZero
	
	lda #'0'					; literal '0' char
	iny							; increment index
	sta (flt_vec),y				; save to output string

f2s_noZero:
	sty flt_len					; save output string index

f2s_ifdigits:
	ldy #$0						; clear index point
	ldx #$80					; loop count

f2s_subMagn:
	lda flt_m1+2				; get F1, mantissa-3
 	clc							; prepare to add
	adc f_asc3,Y				; add -ve LSB
	sta flt_m1+2				; save F1, mantissa-3
	
	lda flt_m1+1				; get F1, mantissa-2
	adc f_asc2,Y				; add -ve LSB
	sta flt_m1+1				; save F1, mantissa-2
	
	lda flt_m1					; get F1, mantissa-1
	adc f_asc1,Y				; add -ve LSB
	sta flt_m1					; save F1, mantissa-1
	
	inx
	bcs f2s_nowNeg				; carry is set, so check
	bpl f2s_subMagn				; not negative, try again
	bmi f2s_dodigits			; we're now negative, so jump
	
f2s_nowNeg:
	bmi f2s_subMagn				; go round again if -ve

f2s_dodigits:
	txa							; X -> A
	bcc f2s_pldigits			; no need to correct

	eor #$ff					; complement it
	adc #$a						; add 10
	
f2s_pldigits:
	adc #'0' - 1				; Add "0"-1 to the result
	iny							; increment Y ..
	iny							; to the next less ...
	iny							; power of 10
	sty flt_currVar				; save as current var address, lo byte
	
	ldy flt_len					; get output string index
	iny							; increment index
	tax							; copy character to x
	and #$7f					; get rid of any high bit
	sta (flt_vec),y				; save to output string
	
	dec flt_numExp				; decrement the number of chars before '.'
	bne f2s_notDot				; if this isn't where the dot goes, branch
	lda #'.'					; set the character to be '.'
	iny							; increment index
	sta (flt_vec),y				; save to output string

f2s_notDot:
	sty flt_len					; save output string index
	ldy flt_currVar				; load current var address, lo byte
	
	txa							; get character back
	eor #$ff
	and #$80
	tax
	
	cpy #$12					; compare index with max
	bne f2s_subMagn				; loop if not max
	
	; now remove trailing zeros
	
	ldy flt_len
	
f2s_last0:
	lda (flt_vec),Y				; get character from output string
	dey							; decrement string index
	cmp #'0'					; is this a zero ?
	beq f2s_last0
	
	cmp #'.'					; check to see if it was a '.'
	beq fs2_skipdot
	iny

f2s_skipdot:
	ldx flt_expCnt				; get the exponent count
	beq f2s_terminate			; done, add a trailing '\0' to the string
	
	; exponent is not zero, so append to the string
	iny
	lda #'E'					; add an 'E' to the string
	sta (flt_vec),y
	lda #'+'					; prepare for adding '+'
	
	ldx flt_expCnt				; get the exponent count again
	bpl f2s_posexp				; branch if exponent is positive
	
	lda #0						; clear A
	sec							; prepare for subtract
	sbc flt_expCnt				; convert to +ve exponent
	tax
	lda #'-'					; change exponent to be '-' as 1st char

f2s_posexp:
	iny							; increment the string index
	sta (flt_vec),y				; store the + or - char
	txa							; get the exponent count back
	
	ldx #'0' - 1				; get one less than the '0' char
	sec

f2s_exp10:
	inx							; increment the 10's char
	sbc #$a						; subtract 10 from exponent
	bcs f2s_exp10				; loop while > 0
	
	adc #':'					; $30 ('0') + $a
	pha
	txa
	iny							; increment the string index
	sta (flt_vec),y				; store the 10's character
	pla
	iny							; increment the string index
	sta (flt_vec),y				; store the units character
	
f2s_terminate:
	lda #0						; terminate the string with \0
	iny
	sta (flt_vec),y				; store the units character
	rts
	


; ---------------------------------------------------------------------------
; Internal routine: Increment the character pointer and get the next one in A
;
f_incGetCh:						; increment the pointer prior ..
	inc flt_vec					; to fetching another character ..
	bne f_getCh					; from the string pointer
	inc flt_vec+1
	
f_getCh:						; fetch a character from the string
	ldy	#0
	lda (flt_vec),y
	
	cmp #' '					; checking for ' ' and fetching ..
	beq f_incGetCh				; the next one if we find a ' '
	
	sec 						; Determine whether the char ..
	sbc #'0'					; is in the range '0' to ..
	sec							; '9'
	sbc #$d0
f_done:
	rts

	
; ---------------------------------------------------------------------------
; Multiply F1 by 10
;
fp_mult10:
	_xfer32 flt_x1,flt_x2		; copy F1 to F2
	lda flt_x1					; make sure the exponent is in A
	tax							; copy exponent
	beq f_done					; return if 0 exponent
	
	clc
	adc #02						; add 2 to exponent (=> *4)
	sta flt_x1
	bcc fp_mult10_1				; no overflow, continue
	jmp f_Overflow				; abort on overflow

fp_mult10_1:
	jsr fp_add					; add f1 to f0, store in f0	(=> *5)
	inc flt_x1					; increment f0 exponent (=> *10)
	bne f_done					; if non-zero return, we're done
	jmp doOverflow				; overflow error

	
; ---------------------------------------------------------------------------
; Clear F1 and return
;
; expects A to be 0 on entry

fp_clrMantissa:
	sta flt_m1					; Clear all the bytes
	sta flt_m1+1
	sta flt_m1+2
	tay							; Clear Y
	rts							; return
	
; ---------------------------------------------------------------------------
; Divide F1 by 10
;
fp_div10:
	_xfer32 flt_x1,flt_x2		; copy F1 to F2
	_xfer32 f_ten,flt_x1		; copy '10' to F1
	jmp fp_div					; divide, leaving result in F1


	
; ---------------------------------------------------------------------------
; Convert a float to fixed
;
fp_tofixed:
	lda	flt_x1					; get FP1 exponent
	beq fp_clrMantissa			; if zero, clear mantissa and return
	
	sec							; prepare for subtract
	sbc #FLT_MAX_INT_EXP		; subtract the max integer range exponent
	bit flt_m1					; test the sign of F1
	bpl toF_notNeg				; skip adjustment if positive
	
	; F1 was negative
	tax							; copy subtracted exponent
	lda #$ff					; overflow for -ve number
	sta flt_o1					; set the overflow byte for F1
	jsr fp_comp2				; complement the mantiss of F1
	txa							; restore A (subtracted exponent)
	
toF_notNeg:
	ldx #flt_x1					; set X to offset in page of F1 exponent
	cmp #$f9					; compare size of exponent with 8
	bpl fp_Alt8Shft				; if <8 shifts, branch and return
	jsr fp_Agt8Shft				; else: shift A times right (A>8)
	sty flt_o1					; clear overflow byte
	rts


; ---------------------------------------------------------------------------
; Shift FP1 A times to the right, where A < 8
;
fp_Alt8Shft:
	tay							; copy shift count
	lda flt_m1					; extract the sign
	and #$80
	lsr flt_m1					; shift F1 mantissa-1
	ora flt_m1					; OR the sign bit back in
	sta flt_m1					; save F1 mantissa-1
	jsr fp_Ylt8Shft				; shift F1 Y times to the right
	sty flt_o1					; clear the overflow
	rts

; ---------------------------------------------------------------------------
; Shift FP1 -A times to the right, where -A > 8
;
f_partShft:
	ldy PLUS_2,x				; get Fx mantissa-2
	sty PLUS_3,x				; store in Fx mantissa-3
	ldy PLUS_1,x				; get Fx mantissa-1
	sty PLUS_2,x				; store in Fx mantissa-2
	ldy flt_o1					; get the overflow byte
	sty PLUS_1,x				; store in Fx mantissa-1
	
fp_Agt8Shft:
	adc #$8						; add 8 to the shift count
	bmi f_partShft				; do 8 shifts if still -ve
	beq f_partShft				; do 8 shifts if zero, as well
	
	sbc #$8						; else subtract 8 again
	tay							; save count to Y
	bcs f_shiftDone
	
; ---------------------------------------------------------------------------
; Shift Fx Y times to the right
;
f_shftMore:
	asl PLUS1,x					; shift Fx mantissa 1
	bcc f_skipShift				; if +ve, skip add
	inc PLUS1,x					; will set b7 eventually

f_skipShift:
	ror PLUS1,x					; shift Fx mantissa (correct for ASL)
	ror PLUS1,x					; shift Fx mantissa (place carry in b7)

fp_Ylt8Shft:
	ror PLUS2,x					; shift Fx mantissa 2
	ror PLUS3,x					; shift Fx mantissa 3
	iny
	bne f_shftMore				; branch if more to do (Y!=0)

f_shiftDone:
	clc
	rts
	
; ---------------------------------------------------------------------------
; Two's complement the mantissa of F1
;
fp_comp2:
	lda flt_m1					; get byte 1 of the mantissa
	eor #$ff					; complement it
	sta flt_m1					; and store
	
	lda flt_m1+1				; get byte 2 of the mantissa
	eor #$ff					; complement it
	sta flt_m1+1				; and store

	lda flt_m1+2				; get byte 3 of the mantissa
	eor #$ff					; complement it
	sta flt_m1+2				; and store
	rts
	
; ---------------------------------------------------------------------------
; Natural log
;
; Computes the natural log of the 16-bit int value in M1 and places the
; floating-point result in M1

;.function logf
;.clobber x,y,a


logf:
	lda #0
	sta flt_err					; Initialise the error register
	lda	flt_m1
	beq error					; Can't take log(0)
	bpl ok						; Can work with >0

error:
	lda #FLT_BAD_ARG
	sta flt_err
	rts

ok:
	jsr swap					; move integer input to M2
	
	ldx #0						; load x for high byte of exponent
	lda flt_x2					; Get low byte of input
	ldy #$80					; set y to the value of a zero exponent
	sty flt_x2					; set exponent 2 to 0
	eor #$80					; complement the sign bit of the input
	sta flt_m1+1				; store in fp1-low-byte-if-int
	
	bpl negexp					; is exponent negative ?
	dex							; yes, set X to $FF
	stx flt_m1					; set upper byte of exponent
	
negexp:
	jsr float					; convert arg to float
	
	ldx #3						; set up for a 4-byte transfer
sexp1:
	lda flt_x2,x
	sta flt_z,x					; copy mantissa to flt_z
	lda flt_x1,x
	sta	flt_save,x				; save exponent in flt_save
	lda f_r22,x
	sta	flt_x1,x					; store sqrt(2) in flt_x1
	dex
	bpl sexp1

	jsr fp_sub					; Z - SQRT(2)

	ldx #3						; set up for 4-byte transfer
savet:
	lda flt_x1,x				; save exp/mant1 as T
	sta flt_t,x
	lda flt_z,x					; load exp/mant1 as Z
	sta flt_x1,x
	lda f_r22,x					; load exp/mant2 with sqrt(2)
	sta flt_x2,x
	dex
	bpl savet					; loop until done
	
	jsr fp_add					; Z + SQRT(2)

	ldx #3						; set up for 4-byte transfer
tm2:
	lda flt_t,x					; load T into exp/mant2
	sta flt_x2,x
	dex
	bpl tm2
	
	jsr fp_div					; T = (Z-SQRT(2)) / (Z + SQRT(2))
	
	ldx #3						; set up for 4-byte transfer
mit:
	lda flt_x1,x				; copy exp/mant1 to T and..
	sta flt_t,x					; load exp/mant2 with T
	sta flt_x2,x
	dex
	bpl mit

	jsr fp_mul					; T = T*T
	jsr swap					; move T*T to exp/mant2
	
	ldx #3						; set up for 4-byte transfer
mic:
	lda f_c,x
	sta flt_x1,x				; load exp/mant1 with C
	dex
	bpl mic						; loop until done
	
	jsr fp_sub					; T*T-C
	
	ldx #3						; set up for 4-byte transfer
m2mb:
	lda f_mb,x					; load exp/mant2 with mb
	sta flt_x2,x
	dex
	bpl m2mb
	
	jsr fp_div					; MB / (T*T-C)
	
	ldx #3						; set up for 4-byte transfer
m2a1:
	lda f_a1,x					; load exp/mant2 with a1
	sta flt_x2,x
	dex
	bpl m2a1

	jsr fp_add					; A1 + MB / (T*T-C)

	ldx #3						; set up for 4-byte transfer
m2t:
	lda flt_t,x					; load exp/mant2 with T
	sta flt_x2,x
	dex
	bpl m2t
	
	jsr fp_mul					; (MB/(T*T-C)+A1)*T
	
	ldx #3						; set up for 4-byte transfer
m2mhl:
	lda f_mhlf,x				; load exp/mant2 with 0.5
	sta flt_x2,x
	dex
	bpl m2mhl

	jsr fp_add 					; (MB/(T*T-C)+A1)*T + 0.5

	ldx #3						; set up for 4-byte transfer
ldexp:
	lda flt_save,x				; load exp/mant2 with original Exponent
	sta flt_x2,x
	dex
	bpl ldexp

	jsr fp_add					; + orig exponent

	ldx #3						; set up for 4-byte transfer
mle2:
	lda f_le2,x					; load exp/mant2 with LN(2)
	sta flt_x2,x
	dex
	bpl mle2

	jsr fp_mul					; * LN(2)
	rts							; return result in mant/exp1
	
	

; ---------------------------------------------------------------------------
; log base 10
;
log10:
	jsr logf					; compute natural log
	ldx #3						; set up for 4-byte transfer
l10:
	lda f_ln10,x				; load exp/mant2 with 1/ln(10)
	sta flt_x2,x
	dex
	bpl l10
	jsr fp_mul					; log10(x) = ln(x)/ln(10)
	rts

; ---------------------------------------------------------------------------
; Exponent
;

exp:
	lda #0						; initialise the error register
	sta flt_err
	ldx #3						; set up for 4-byte transfer
	lda f_l2e,x					; load exp/mant2 with log(base 2) of E
	sta flt_x2,x
	dex
	bpl exp+2

	jsr fp_mul					; log2(e) * x
	
	ldx #3						; set up for 4-byte transfer
fsa:
	lda flt_x1,x				; copy exp/mant1 to Z
	sta flt_z,x
	dex
	bpl fsa
	
	jsr fix						; convert contents of exp/mant2 to an int
	
	lda flt_m1+1				; save result into 'flt_int'
	sta flt_int
	
	sec
	sbc #124					; int val - 124
	lda flt_m1
	sbc #0
	bpl f_Overflow

	clc
	lda flt_m1+1
	adc #120					;  add 120 to int
	lda flt_m1
	adc #0
	bpl contin					; if result positive, continue
	
	lda #0						; int < -120, set result to 0 and return
	ldx #3
zero:
	sta flt_x1,x
	dex
	bpl zero
	rts

f_Overflow:
	lda #FLT_OVERFLOW
	sta flt_err
	rts

contin:
	jsr float					; convert int to float
	ldx #3						; set up for 4-byte transfer
entd:
	lda flt_z,x					; load exp/mant2 with z
	sta flt_x2,x
	dex
	bpl entd
	
	jsr fp_sub					; z=z-float(int)
	
	ldx #3						; set up for 4-byte transfer
zsav:
	lda flt_x1,x					; save exp/mant1 ..
	sta flt_z,x					; in z ..
	sta flt_x2,x					; and x2
	dex
	bpl zsav

	jsr fp_mul					; z = z*z
	
	ldx #3						; set up for 4-byte transfer
la2:
	lda f_a2,x					; load exp/mant2 with a2
	sta flt_x2,x
	lda flt_x1,x					; save exp/mant1 as flt_save
	sta flt_save,x
	dex
	bpl la2

	jsr fp_add					; z*z + a2
	
	ldx #3						; set up for 4-byte transfer
lb2:
	lda f_b2,x					; load exp/mant2 with b2
	sta flt_x2,x
	dex
	bpl lb2
	
	jsr fp_div					; B / (Z*Z + A2)
	
	ldx #3						; set up for 4-byte transfer
dload:
	lda flt_x1,x					; save exp/mant1 as T
	sta flt_t,x
	lda f_c2,x					; load exp/mant1 with C2
	sta flt_x1,x
	lda flt_save,x				; load exp/mant2 with sExp
	sta flt_x2,x
	dex
	bpl dload

	jsr fp_mul					; Z * Z * C2
	jsr swap					; swap exp/mant1 with exp/mant2
	
	ldx #3						; set up for 4-byte transfer
ltmp:
	lda flt_t,x					; load exp/mant1 with T
	sta flt_x1,x
	dex
	bpl ltmp
	
	jsr fp_sub					; C2*Z*Z-B2/(Z*Z+A2)

	ldx #3						; set up for 4-byte transfer
ldd:
	lda f_d,x					; load exp/mant2 with D
	sta flt_x2,x
	dex
	bpl ldd
	
	jsr fp_add					; D+C2*Z*Z-B2/(Z*Z+A2)
	jsr swap					; swap exp/mant1 with exp/mant2

	ldx #3						; set up for 4-byte transfer
lfa:
	lda flt_z,x					; load exp/mant1 with Z
	sta flt_x1,x
	dex
	bpl lfa
	
	jsr fp_sub					; -Z+D+C2*Z*Z-B2/(Z*Z+A2)

	ldx #3						; set up for 4-byte transfer
lf3:
	lda flt_z,x					; load exp/mant2 with Z
	sta flt_x2,x
	dex
	bpl lf3

	jsr fp_div					; Z/(-Z+D+C2*Z*Z-B2/(Z*Z+A2))

	ldx #3						; set up for 4-byte transfer
ld12:
	lda f_mhlf,x				; load exp/mant2 with 0.5
	sta flt_x2,x
	dex
	bpl ld12
	
	jsr fp_add					; Z/(-Z+D+C2*Z*Z-B2/(Z*Z+A2)) + 0.5
	
	sec							; add int to exponent with carry set
	lda flt_int					; to multiply by
	adc flt_x1					; 2**(int+1)
	sta flt_x1					; return result to Exponent
	rts							; return (.5+Z/(-Z+D+C2*Z*Z-B2/(Z*Z+A2))*2**(INT+1)
	
	
	
	
	
	
	
; ---------------------------------------------------------------------------
; Internal routine - Add two aligned mantissas

add:
	clc							; clear carry
	ldx #2						; set up for 3-byte add
add1:
	lda flt_m1,x				; add a byte of ..
	adc flt_m2,x				; mant2 to mant1
	sta flt_m1,x
	dex
	bpl add1
	rts

		
; ---------------------------------------------------------------------------
; Internal routine - Take absolute value of mantissas 1 and 2

md1:
	asl flt_sign				; clear bit1 of sign
	jsr abswap					; absolute value of mant1, then swap mant2

abswap:
	bit flt_m1					; is mant1 negative ?
	bpl abswp1					; no: swap with mant2 and return
	jsr fcompl					; yes: complement it
	inc flt_sign				; increment sign, complementing lsb

abswp1:
	sec							; set carry for return to mul/div
	; fall through to swap
		
; ---------------------------------------------------------------------------
; Internal routine - swap Exp/Mant1 to Exp/Mant2, and mant1->flt_e

swap:
	ldx #$04					; set up for 4-byte swap
swap1:
	sty flt_e-1,x				; copy to E
	lda flt_x1-1,x				; swap a byte of exp/mant1 with..
	ldy flt_x2-1,x				; exp/mant2 and leave a copy of..
	sty flt_x1-1,x				; mant1 in E (3 bytes). E+3 used
	sta flt_x2-1,x
	dex							; next byte
	bne swap1					; loop until done
	rts
	
		
; ---------------------------------------------------------------------------
; Internal routine - convert a 16-bit integer into a float representation
; note that the high-byte of the integer is expected in flt_m1 and the low
; byte is expected in flt_m1+1 (the opposite way around to normal)

float:
	lda #$8E
	sta flt_x1					; Set exponent-1 to 14 dec

norm1:
	dec flt_x1					; decrement exponent-1
	
	asl flt_m1+2				; 3-byte shift of mantissa-1 left
	rol flt_m1+1
	rol flt_m1
	
norm:
	lda flt_m1					; Mantissa-1 high-byte
	asl a
	eor flt_m1
	bmi rts1
	lda flt_x1
	bne norm1					; no, continue normalising
rts1:
	rts							; all done


; ---------------------------------------------------------------------------
; fp_sub
;
; The minuend (subtract from) is in FP1 and the subtrahend (val to subtract)
; is in FP2.  Both should be normalized to retain maximum precision prior
; to calling FSUB.
;
; Uses: FCOMPL, ALGNSWP, FADD, ADD, NORM, RTLOG.
;
; Exit: The normalized difference is in FP1 with the mantissa truncated to 24
; bits.  FP2 holds either the minued or the negated subtrahend, whichever is
; of greater magnitude.  E is altered but SIGN and SCR are not.  the A-REG is
; altered and the X-REG is cleared.  The Y-REG is not disturbed.

fp_sub:
	lda #0						; initialise the error register
	sta flt_err
	jsr fcompl					; Complement mantissa, clears carry unless 0

swpalg:
	jsr algnsw					; Right shift M1 or swap with M2 on carry
	; fall through into fp_add


; ---------------------------------------------------------------------------
; fp_add
;
; The two addends are in FP1 and FP2 respectively.  For maximum
; precision, both should be normalized.
;
; Uses: SWPALGN, ADD, NORM, RTLOG.
;
; Exit: The normalized sum is left in FP1.  FP2 contains the addend of
; greatest magnitude.  E is altered but sign is not.  The A-REG is altered
; and the X-REG is cleared.  The sum mantissa is truncated to 24 bits.

fp_add:
	lda #0						; initialise the error register
	sta flt_err
	lda flt_x2					; compare exp1 with exp2
	cmp flt_x1
	bne swpalg					; if unequal swap addends, or align mantissas
	
	jsr add						; Add aligned mantissas

addend:
	bvc norm					; no overflow, normalise results
	bvs rtlog					; ov: shift mant1 right, note carry is correct sign

algnsw:
	bcc swap					; swap if carry clear, else shift right

rtar:
	lda flt_m1					; sign of mantissa-1 into carry ..
	asl	a						; for right arithmetic shift

rtlog:
	inc flt_x1					; inc x1 to compensate for right-shift
	beq ovfl					; exp1 out of range

rtlog1:
	ldx #$fa					; index for 6-byte right-shift
ror1:
	ror flt_e+3,x
	inx							; next byte of shift
	bne ror1					; loop until done
	rts
	
; ---------------------------------------------------------------------------
; fp_mul
;
; Entry: The multiplicand and multiplier must reside in FP1 and FP2
; respectively.  Both should be normalized prior to calling fp_mul to retain
; maximum precision.
;
; Uses: MD1, MD2, RTLOG1, ADD, MDEND.
;
; Exit: The signed normalized floating point product is left in FP1.  M1 is
; truncated to contain the 24 most significant mantissa bits (including
; sign).  The absolute value of the multiplier mantissa (M2) is left in FP2.
; E, SIGN, and SCR are altered.  The A- and X-REGs are altered and the Y-REG
; contains $FF upon exit.

fp_mul:
	jsr md1						; Abs value of mant1 and mant2
	adc flt_x1					; add exp1 to exp2 for product exponent
	jsr md2						; check product exponent and prepare for mul
	clc							; clear carry
mul1:
	jsr rtlog1					; mant1 and E right (product & multiplier)
	bcc mul2					; if carry clear, skip partial product
	jsr add						; add multiplicand to product

mul2:
	dey							; next mul iteration
	bpl mul1

mdend:
	lsr flt_sign				; test sign (even/odd)

normx:
	bcc norm					; if even, normalise product, else complement

fcompl:
	sec							; set the carry
	ldx #$03					; set up for 3 byte subtraction
compl1:
	lda #0						; clear A
	sbc flt_x1,x				; subtract byte of exp1
	sta flt_x1,x				; restore it
	dex							; next byte
	bne compl1					; loop until done
	beq addend					; normalise (or shift right on overflow)

	
; ---------------------------------------------------------------------------
; fp_div
;
; Entry: The normalized dividend is in FP2 and the normalized divisor is in
; FP1.
;
; Uses: MD1, MD2, MDEND.
;
; Exit: The signed normalized floating point quotient is left in FP1.  The
; mantissa (M1) is truncated to 24 bits.  The 3-bit M1 extension (E) contains
; the absolute value of the divisor mantissa.  MD2, SIGN, and SCR are
; altered.  The A- and X-REGs are altered and the Y-REG is cleared.

fp_div:
	lda #0						; initialise the error register
	sta flt_err
	jsr md1						; Take abs value of mant1, mant2
	sbc flt_x1					; subtract exp1 from exp2
	jsr md2						; Save as quotient exp
div1:
	sec							; set carry for subtract
	ldx #$02					; set up for 3-byte subtract
div2:
	lda flt_m2,x				; subtract a byte of E ..
	sbc flt_e,x					; from mant2
	pha
	dex							; next byte
	bpl div2					; and loop until done
	
	ldx #$fd					; set up for 3-byte conditional move
div3:
	pla							; pull a byte of difference off the stack
	bcc div4					; if mant2 < E, then don't restore mant2
	sta flt_m2+3,x
div4:
	inx							; next less-significant byte
	bne div3					; loop until done

	rol flt_m1+2				; roll quotient ..
	rol flt_m1+1				; left, carry ..
	rol flt_m1					; into lsb
	asl flt_m2+2
	rol flt_m2+1				; shift dividend left
	rol flt_m2
	bcs ovfl					; overflow is due to unnormalised divisor
	dey							; next divide iteration
	bne div1					; loop until done, 23 iterations
	beq mdend					; normalise quotient and correct sign

md2:
	stx flt_m1+2				; clear mantissa (3 bytes) for ..
	stx flt_m1+1				; mul/div
	stx flt_m1
	bcs ovchk					; if exp calc, set carry, check for overflow
	bmi md3						; if neg, no underflow
	pla 						; pop one
	pla							; return level
	bcc normx					; clear x1 and return

md3:
	eor #$80					; complement sign bit of exponent
	sta flt_x1					; store it
	ldy #$17					; count for 24 mul or 23 div iterations
	rts							; return

ovchk:
	bpl md3						; if positive exponent, then no overflow
	
ovfl:
	lda #FLT_EXPONENT
	sta flt_err
	rts



; ---------------------------------------------------------------------------
; fix
;
; Entry: Convert a 16-bit integer to floating point format
;
;
fix1:
	jsr rtar					; shift mant1 right and inc Exponent
fix:
	lda flt_x1					; check Exponent
	bpl undfl
	cmp #$8E					; is exponent 14 ?
	bne fix1					; no, shift
	bit flt_m1
	bpl rtrn
	lda flt_m1+2
	beq rtrn
	inc flt_m1+1
	bne rtrn
	inc m1
rtrn:
	rts

undfl:
	lda #0
	sta flt_m1
	sta flt_m1+1
	rts
	
; ---------------------------------------------------------------------------
; fp_compare
;
; Entry: The value to compare against is in F1, and (AY) points to the value
; to compare with
;
; Uses:
;
; Exit:
; 	A = 0, 		if FP1 == (AY)
; 	A = 1,		if FP1 >  (AY)
; 	A = $FF,	if FP1 <  (AY)
;

fp_compare:
	sta flt_vecl				; save vector, low byte
	sty	flt_vech				; save vector, high byte
	
	ldy #$0						; clear index
	lda (flt_vecl),Y			; get exponent
	iny							; increment index
	tax							; copy (AY)exponent to X
	beq fp_sign					; if AY exp == 0, return sign as above

	lda (flt_vecl),Y			; get the first byte of the mantissa
	eor flt_m1					; compare to m1 (only interested in sign bit)
	bmi fp_sign1				; if signed not equal, return sign as above
	
	cpx flt_x1					; compare (AY) exponent with FP1 exponent
	bne fp_cmp_diff				; branch if different
	
	lda (flt_vecl),Y			; get the first byte of the mantissa
	ora #$80					; normalise the top bit
	cmp flt_m1					; compare with byte 1 of the mantissa
	bne fp_cmp_diff
	
	iny
	lda (flt_vecl),Y			; get the 2nd byte of the mantissa
	cmp flt_m1+1				; compare with byte 2 of the mantissa
	bne fp_cmp_diff

	iny
	lda (flt_vecl),Y			; get the 2nd byte of the mantissa
	sec
	sbc flt_m1+2				; subtract byte 3 of the mantissa
	beq fp_cmp_rts

fp_cmp_diff:
	lda flt_m1					; get the sign bit
	and #$80
	bcc fp_cmp_ret
	
	eor #$ff					; toggle sign
fp_cmp_ret:
	jmp fp_sign2

fp_cmp_rts:
	rts							; just return
	
; ---------------------------------------------------------------------------
; fp_sign
;
; Entry: Computes the sign of FP1
;
; Exit:
; 	A = 0, 		if FP1 == (AY)
; 	A = 1,		if FP1 >  (AY)
; 	A = $FF,	if FP1 <  (AY)

fp_sign:
	lda flt_x1					; get FP1 exponent
	beq fp_sign_rts				; if 0, just return (A=0)

fp_sign1:
	lda flt_m1					; load the byte with the sign in

fp_sign2:
	rol	a						; move sign bit to carry
	lda #$ff					; set byte for -ve result
	bcs fp_sign_rts				; and return if we're -ve
	lda #$01					; set byte for +ve result

fp_sign_rts:
	rts							; return
	


flt_err:
	.byte 0						; floating point error value, 0 = success


f_r22:
	.byte $80, $5a, $82, $7a	; sqrt(2)	.float 1.4142136

f_c:
	.byte $80, $6a, $08, $66	; .float 1.6567626

f_mb:
	.byte $81, $ab, $86, $49	; .float -2.6398577

f_ln10:
	.byte $7e, $6f, $2d, $ed	; .float 0.4342945

f_le2:
	.byte $7f, $58, $b9, $0c	; log (base e) of 2 .float 0.69314718
	
f_a1:
	.byte $80, $52, $80, $40	; .float 1.2920074

f_mhlf:
	.byte $7f, $40, $00, $00	; .float 0.5
	
; exp

f_l2e:
	.byte $80, $5c, $55, $1e	; log (base 2) of e 	.float 1.4426950409
f_a2:
	.byte $86, $57, $6a, $e1	; .float 87.417497202
f_b2:
	.byte $89, $4d, $3f, $1d	; .float 617.9722695
f_c2:
	.byte $7b, $46, $fa, $70	; .float 0.03465735903
f_d:
	.byte $83, $4f, $a3, $03	; .float 9.9545957821
		
f_million:
	.byte $93, $7a, $12, $00	; .float 1000000

f_maxDec:
	.byte $90, $61, $a7, $fc	; .float 99999.9375
	
f_maxNonSci:
	.byte $93, $7a, $11, $fb	; .float 999999.4375
f_ten:
	.byte $83, $50, $00, $00	; .float 10

f_asc1:
	.byte 	$0
f_asc2:
	.byte 	$0
f_asc3:
	.byte	$FE,$79,$60		; -100000
	.byte	$00,$27,$10		; 10000
	.byte	$FF,$FC,$18		; -1000
	.byte	$00,$00,$64		; 100
	.byte	$FF,$FF,$F6		; -10
	.byte	$00,$00,$01		; 1

f_asc2 = f_asc3 - 1
f_asc1 = f_asc2 - 1
