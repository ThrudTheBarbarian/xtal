;
;
; Beginnings of a simple stdio library for xtal
; ----


CMD_PUT_REC 	= $09   	; Send until EOL
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
ICBLL   		= ICHID+8   ;   buffer length (low byte) in put/get operations
ICBLH   		= ICHID+9   ;   buffer length (high byte)
ICAX1   		= ICHID+10  ;   auxiliary information.  Used by Open .org $600

.function printLine
.clobber x,a
@printLine:
	ldx #0					; just use the default output device
	lda f0					; Get the low byte of the address to print..
	sta ICBAL,x				; and tell CIO
	lda f0+1				; Get the high byte of the address..
	sta ICBAH,x				; and tell CIO
	lda #CMD_PUT_REG		; set command to "print to end of line"
	sta ICCOM,x				; and tell CIO
	lda #0					; set low byte of buffer length...
	sta ICBLL,x				; and tell CIO
	lda #$ff				; set high-byte of buffer length
	sta ICBLH,x				; and tell CIO
	jsr CIOV				; Jump to the print-to-EOL routine
	rts

.endfunction

