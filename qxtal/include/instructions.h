//
//  instructions.h
//  xtal
//
//  Created by Simon Gornall on 11/28/22.
//

#ifndef instructions_h
#define instructions_h


/*****************************************************************************\
|* Addressing modes of each instruction
\*****************************************************************************/
typedef enum
	{
	anon	= -1,		// None, illegal op
	aACC	= 0,		// Accumulator
	aABS,				// Absolute
	aABX,				// Absolute, X
	aABY,				// Absolute, Y
	aIMM,				// Immediate
	aIMP,				// Implied
	aIND,				// Indirect
	aXIN,				// X,Indirect
	aINY,				// Indirect,Y
	aREL,				// Relative
	aZPG,				// Zero-page
	aZPX,				// Zero-page,X
	aZPY				// Zero-page,Y
	} AddressingMode;

/*****************************************************************************\
|* Instruction types
\*****************************************************************************/
typedef enum
	{
	inon = -1;
	iADC = 0,
	iAND,
	iASL,
	iBCC,
	iBCS,
	iBEQ,
	iBIT,
	iBMI,
	iBNE,
	iBPL,
	iBRK,
	iBVC,
	iBVS,
	iCLC,
	iCLD,
	iCLI,
	iCLV,
	iCMP,
	iCPX,
	iCPY,
	iDEC,
	iDEX,
	iDEY,
	iEOR,
	iINC,
	iINX,
	iINY,
	iJMP,
	iJSR,
	iLDA,
	iLDX,
	iLDY,
	iLSR,
	iNOP,
	iORA,
	iPHA,
	iPHP,
	iPLA,
	iPLP,
	iROL,
	iROR,
	iRTI,
	iRTS,
	iSBC,
	iSEC,
	iSED,
	iSEI,
	iSTA,
	iSTX,
	iSTY,
	iTAX,
	iTAY,
	iTSX,
	iTXA,
	iTXS,
	iTYA
	} InsnType;

#endif // ! instructions_h

