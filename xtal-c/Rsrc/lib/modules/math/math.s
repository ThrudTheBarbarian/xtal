;
; Floating math support for xtal based on Rankin / Woz code
;

f_x1		= $a4				; Exponent for floating reg #1
f_m1		= $a5				; Mantissa for floating reg #1

f_x2		= $a0				; Exponent for floating reg #2
f_m2		= $a1				; Mantissa for floating reg #2

f_sign		= $a8				; sign storage
f_int		= $a9				; integer flag

f_e			= $b0				; scratch workspace
f_z			= $b4				; scratch workspace
f_t			= $b8				; scratch workspace
f_sExp		= $bc				; scratch workspace

f_err		= $89				; Error-flag register




	ldx #16
init:
	lda #0
	sta f_e,x
	dex
	bpl init
	

	ldx #3
setup:
	lda one,x
	sta f_x2,x
	lda two,x
	sta f_x1,x
	dex
	bpl setup
	
	jsr fdiv
	
	brk

one:
	.byte $88, $40, $03, $00
two:
	.byte $8e, $01, $03, $00

; ---------------------------------------------------------------------------
; Natural log
;
; Computes the natural log of the 16-bit int value in M1 and places the
; floating-point result in M1

;.function logf
;.clobber x,y,a


logf:
	lda	f_m1
	beq error					; Can't take log(0)
	bpl ok						; Can work with >0

error:
	lda #1
	sta f_err
	rts

ok:
	jsr swap					; move integer input to M2
	
	ldx #0						; load x for high byte of exponent
	lda f_x2					; Get low byte of input
	ldy #$80					; set y to the value of a zero exponent
	sty f_x2					; set exponent 2 to 0
	eor #$80					; complement the sign bit of the input
	sta f_m1+1					; store in fp1-low-byte-if-int
	
	bpl negexp					; is exponent negative ?
	dex							; yes, set X to $FF
	stx f_m1					; set upper byte of exponent
	
negexp:
	jsr float					; convert arg to float
	
	ldx #3						; set up for a 4-byte transfer
sexp1:
	lda f_x2,x
	sta f_z,x					; copy mantissa to f_z
	lda f_x1,x
	sta	f_sExp,x				; save exponent in f_sExp
	lda f_r22,x
	sta	f_x1,x					; store sqrt(2) in f_x1
	dex
	bpl sexp1

	jsr fsub					; Z - SQRT(2)

	ldx #3						; set up for 4-byte transfer
savet:
	lda f_x1,x					; save exp/mant1 as T
	sta f_t,x
	lda f_z,x					; load exp/mant1 as Z
	sta f_x1,x
	lda f_r22,x					; load exp/mant2 with sqrt(2)
	sta f_x2,x
	dex
	bpl savet					; loop until done
	
	jsr fadd					; Z + SQRT(2)

	ldx #3						; set up for 4-byte transfer
tm2:
	lda f_t,x					; load T into exp/mant2
	sta f_x2,x
	dex
	bpl tm2
	
	jsr fdiv					; T = (Z-SQRT(2)) / (Z + SQRT(2))
	
	ldx #3						; set up for 4-byte transfer
mit:
	lda f_x1,x					; copy exp/mant1 to T and..
	sta f_t,x					; load exp/mant2 with T
	sta f_x2,x
	dex
	bpl mit

	jsr fmul					; T = T*T
	jsr swap					; move T*T to exp/mant2
	
	ldx #3						; set up for 4-byte transfer
mic
	lda f_c,x
	sta f_x1,x					; load exp/mant1 with C
	dex
	bpl mic						; loop until done
	
	jsr fsub					; T*T-C
	
	ldx #3						; set up for 4-byte transfer
m2mb:
	lda f_mb,x					; load exp/mant2 with mb
	sta f_x2,x
	dex
	bpl m2mb
	
	jsr fdiv					; MB / (T*T-C)
	
	ldx #3						; set up for 4-byte transfer
m2a1:
	lda f_a1,x					; load exp/mant2 with a1
	sta f_x2,x
	dex
	bpl m2a1

	jsr fadd					; A1 + MB / (T*T-C)

	ldx #3						; set up for 4-byte transfer
m2t:
	lda f_t,x					; load exp/mant2 with T
	sta f_x2,x
	dex
	bpl m2t
	
	jsr fmul					; (MB/(T*T-C)+A1)*T
	
	ldx #3						; set up for 4-byte transfer
m2mhl:
	lda f_mhlf,x				; load exp/mant2 with 0.5
	sta f_x2,x
	dex
	bpl m2mhl

	jsr fadd 					; (MB/(T*T-C)+A1)*T + 0.5

	ldx #3						; set up for 4-byte transfer
ldexp:
	lda f_sExp,x				; load exp/mant2 with original Exponent
	sta f_x2,x
	dex
	bpl ldexp

	jsr fadd					; + orig exponent

	ldx #3						; set up for 4-byte transfer
mle2:
	lda f_le2,x					; load exp/mant2 with LN(2)
	sta f_x2,x
	dex
	bpl mle2

	jsr fmul					; * LN(2)
	rts							; return result in mant/exp1
	
	

; ---------------------------------------------------------------------------
; log base 10
;
log10:
	jsr logf					; compute natural log
	ldx #3						; set up for 4-byte transfer
l10:
	lda f_ln10,x				; load exp/mant2 with 1/ln(10)
	sta f_x2,x
	dex
	bpl l10
	jsr fmul					; log10(x) = ln(x)/ln(10)
	rts

; ---------------------------------------------------------------------------
; Exponent
;

exp:
	ldx #3						; set up for 4-byte transfer
	lda f_l2e,x					; load exp/mant2 with log(base 2) of E
	sta f_x2,x
	dex
	bpl exp+2

	jsr fmul					; log2(e) * x
	
	ldx #3						; set up for 4-byte transfer
fsa:
	lda f_x1,x					; copy exp/mant1 to Z
	sta f_z,x
	dex
	bpl fsa
	
	jsr fix						; convert contents of exp/mant2 to an int
	
	lda f_m1+1					; save result into 'f_int'
	sta f_int
	
	sec
	sbc #124					; int val - 124
	lda f_m1
	sbc #0
	bpl ovflw

	clc
	lda f_m1+1
	adc #120					;  add 120 to int
	lda f_m1
	adc #0
	bpl contin					; if result positive, continue
	
	lda #0						; int < -120, set result to 0 and return
	ldx #3
zero:
	sta f_x1,x
	dex
	bpl zero
	rt

ovflw:
	lda #3
	sta f_err
	rts

contin:
	jsr float					; convert int to float
	ldx #3						; set up for 4-byte transfer
entd:
	lda f_z,x					; load exp/mant2 with z
	sta f_x2,x
	dex
	bpl entd
	
	jsr fsub					; z=z-float(int)
	
	ldx #3						; set up for 4-byte transfer
zsav:
	lda f_x1,x					; save exp/mant1 ..
	sta f_z,x					; in z ..
	sta f_x2,x					; and x2
	dex
	bpl zsav

	jsr fmul					; z = z*z
	
	ldx #3						; set up for 4-byte transfer
la2:
	lda f_a2,x					; load exp/mant2 with a2
	sta f_x2,x
	lda f_x1,x					; save exp/mant1 as f_sExp
	sta f_sExp,x
	dex
	bpl la2

	jsr fadd					; z*z + a2
	
	ldx #3						; set up for 4-byte transfer
lb2:
	lda f_b2,x					; load exp/mant2 with b2
	sta f_x2,x
	dex
	bpl lb2
	
	jsr fdiv					; B / (Z*Z + A2)
	
	ldx #3						; set up for 4-byte transfer
dload
	lda f_x1,x					; save exp/mant1 as T
	sta f_t,x
	lda f_c2,x					; load exp/mant1 with C2
	sta f_x1,x
	lda f_sExp,x				; load exp/mant2 with sExp
	sta f_x2,x
	dex
	bpl dload

	jsr fmul					; Z * Z * C2
	jsr swap					; swap exp/mant1 with exp/mant2
	
	ldx #3						; set up for 4-byte transfer
ltmp:
	lda f_t,x					; load exp/mant1 with T
	sta f_x1,x
	dex
	bpl ltmp
	
	jsr fsub					; C2*Z*Z-B2/(Z*Z+A2)

	ldx #3						; set up for 4-byte transfer
ldd:
	lda f_d,x					; load exp/mant2 with D
	sta f_x2,x
	dex
	bpl ldd
	
	jsr fadd					; D+C2*Z*Z-B2/(Z*Z+A2)
	jsr swap					; swap exp/mant1 with exp/mant2

	ldx #3						; set up for 4-byte transfer
lfa:
	lda f_z,x					; load exp/mant1 with Z
	sta f_x1,x
	dex
	bpl lfa
	
	jsr fsub					; -Z+D+C2*Z*Z-B2/(Z*Z+A2)

	ldx #3						; set up for 4-byte transfer
lf3:
	lda f_z,x					; load exp/mant2 with Z
	sta f_x2,x
	dex
	bpl lf3

	jsr fdiv					; Z/(-Z+D+C2*Z*Z-B2/(Z*Z+A2))

	ldx #3						; set up for 4-byte transfer
ld12:
	lda f_mhlf,x				; load exp/mant2 with 0.5
	sta f_x2,x
	dex
	bpl ld12
	
	jsr fadd					; Z/(-Z+D+C2*Z*Z-B2/(Z*Z+A2)) + 0.5
	
	sec							; add int to exponent with carry set
	lda f_int					; to multiply by
	adc f_x1					; 2**(int+1)
	sta f_x1					; return result to Exponent
	rts							; return (.5+Z/(-Z+D+C2*Z*Z-B2/(Z*Z+A2))*2**(INT+1)
	
	
	
	
	
	
	
; ---------------------------------------------------------------------------
; Internal routine - Add two aligned mantissas

add:
	clc							; clear carry
	ldx #2						; set up for 3-byte add
add1:
	lda f_m1,x					; add a byte of ..
	adc f_m2,x					; mant2 to mant1
	sta f_m1,x
	dex
	bpl add1
	rts

		
; ---------------------------------------------------------------------------
; Internal routine - Take absolute value of mantissas 1 and 2

md1:
	asl f_sign					; clear bit1 of sign
	jsr abswap					; absolute value of mant1, then swap mant2

abswap:
	bit f_m1					; is mant1 negative ?
	bpl abswp1					; no: swap with mant2 and return
	jsr fcompl					; yes: complement it
	inc f_sign					; increment sign, complementing lsb

abswp1:
	sec							; set carry for return to mul/div
	; fall through to swap
		
; ---------------------------------------------------------------------------
; Internal routine - swap Exp/Mant1 to Exp/Mant2, and mant1->f_e

swap:
	ldx #$04					; set up for 4-byte swap
swap1:
	sty f_e-1,x					; copy to E
	lda f_x1-1,x				; swap a byte of exp/mant1 with..
	ldy f_x2-1,x				; exp/mant2 and leave a copy of..
	sty f_x1-1,x				; mant1 in E (3 bytes). E+3 used
	sta f_x2-1,x
	dex							; next byte
	bne swap1					; loop until done
	rts
	
		
; ---------------------------------------------------------------------------
; Internal routine - convert a 16-bit integer into a float representation
; note that the high-byte of the integer is expected in f_m1 and the low
; byte is expected in f_m1+1 (the opposite way around to normal)

float:
	lda #$8E
	sta f_x1					; Set exponent-1 to 14 dec
	lda #0
	sta f_m1+2					; clear lowest byte of m1 mantissa
	beq norm					; normalise the result

norm1:
	dec f_x1					; decrement exponent-1
	
	asl f_m1+2					; 3-byte shift of mantissa-1 left
	rol f_m1+1
	rol f_m1
	
norm:
	lda f_m1					; Mantissa-1 high-byte
	asl							; upper 2 bits unequal ?
	eor f_m1
	bmi rts1					; yes, return with mantissa-1 normalised
	lda f_x1					; exponent-1 = 0 ?
	bne norm1					; no, continue normalising
rts1:
	rts							; all done


; ---------------------------------------------------------------------------
; fadd / fsub

fsub:
	jsr fcompl					; Complement mantissa, clears carry unless 0
swpalg:
	jsr algnsw					; Right shift M1 or swap with M2 on carry
	
fadd:
	lda f_x2					; compare exp1 with exp2
	cmp f_x1
	bne swpalg					; if unequal swap addends, or align mantissas
	
	jsr add						; Add aligned mantissas

addend:
	bvc norm					; no overflow, normalise results
	bvs rtlog					; ov: shift mant1 right, note carry is correct sign

algnsw:
	bcc swap					; swap if carry clear, else shift right

rtar:
	lda f_m1					; sign of mantissa-1 into carry ..
	asl							; for right arithmetic shift

rtlog:
	inc f_x1					; inc x1 to compensate for right-shift
	beq ovfl					; exp1 out of range

rtlog1:
	ldx #$fa					; index for 6-byte right-shift
ror1:
	lda #$80
	bcs ror2
	asl
ror2:
	lsr f_e+3,x					; simulate ror E+3,x
	ora f_e+3,x
	sta f_e+3,x
	inx							; next byte of shift
	bne ror1					; loop until done
	rts
	
; ---------------------------------------------------------------------------
; fmul

fmul:
	jsr md1						; Abs value of mant1 and mant2
	adc f_x1					; add exp1 to exp2 for product exponent
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
	lsr f_sign					; test sign (even/odd)

normx:
	bcc norm					; if even, normalise product, else complement

fcompl:
	sec							; set the carry
	ldx #$03					; set up for 3 byte subtraction
compl1:
	lda #0						; clear A
	sbc f_x1,x					; subtract byte of exp1
	sta f_x1,x					; restore it
	dex							; next byte
	bne compl1					; loop until done
	beq addend					; normalise (or shift right on overflow)

	
; ---------------------------------------------------------------------------
; fdiv

fdiv:
	jsr md1						; Take abs value of mant1, mant2
	sbc f_x1					; subtract exp1 from exp2
	jsr md2						; Save as quotient exp
div1:
	sec							; set carry for subtract
	ldx #$02					; set up for 3-byte subtract
div2:
	lda f_m2,x					; subtract a byte of E ..
	sbc f_e,x					; from mant2
	pha
	dex							; next byte
	bpl div2					; and loop until done
	
	ldx #$fd					; set up for 3-byte conditional move
div3:
	pla							; pull a byte of difference off the stack
	bcc div4					; if mant2 < E, then don't restore mant2
	sta f_m2+3,x
div4:
	inx							; next less-significant byte
	bne div3					; loop until done

	rol f_m1+2					; roll quotient ..
	rol f_m1+1					; left, carry ..
	rol f_m1					; into lsb
	asl f_m2+2
	rol f_m2+1					; shift dividend left
	rol f_m2
	bcs ovfl					; overflow is due to unnormalised divisor
	dey							; next divide iteration
	bne div1					; loop until done, 23 iterations
	beq mdend					; normalise quotient and correct sign

md2:
	stx f_m1+2					; clear mantissa (3 bytes) for ..
	stx f_m1+1					; mul/div
	stx f_m1
	bcs ovchk					; if exp calc, set carry, check for overflow
	bmi md3						; if neg, no underflow
	pla 						; pop one
	pla							; return level
	bcc normx					; clear x1 and return

md3:
	eor #$80					; complement sign bit of exponent
	sta f_x1					; store it
	ldy #$17					; count for 24 mul or 23 div iterations
	rts							; return

ovchk:
	bpl md3						; if positive exponent, then no overflow
	
ovfl:
	lda #2
	sta f_err
	rts

	jsr rtar					; shift mant1 right and inc Exponent
fix:
	lda f_x1					; check Exponent
	cmp #$8E					; is exponent 14 ?
	bne fix-3					; no, shift
rtrn:
	rts
	

	
f_r22:
	.byte $80, $5a, $02, $7a	; sqrt(2)	.float 1.4142136

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
		
	
