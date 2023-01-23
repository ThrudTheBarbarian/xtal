;
; Maths floating point routines
; =============================
;
; Format:
; -------
;
;Laid out in memory as below:
;
;         +--------+  +--------+  +---------+  +--------+
;         |  LSB   |  |Mantissa|  |   MSB   |  |Exponent|
;         |   (n)  |  |  (n+1) |  |  (n+2)  |  |  (n+3) |
;         |MMMMMMMM|  |MMMMMMMM|  |S.MMMMMMM|  |SEEEEEEE|
;         +--------+  +--------+  +---------+  +--------+;
;
; Definitions:
; ------------

FP_fmpnt        = $1C       ; From pointer (2 bytes)
FP_topnt        = $1E       ; To pointer   (2 bytes)

FP_lswe         = $80       ; FP accumulator extension
FP_lsw          = $81       ; FP accumulator least significant byte
FP_nsw          = $82       ; FP accumulator next significant byte
FP_msw          = $83       ; FP accumulator most significant byte
FP_acce         = $84       ; FP accumulator exponent

FP_mcand0       = $85       ; Multiplicand work area. Note that these ..
FP_mcand1       = $86       ; .. three bytes must be located immediately ..
FP_mcand2       = $87       ; .. before the FPop space

FP_olswe        = $88       ; FP op extension
FP_olsw         = $89       ; FP op least significant byte
FP_onsw         = $8A       ; FP op next significant byte
FP_omsw         = $8B       ; FP op most signifcant byte
FP_oexp         = $8C       ; FP op exponent

FP_err			= $8D		; Error codes

FP_cntr         = $F0       ; Counter
FP_tsign        = $F1       ; Sign indicator
FP_signs        = $F2       ; Signs indicator (mult and div)
FP_work0        = $F3       ; Work area
FP_work1        = $F4       ; Work area
FP_work2        = $F5       ; Work area
FP_work3        = $F6       ; Work area
FP_work4        = $F7       ; Work area
FP_work5        = $F8       ; Work area
FP_work6        = $F9       ; Work area
FP_work7        = $FA       ; Work area

FP_svec			= $FC		; Pointer to string
FP_sindx		= $FE
FP_iocnt		= $FF		; index into string

; We can use the ZIOCB space, as long as there's no actual i/o taking
; place, and as long as the values stored there don't need to persist
; outside of the function they're being used in.

FP_inmtas		= $20		; to f/a mantissa sign
FP_inexps		= $21		; to f/a exponent sign
FP_inprdi		= $22		; to f/a period indicator
FP_iolsw		= $23		; to f/a LSB
FP_ionsw		= $24		; to f/a mid-SB
FP_iomsw		= $25		; to f/a MSB
FP_ioexp		= $26		; to f/a exponent
FP_iostr		= $27		; to f/a storage
FP_iostr1		= $28		; to f/a storage
FP_iostr2		= $29		; to f/a storage
FP_iostr3		= $2A		; to f/a storage
FP_ioexpd		= $2B		; to f/a exponent storage
FP_tplsw		= $2C		; temp input storage, LSB
FP_tpnsw		= $2D		; temp input storage, mid-SB
FP_tpmsw		= $2E		; temp input storage, MSB
FP_tpexp		= $2F		; temp input storage, exponent
FP_temp1		= $FB		; temp storage for pages

FP_PAGE0        = $00       ; Used with indexed addressing

FLT_DIV0		= $80		; Error: attempt to divide by 0

; =============================================================================
; FP macros

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

.macro _fploadAcc
    _xfer32 %1,FP_lsw
.endmacro

.macro _fploadOp
    _xfer32 %1,FP_olsw
.endmacro


; =============================================================================
init:

lda #<str
sta FP_svec
lda #>str
sta FP_svec+1

jsr FP_stof
brk

two:
.byte $00,$00,$40,$02
nine:
.byte $00,$00,$48,$04
three:
.byte $00,$00,$60,$02

mtwo:
.byte $00,$00,$c0,$02
mnine:
.byte $00,$00,$b8,$04

str:
.byte "0.0045."


; =============================================================================
;
; Routine    : rotatr
; Description: Rotates Y bytes of data right in page 0.
; Notes      : 1) Not exported, just an internal function
;            : 2) On entry, X should point to highest byte to rotate
;              3) On exit, Y=0, X=first byte to rotate

rotatr:
    clc                         ; clear the carry

rotr:
    ror FP_PAGE0,X              ; rotate the byte right
    dey                         ; decrement the byte counter
    bne morrtr                  ; not zero, continue to rotate
    rts
                                ; done
morrtr:
    dex                         ; decrement memory pointer
    jmp rotr                    ; continue to rotate right


; =============================================================================
;
; Routine    : rotatl
; Description: Rotates Y bytes of data left in page 0.
; Notes      : 1) Not exported, just an internal function
;              2) On entry, X should point to first byte to rotate
;              3) On exit, Y=0, X points to last byte to rotate

rotatl:
    clc                         ; clear the carry

rotl:
    rol FP_PAGE0,X              ; rotate the byte right
    dey                         ; decrement the byte counter
    bne morrtl                  ; not zero, continue to rotate
    rts
                                ; done
morrtl:
    inx                         ; Advance memory pointer
    jmp rotl                    ; continue to rotate right


; =============================================================================
;
; Routine    : compl
; Description: Complements Y bytes of data in page 0.
; Notes      : 1) Not exported, just an internal function
;              2) X must point at LSB
;              3) On return, X = MSB + 1

complm:
    sec                         ; set the carry

compl:
    lda #$ff                    ; Load $FF for complementing op
    eor FP_PAGE0,X              ; Complement byte
    adc #0                      ; IF C==1, 2's complement
    sta FP_PAGE0,X              ; Store byte in memory
    inx                         ; Advance memory pointer
    dey                         ; Decrement byte counter
    bne compl                   ; not zero, continue
    rts


; =============================================================================
;
; Routine    : adder
; Description: Variable precision add routine
; Notes      : 1) Precision stored in X
;              2) Y should start at offset from pointer and will increment
;              3) On return, Y = MSB + 1

adder:
    clc                         ; clear the carry

addr1:
    lda (FP_topnt),Y            ; fetch byte from one value
    adc (FP_fmpnt),Y            ; add to byte from other value
    sta (FP_topnt),Y            ; store result
    iny                         ; increment index pointer
    dex                         ; decrement counter
    bne addr1                   ; Not zero ? continue!
    rts


; =============================================================================
;
; Routine    : clrmem
; Description: Variable precision clear-memory
; Notes      : 1) Precision stored in X

clrmem:
    lda #0                      ; set up 0-value
    tay                         ; initialise index pointer

clrm1:
    sta (FP_topnt),Y            ; Clear memory location
    iny                         ; increment memory offset
    dex                         ; decrement count
    bne clrm1                   ; Done ? No: loop
    rts                         ; Yes : return

; =============================================================================
;
; Routine    : movind
; Description: Variable precision indirect move
; Notes      : 1) Precision stored in X

movind:
    ldy #0                      ; initialise the index pointer

movin1:
    lda (FP_fmpnt),Y            ; fetch byte to transfer
    sta (FP_topnt),Y            ; store byte in new location
    iny                         ; increment memory offset
    dex                         ; decrement count
    bne movin1                  ; Done ? No: loop
    rts                         ; Yes : return

; =============================================================================
;
; Routine    : swpind
; Description: Variable precision indirect swap
; Notes      : 1) Precision stored in X

swpind:
    ldy #0                      ; initialise the index pointer

swpin1:
    lda (FP_fmpnt),Y            ; fetch byte to transfer
    sta FP_mcand0				; temporary store
    lda (FP_topnt),Y			; fetch byte to swap
    sta	(FP_fmpnt),Y			; store in orig
    lda FP_mcand0				; fetch temporarily stored byte
    sta (FP_topnt),Y            ; store byte in swapped location
    iny                         ; increment memory offset
    dex                         ; decrement count
    bne swpin1                  ; Done ? No: loop
    rts                         ; Yes : return
    
; =============================================================================
;
; Routine    : FP_norm
; Description: Normalises a floating point number so that the first '1'
;              bit of the mantissa is up against where the virtual binary
;              point is.
; Result     : FPAcc = NORMALISE(FPAcc)
;
; Notes      : 1) Treat negative numbers by complementing, normalising and
;                 re-complementing
;              2) Must check for mantissa == 0, else we'll be forever in a
;                 loop looking for the first '1' bit
; Algorithm  :
;
;                           +------------+
;                           |   Start    |
;                           +------------+
;                                  |
;                                  v
;                           +------------+
;                           |FPacc -ve ? |-------------+
;                           +------------+             v
;                                  |             +-----------+
;                                  |             |Complement |
;                                  |             |   FPacc   |
;                                  v             +-----------+
;                           +------------+             |
;                +----------|FPacc == 0 ?|<------------+
;                |          +------------+
;                |                 |
;                |                 v
;                |          +------------+
;                v          | Normalise  |
;          +-----------+    +------------+
;          | Exponent  |           |
;          |   => 0    |           v
;          +-----------+    +------------+
;                |          |FPacc -ve ? |------------+
;                |          +------------+            v
;                |                 |            +-----------+
;                |                 |            |Complement |
;                |                 |            |   FPacc   |
;                |                 |            +-----------+
;                |                 v                  |
;                |          +------------+            |
;                +--------->|   Return   |<-----------+
;                           +------------+
;
; -----------------------------------------------------------------------------

@FP_norm:
    ldx #FP_tsign               ; set pointer to sign register
    lda FP_msw                  ; fetch Acc MSB
    bmi accmin                  ; if negative, branch
    ldy #0                      ; if positive, clear sign register
    sty FP_PAGE0,X              ; by storing 0
    jmp aczert                  ; then test if FPACC is 0

accmin:
    sta FP_PAGE0,X              ; set sign indicator if -ve
    ldy #$4                     ; set precision counter
    ldx #FP_lswe                ; set pointer to Acc LSB -1
    jsr complm                  ; 2's complement Acc

aczert:
    ldx #FP_msw                 ; set pointer to Acc MSB
    ldy #4                      ; set precision counter

look0:
    lda FP_PAGE0,X              ; see if Acc == 0
    bne acnonz                  ; branch if non-zero
    dex                         ; decrement index pointer
    dey                         ; decrement byte counter
    bne look0                   ; if counter not 0, next byte
    sty FP_acce                 ; Acc = 0, clear exponent too

normex:
    rts                         ; exit normalisation routine

acnonz:
    ldx #FP_lswe                ; set pointer to Acc LSB -1
    ldy #4                      ; set precision counter
    jsr rotatl                  ; rotate Acc left
    lda FP_PAGE0,X              ; see if 1 in MS bit
    bmi accset                  ; if minus, we're now normalised
    dec FP_acce                 ; if +ve, decrement exponent
    jmp acnonz                  ; .. and continue to rotate

accset:
    ldx #FP_msw                 ; set pointer to ACC MSB
    ldy #3                      ; set precision counter
    jsr rotatr                  ; compensating rotate-right of acc
    lda FP_tsign                ; Was original sign  +ve ?
    beq normex                  ; Yes ? Just return
    
    ldy #3                      ; set precision counter
    jmp complm                  ; restore Acc to -ve and return




; =============================================================================
;
; Routine    : FP_add
; Description: Adds the FP accumulator and FPop registers, leaving the result
;            : in the accumulator
; Result     : FPAcc = FPAcc + FPop
;
; Notes      : 1) If either value is 0, then just return because X + 0 = X
;              2) If the exponents differ by at least 23, there is no point in
;                 adding, so just return immediately as well
; Algorithm  :
;
;                         +------------+
;                         |   Start    |
;                         +------------+
;                                |
;                                v
;                         +------------+
;              +----------|FPacc == 0 ?|
;              |          +------------+
;              v                 |
;     +-----------------+        |
;     |Set FPAcc to FPop|        |
;     +-----------------+        |
;              |                 v
;              |          +------------+
;              +----------|FPop == 0 ? |
;              |          +------------+
;              |                 |
;              |                 v
;              |        +-----------------+
;              |        |Exponents match ?|---------+
;              |        +-----------------+         |
;              |                 |                  v
;              |                 |       +--------------------+
;              |                 |       |Exponents in range ?|---+
;              |                 |       +--------------------+   |
;              |                 |                  |             |
;              |                 |                  v             |
;              |                 |       +--------------------+   |
;              |                 |       |  Align smaller to  |   |
;              |                 |       |       larget       |   |
;              |                 |       +--------------------+   |
;              |                 |                  |             |
;              |                 +------------------+             |
;              |                 |                                |
;              |                 v                                |
;              |        +-----------------+                       |
;              |        |Add FPop to FPAcc|                       |
;              |        +-----------------+                       |
;              |                 |                                |
;              |                 v                                |
;              |        +-----------------+     +-------------+   |
;              |        | Normalise FPAcc |     |Set FPAcc to |   |
;              |        +-----------------+     |larger value |<--+
;              |                 |              +-------------+
;              |                 v                     |
;              |          +------------+               |
;              +--------->|   Return   |<--------------+
;                         +------------+
;

@FP_add:
    lda FP_msw                  ; see if Acc MSB == 0
    bne nonzac                  ; branch if not zero
    
movop:
    ldx #FP_olsw                ; set pointer to FPop, LSB
    stx FP_fmpnt                ; save in from-pointer
    ldx #FP_lsw                 ; set pointer to FPAcc, LSB
    stx FP_topnt                ; save in to-pointer
    lda #0                      ; high-bytes are 0
    sta FP_fmpnt+1              ; store in page-part of from-pointer
    sta FP_topnt+1              ; .. and to-pointer
    
    ldx #4                      ; set precision counter
    jmp movind                  ; move FPop to FPacc and return

nonzac:
    lda FP_omsw                 ; see if FPop MSB == 0
    bne ckeqex                  ; no, branch to check exponents being equal
    rts                         ; yes: return

ckeqex:
    ldx #FP_acce                ; set pointer to FPAcc exponent
    lda FP_PAGE0,X              ; load the FPAcc exponent
    cmp FP_oexp                 ; compare with the FPop exponent
    beq shacop                  ; branch to add routine if equal
    sec                         ; if not, determine which is larger
    lda #0                      ; form the two's complement of ..
    sbc FP_PAGE0,X              ; .. the FPAcc's exponent
    adc FP_oexp                 ; add the FPop exponent
    bpl skpneg                  ; if +, FPop > FPAcc
    sec                         ; if -, form 2's complement ..
    sta FP_work0                ; .. of the result
    lda #0                      ; this will be used to test the ..
    sbc FP_work0                ; .. the magnitude of the exponent difference

skpneg:
    cmp #$18                    ; is difference less than 18 hex
    bmi lineup                  ; if so, align the mantissas
    sec                         ; if not, id FPop > FPAcc
    lda FP_oexp                 ; this is tested by comparing the ..
    sbc FP_PAGE0,X              ; .. exponents of each
    bpl movop                   ; FPop larger, move FPop to FPAcc
    rts                         ; FPAcc larger, return

lineup:
    lda FP_oexp                 ; fetch FPop exponent
    sec                         ; prepare to subtract
    sbc FP_PAGE0,X              ; do FPop - FPAcc exponents
    tay                         ; save difference in Y
    bmi shifto                  ; if -ve: FPacc is > FPop, shift FPop

moracc:
    ldx #FP_acce                ; set pointer to FPAcc exponent
    jsr shloop                  ; shift FPAcc right, 1 bit
    dey                         ; decrement difference counter
    bne moracc                  ; if counter != 0, redo
    jmp shacop                  ; when 0, start add operation

shifto:
    ldx #FP_oexp                ; set pointer to FPop exponent
    jsr shloop                  ; shift FPop right, 1 bit
    iny                         ; increment difference counter
    bne shifto                  ; if counter != 0, redo

shacop:                         ; note that Y=0 on entry here
    lda #0                      ; prepare for actual addition
    sta FP_lswe                 ; clear FPAcc LSB - 1
    sta FP_olswe                ; clear FPop LSB - 1
    ldx #FP_acce                ; set pointer to FPAcc exponent
    jsr shloop                  ; rotate FPAcc right to allow for overflow
    ldx #FP_oexp                ; set pointer to FPop exponent
    jsr shloop                  ; rotate FPop right to keep alignment
    
    ldx #FP_olswe               ; set pointer to FPop LSB - 1
    stx FP_fmpnt                ; store in from-pointer
    ldx #FP_lswe                ; set pointer to FPAcc LSB - 1
    stx FP_topnt                ; store in to-pointer
    ldx #4                      ; set precision counter
    jsr adder                   ; add FPop to FPAcc
    jmp FP_norm                 ; normalise the result and return

shloop:
    inc FP_PAGE0,X              ; increment exponent value
    dex                         ; decrement pointer
    tya                         ; save difference counter
    ldy #$4                     ; set precision counter

fshift:
    pha                         ; store difference counter on stack
    lda FP_PAGE0,X              ; fetch MSB of value
    bmi bring1                  ; if -ve, must rotate one in MSB
    jsr rotatr                  ; +ve: rotate value right, 1 bit
    jmp rescnt                  ; return to caller

bring1:
    sec                         ; set carry to maintain -ve state
    jsr rotr                    ; rotate value right, 1 bit

rescnt:
    pla                         ; fetch difference counter
    tay                         ; restore in Y
    rts                         ; return
    
    
    

; =============================================================================
;
; Routine    : FP_sub
; Description: This is just a call into FP_add, after complementing the value
;              to subtract.
; Result     : FPAcc = FPop - FPAcc
;

@FP_sub:
    ldx #FP_lsw                 ; set pointer to FPAcc LSB
    ldy #3                      ; set precision counter
    jsr complm                  ; complement the accumulator
    jmp FP_add                  ; "add" the values together

    
    
    

; =============================================================================
;
; Routine    : FP_mul
; Description: Rotate through all powers of 2 in the multiplicand and add
;              appropriate values as we go
; Result     : FPAcc = FPop * FPAcc
;
; Notes      : 1) Uses partial product register to store info
;              2) Eventual sign is calculated from input signs
;			   3) Uses 6-byte register for accumulator then normalises, so
;				  needs the memory layout of multiplicand work-space as
;				  laid out in register arrangement in ZP
; Algorithm  :
;
;             +------------------+
;             |      Start       |
;             +------------------+
;                       |
;                       v
;             +------------------+
;             | Shift multiplier |
;        +--->|right, placing LSB|
;        |    |     in Carry     |
;        |    +------------------+
;        |              |
;        |              v
;        |    +------------------+Yes
;        |    |   Carry == 1 ?   |--------------+
;        |    +------------------+              v
;        |            No|            +--------------------+
;        |              |            |Add multiplicand to |
;        |              |            |  partial product   |
;        |              v            +--------------------+
;        |    +------------------+              |
;        |    |  Shift partial   |              |
;        |    |  product right   |<-------------+
;        |    +------------------+
;        |              |
;        |              v
;        |    +------------------+
;        |    |   All bits of    |
;        +----|    multiplier    |
;           No|    checked ?     |
;             +------------------+
;                       | Yes
;                       v
;             +------------------+
;             |Return with answer|
;             |in partial product|
;             +------------------+
;

@FP_mul:
    jsr cksign                  ; set up and check sign of mantissa
    lda FP_oexp                 ; get FPop exponent
    clc                         ; add FPAcc exponent ..
    adc FP_acce                 ; .. to FPop exponent
    sta FP_acce                 ; store in FPAcc exponent
    inc FP_acce                 ; inc by 1 for algorithm compensation

setmct:
    lda #$17                    ; set bit counter
    sta FP_cntr                 ; store bit counter

multip:
    ldx #FP_msw                 ; Set pointer to FPAcc MSB
    ldy #3                      ; set precision counter
    jsr rotatr                  ; rotate FPAcc right, 1 bit
    bcc nadopp                  ; C==0 ? Don't add partial-product

addopp:
    ldx #FP_mcand1              ; pointer to LSB of multiplicand
    stx FP_fmpnt                ; store from-pointer
    ldx #FP_work1               ; pointer to LSB of partial product
    stx FP_topnt                ; store to-pointer
    ldx #$6                     ; set precision counter
    jsr adder                   ; add multiplicand to partial product

nadopp:
    ldx #FP_work6               ; set pointer to MSB of partial product
    ldy #$6                     ; set precision counter
    jsr rotatr                  ; rotate partial product right, 1 bit
    dec FP_cntr                 ; decrement the bit counter
    bne multip                  ; not zero, continue multiplying
    ldx #FP_work6               ; else, set pointer to partial product MSB
    ldy #6                      ; set precision counter
    jsr rotatr                  ; make room for possible rounding
    ldx #FP_work3                ; set pointer to bit 24 of partial product
    lda FP_PAGE0,X              ; fetch LSB-1 of result
    rol A                       ; rotate 24th bit to sign
    bpl prexfr                  ; if 24th bit == 0, branch ahead
    clc                         ; clear carry for addition
    ldy #3                      ; set precision counter
    lda #$40                    ; add 1 to 23rd bit of partial product ..
    adc FP_PAGE0,X              ; .. to round off result
    sta FP_work3                ; store sum in memory
    
cround:
    lda #0                      ; clear A without changing carry
    adc FP_PAGE0,X              ; add with carry to propagate
    sta FP_PAGE0,X              ; store in partial product
    inx                         ; increment index pointer
    dey                         ; decrement counter
    bne cround                  ; not zero, add next byte

prexfr:
    ldx #FP_lswe                ; set pointer to FPAcc LSB-1
    stx FP_topnt                ; store in to-pointer
    ldx #FP_work3               ; set pointer to partial product LSB-1
    stx FP_fmpnt                ; store in from-pointer
    ldx #4                      ; set precision counter
    
exmldv:
    jsr movind                  ; move partial product to FPAcc
    jsr FP_norm                 ; normalise result
    lda FP_signs                ; get sign storage
    bne multex                  ; if != 0, sign is +ve
    ldx #FP_lsw                 ; else set pointer to FPAcc LSB
    ldy #$3                     ; set precision pointer
    jsr complm                  ; complement result

multex:
	lda #0						; set the error status
	sta FP_err					; and store for caller
    rts                         ; return to sender

cksign:
    lda #0                      ; set page portion of pointers
    sta FP_topnt+1              ; zero page of to-pointer
    sta FP_fmpnt+1              ; zero page of from-pointer
    
    lda #FP_work0               ; set pointer to work area
    sta FP_topnt                ; store in to-pointer
    ldx #8                      ; set precision pointer
    jsr clrmem                  ; clear memory
    
    lda #FP_mcand0              ; set pointer to multiplicand storage
    sta FP_topnt                ; store in to-pointer
    ldx #4                      ; set precision pointer
    jsr clrmem                  ; clear multiplicand storage
    
    lda #$1                     ; initialise sign indicator..
    sta FP_signs                ; .. by storing 1 in signs
    
    lda FP_msw                  ; fetch FPAcc MSB
    bpl opsgnt                  ; +ve, check FPop

negfpa:
    dec FP_signs                ; -ve: decrement signs
    ldx #FP_lsw                 ; set pointer to FPAcc LSB
    ldy #3                      ; set precision counter
    jsr complm                  ; make +ve for multiplication
    
opsgnt:
    lda FP_omsw                 ; is FPop -ve ?
    bmi negop                   ; yes ? complement value
    rts

negop:
    dec FP_signs                ; decrement signs indicator
    ldx #FP_olsw                ; set pointer to FPop LSB
    ldy #3                      ; set precision counter
    jmp complm                  ; complement and return


    

; =============================================================================
;
; Routine    : FP_div
; Description: Opposite of FP_mul above, a series of shift/subtract ops instead
;              of shift/add
; Result     : FPAcc = FPop / FPacc
; Notes      :
;
; Algorithm  :
;                                    +------------+
;                                    |   Start    |
;                                    +------------+
;                                           |
;                                           v
;          +-----------------+    Yes+------------+
;          | Complement Facc |<------|FPacc < 0 ? |
;          +-----------------+       +------------+
;                   |                       | No
;                   +-----------------------+
;                                           v
;          +-----------------+    Yes+------------+
;          | Complement Fop  |<------| FPop < 0 ? |
;          +-----------------+       +------------+
;                   |                       | No
;                   +-----------------------+
;                                           v
;          +-----------------+       +------------+
;          |  Jump to error  |<------|Divisor == 0|
;          +-----------------+       +------------+
;                                           |
;                                           v
;                            +----------------------------+
;                            |     Subtract exponents     |
;                            +----------------------------+
;                                           |
;                                           |
;                                           v
;                            +----------------------------+
;                            |Divide mantissa (shift/sub) |
;                            +----------------------------+
;                                           |
;                                           v
;                            +----------------------------+
;                            | Round off partial product  |
;                            +----------------------------+
;                                           |
;                                           v
;                            +----------------------------+
;                            | Move to FPAcc / normalise  |
;                            +----------------------------+
;                                           |
;                                           v
;                           Yes---------------------------+
;                     +------| Initial signs different ?  |
;                     |      +----------------------------+
;                     v                     | No
;            +-----------------+            |
;            | Complement Facc |            |
;            +-----------------+            v
;                     |               +-----------+
;                     +-------------->|  Finish   |
;                                     +-----------+
;

@FP_div:
    jsr cksign                  ; set up and check sign of mantissa
	lda FP_msw					; check for divide by zero
	beq derror					; Divide-by-zero error
	
subexp:
	lda FP_oexp					; Get dividend exponent
	sec							; Prepare for subtract
	sbc FP_acce					; Subtract divisor exponent
	sta FP_acce					; Store subtracted exponent
	inc FP_acce					; Algorithm compensation
	
setdct:
	lda #$17					; set the divide-count
	sta FP_cntr					; store the counter

divide:
	jsr setsub					; subtract divisor from dividend
	bmi nogo					; if result -ve, rotate vlaue in quotient
	ldx #FP_olsw				; set pointer to dividend
	stx FP_topnt				; store in to-pointer
	ldx #FP_work0				; set pointer to quotient
	stx FP_fmpnt				; store in from-pointer
	ldx #3						; Set precision counter
	jsr movind					; move quotient to dividend
	sec							; set carry for +ve results
	jmp quorot					; rotate quotient

derror:
	lda #FLT_DIV0				; get error code
	sta FP_err					; store in the error register
	rts							; return. Maybe BRK instead ?
	
nogo:
	clc							; Negative result, clear carry

quorot:
	ldx #FP_work4				; Set pointer to quotient LSB
	ldy #$3						; Set precision counter
	jsr rotl					; rotate carry into LSB of quotient
	
	ldx #FP_olsw				; Set pointer to dividend LSB
	ldy #$3						; Set precision counter
	jsr rotatl					; rotate dividend left
	
	dec FP_cntr					; One more bit-division done
	bne divide					; loop if more yet to do
	
	jsr setsub					; Do it one more time for rounding
	bmi dvexit					; if minus, no rounding
	lda #$1						; else (0,+ve) add 1 to 23rd bit
	clc							; prepare for add chain
	adc FP_work4				; Round off quotient LSB
	sta FP_work4				; Restore byte in work area
	lda #$0						; clear A, but not carry
	adc FP_work5				; Add carry to 2nd byte of quotient
	sta FP_work5				; Store result
	lda #$0						; clear A, but not carry
	adc FP_work6				; Add carry to 3rd byte of quotient
	sta FP_work6				; Store result
	bpl dvexit					; if msb == 0, exit
	ldx #FP_work6				; else prepare to rotate right
	ldy #$3						; set precision
	jsr rotatr					; clear sign bit counter
	inc FP_acce					; compensate exponent for rotate
	
dvexit:
	ldx #FP_lswe				; set pointer to FPacc
	stx FP_topnt				; store in the to-pointer
	ldx #FP_work3				; set pointer to quotient
	stx	FP_fmpnt				; store in the from-pointer
	ldx #$4						; set the precision counter
	jmp exmldv					; move quotient to FPAcc, normalise, exit
	
setsub:
	ldx #FP_work0				; set pointer to work area
	stx FP_topnt				; store in to-pointer
	ldx #FP_lsw					; set pointer to FPacc LSB
	stx FP_fmpnt				; store in from-pointer
	ldx #3						; set precision count
	jsr movind					; copy FPAcc to work area
	
	ldx #FP_work0				; prepare for subtraction
	stx FP_topnt				; store work0 into to-pointer
	ldx #FP_olsw				; set pointer to FPop LSB
	stx FP_fmpnt				; store in from-pointer
	ldy #$0						; initialise index pointer
	ldx #3						; set precision count
	sec							; set up for subtraction

subr1:
	lda (FP_fmpnt),Y			; Fetch FPop byte (dividend)
	sbc (FP_topnt),Y			; subtract FPacc byte (divisor)
	sta (FP_topnt),Y			; store in place of divisor
	iny							; increment index pointer
	dex							; decrement precision counter
	bne subr1					; not zero ? continue
	lda FP_work2				; set sign bit result in N flag
	rts
	

; =============================================================================
;
; Routine    : FP_stof
; Description: Converts an (AT)ASCII string to a float number
; Result     : FPAcc = convert(ptr)
; Notes      :

input:
	ldy FP_sindx				; get the index of the byte to fetch
	lda (FP_svec),Y				; fetch the next byte
	inc FP_sindx				; increment the index pointer
	rts

@FP_stof:
    lda #0                      ; set page portion of pointers
    sta FP_topnt+1              ; zero page of to-pointer
    sta FP_fmpnt+1              ; zero page of from-pointer
    sta FP_sindx				; zero string index
    
	cld							; clear decimal mode flag
	
	ldx #FP_inmtas				; set pointer to storage area
	stx FP_topnt				; store in to-pointer
	ldx #$0c					; set precision counter
	jsr clrmem					; clear the storage area
	
	jsr input					; fetch the next character in the string
	
	cmp #'+'					; test to see if it's a + sign
	beq ninput					; if so, just get the next input
	cmp #'-'					; test to see if it's a - sign
	bne notplm					; no, test if valid char
	sta FP_inmtas				; make input-sign non-zero
	
ninput:
	jsr input					; get next character from string

notplm:
	cmp #'.'					; do we have a decimal point
	bne spriod					; no: skip period

period:
	ldx FP_inprdi				; X <- whether we've seen a period before
	cpx #'.' 					; Do we already have a decimal point ?
	beq endinp					; Yes: end input

per1:
	sta FP_inprdi				; store the fact that we have a .
	ldy #0						; zero the index
	sty FP_iocnt					; Reset the digit counter
	jmp ninput					; next character

spriod:
	cmp #'E'					; test for E for exponent notation
	bne sfndxp					; no, skip the found-exponent code

fndexp:
	jsr input					; get the next character
	cmp #'+'					; test for '+' sign
	beq expinp					; yes ? ignore, fetch next char
	cmp #'-'					; test for '-' sign
	bne noexps					; no, test for digit
	sta FP_inexps				; yes: store exponent minus sign

expinp:
	jsr input					; get the next character

noexps:
	cmp #'0'					; number, test lower limit

island:
	bmi endinp					; No, end input string
	cmp #':'					; numner, test upper limit
	bpl endinp					; illegal char, stop accepting input
	and #$0F					; strip off everything except the lower nibble
	sta FP_temp1				; store BCD in temporary storage
	ldx $FP_ioexpd				; set pointer to exponent storage
	lda #$3						; test for upper limit of exponent
	cmp FP_PAGE0,X				; is tens digit > 3
	bmi endinp					; yes, end input
	lda FP_PAGE0,X				; store temporarily in A
	clc							; clear carry so it doesn't interfere
	rol FP_PAGE0,X				; tens digit x2
	rol FP_PAGE0,X				; tens digit x2 (again) => x4
	adc FP_PAGE0,X				; tens digit + original => x5
	rol FP_PAGE0,X				; tens digit x2			=> x10
	adc FP_temp1				; Add new input
	sta FP_PAGE0,X				; store in exponent storage
	jmp expinp					; next character
	
sfndxp:
	cmp #'0'					; number, test lower limit
	bmi endinp					; No, end input string
	cmp #':'					; numner, test upper limit
	bpl endinp					; illegal char, stop accepting input
	tay							; store temporarily
	lda #$F8					; Input too large ?
	bit FP_iostr2				; AND with mem, +ve if too large
	bne ninput					; yes, ignore and fetch next char
	tya							; retrieve the value
	inc FP_iocnt					; increment digit counter
	and #$0F					; mask off any non-numeric parts
	pha							; save temporarily
	jsr decbin					; multiply previous value x 10
	ldx #FP_iostr				; set pointer to storage
	pla							; fetch digit just obtained
	clc							; clear for addition
	adc FP_PAGE0,X				; add digit to storage
	sta FP_PAGE0,X				; save new sum
	lda #0						; clear A for next addition
	adc FP_PAGE0+1,X			; add carry to next byte
	sta FP_PAGE0+1,X			; store in next byte
	lda #0						; clear A for next addition
	adc FP_PAGE0+2,X			; add carry to next byte
	sta FP_PAGE0+2,X			; store in next byte
	jmp ninput					; next character...

endinp:
	lda FP_inmtas				; Test is +ve or -ve
	beq finput					; indicator zero, number +ve
	ldx #FP_iostr				; index to LSB of input mantissa
	ldy #$03					; set precision
	jsr complm					; 2's complement for negative

finput:
	lda #$0
	sta FP_iostr-1				; clear input storage LSB-1
	lda #FP_lswe				; set pointer to FPacc
	sta FP_topnt				; store in to-pointer
	lda #FP_iostr-1				; set pointer to input storage
	sta FP_fmpnt				; store in from-pointer
	ldx #4						; set precision
	jsr movind					; move input to FPacc
	
	ldy #$17					; set exponent for normalisation
	sty FP_acce					; store in FPAcc exponent
	jsr FP_norm					; normalise the input
	lda FP_inexps				; Test exponent sign indicator
	beq posexp					; +ve ? Same exponent
	lda #$ff					; -ve ? Form 2's complement ..
	eor FP_ioexpd				; .. of exponent value ..
	sta FP_ioexpd				; .. by complementing and ..
	inc FP_ioexpd				; .. and incrementing
	
posexp:
	lda FP_inprdi				; Test period indicator
	beq expok					; if zero, no decimal point
	lda #0						; clear A
	sec							; prepare for subtraction
	sbc FP_iocnt					; form negative of digit-count

expok:
	clc							; clear carry for addition
	adc FP_ioexpd				; add to compensate for decimal point
	sta FP_ioexpd				; store results
	bmi minexp					; -ve exponent, adjust to 0
	bne expfix					; not zero, adjust to 0
	rts							; return with value in FPacc

expfix:
	jsr fpix10					; multiply by 10 ..
	bne expfix					; .. until we're good
	rts

fpix10:
	lda #$4						; multiply FPAcc by 10
	sta FP_oexp					; load FPop with a value of 10 ..
	lda #$50					; .. by setting the exponent to 4 ..
	sta FP_omsw					; .. and the mantissa to $50 ..
	lda #0						; ..
	sta FP_onsw					; .. $00 ..
	sta FP_olsw					; .. $00
	jsr FP_mul					; and multiply
	dec FP_ioexpd				; decrement decimal exponent
	rts							; return to test for completion

minexp:
	jsr fpd10					; compensate decimal exponent minus ..
	bne minexp					; .. until done
	rts

fpd10:
	lda #$fd					; multiply FPAcc by 0.1
	sta FP_oexp					; load FPop with a value of 0.1 ..
	lda #$66					; .. by setting the exponent to -3 ..
	sta FP_omsw					; .. and the mantissa to $66 ..
	sta FP_onsw					; .. $66 ..
	lda #$67					; ..
	sta FP_olsw					; .. $67
	jsr FP_mul					; and multiply
	inc FP_ioexpd				; increment decimal exponent
	rts							; return
	
decbin:
	lda #0
	sta FP_iostr3				; clear MSB+1 of result
	ldx #FP_iolsw				; set pointer to io work area
	stx FP_topnt				; store in to-pointer
	ldx #FP_iostr				; set pointer to io storage
	stx FP_fmpnt				; store in from-pointer
	ldx #$4						; set the precision
	jsr movind					; copy io storage to work area
	
	ldx #FP_iostr				; set pointer to original value
	ldy #$4						; set precision counter
	jsr rotatl					; start x10 routine (total = x2)

	ldx #FP_iostr				; set pointer to original value
	ldy #$4						; set precision counter
	jsr rotatl					; do x10 routine (total = x4)

	ldx #FP_iolsw				; set pointer to io work area
	stx FP_fmpnt				; set from-pointer
	ldx #FP_iostr				; set pointer to io storage
	stx FP_topnt				; store in to-pointer
	ldx #4						; set precision counter
	jsr adder					; add original to rotated (total = x5)
	
	ldx #FP_iostr				; reset pointer
	ldy #$4						; set precision counter
	jmp rotatl					; x2 again (total = x10) and return
	
	
	
	
