//
//  Token.h
//  xtal-a
//
//  Created by Thrud The Barbarian on 10/28/22.
//

#ifndef Token_h
#define Token_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"


typedef enum
	{
	P_NONE,
	P_INCLUDE,
	P_IF,
	P_ELSE,
	P_ENDIF,
	P_BYTE,
	P_WORD,
	P_ADDR,
	P_ORG,
	P_SRCREF,
	P_REG,
	
	P_MOVE,
	P_ADD,
	P_DIV,
	P_MUL,
	P_SUB,
	P_CALL,
	P_EXEC,
	
	P_ADC,
	P_AND,
	P_ASL,
	P_BCC,
	P_BCS,
	P_BEQ,
	P_BMI,
	P_BNE,
	P_BPL,
	P_BVC,
	P_BVS,
	// Note that P_BRK & P_BIT are out-of-order on purpose, to make the
	// 'test for branch' a lot easier
	P_BRK,
	P_BIT,
	P_CLC,
	P_CLD,
	P_CLI,
	P_CLV,
	P_CMP,
	P_CPX,
	P_CPY,
	P_DEC,
	P_DEX,
	P_DEY,
	P_EOR,
	P_INC,
	P_INX,
	P_INY,
	P_JMP,
	P_JSR,
	P_LDA,
	P_LDX,
	P_LDY,
	P_LSR,
	P_NOP,
	P_ORA,
	P_PHA,
	P_PHP,
	P_PLA,
	P_PLP,
	P_ROL,
	P_ROR,
	P_RTI,
	P_RTS,
	P_SBC,
	P_SEC,
	P_SED,
	P_SEI,
	P_STA,
	P_STX,
	P_STY,
	P_TAX,
	P_TAY,
	P_TSX,
	P_TXA,
	P_TXS,
	P_TYA,
	P_LABEL,
	P_LLABEL
	} PseudoOp;

typedef enum
	{
	T_NONE,
	T_DIRECTIVE,
	T_MACRO,
	T_META,
	T_LABEL,
	T_LLABEL,
	T_6502
	} OpType;

typedef enum
	{
	A_NONE				= 0,
	A_ACCUMULATOR		= (1<<0),
	A_ABSOLUTE			= (1<<1),
	A_ABSOLUTE_XINDEX	= (1<<2),
	A_ABSOLUTE_YINDEX	= (1<<3),
	A_IMMEDIATE			= (1<<4),
	A_IMPLIED			= (1<<5),
	A_INDIRECT			= (1<<6),
	A_XINDEX_INDIRECT	= (1<<7),
	A_INDIRECT_YINDEX	= (1<<8),
	A_RELATIVE			= (1<<9),
	A_ZEROPAGE			= (1<<10),
	A_ZEROPAGE_XINDEX	= (1<<11),
	A_ZEROPAGE_YINDEX	= (1<<12),
	} AddressingMode;

class Token
	{
	/*************************************************************************\
    |* Types of token. Make sure this syncs up with the code in toString()
    \*************************************************************************/
	public:

		typedef struct
			{
			OpType type;			// Type of token
			PseudoOp which;			// Which exact token
			String name;			// Human-readable name
			String prefix;			// What to look for in the parser
			int length;				// Prefix length
			} TokenInfo;

		typedef enum
			{
			S_UNUSED	= 0,
			S_SIGNED	= 1,
			S_UNSIGNED	= 2
			} Signed;
		
		static const int64_t NO_ADDRESS = 0xffffffff;
		
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GETSET(OpType, type, Type);					// Token type
    GETSET(PseudoOp, which, Which);				// Token PseudoOp
    GETSET(String, arg1, Arg1);					// First argument
    GETSET(String, arg2, Arg2);					// Second argument
    GETSET(int, size, Size);					// Bytes to operate over
    GETSET(Signed, signType, SignType);			// Using signed arithmetic ?
    GETSET(int, bytes, Bytes);					// Instruction stream bytes
    GETSET(int32_t, addr, Addr);				// Any address argument
    GETSET(uint8_t, opcode, Opcode);			// Which opcode to write
    GETSET(uint8_t, op1, Op1);					// Operand 1
    GETSET(uint8_t, op2, Op2);					// Operand 2
    GETSET(std::vector<uint8_t>, data, Data);	// Bytes reserved by user
    GETSET(AddressingMode, addrMode, AddrMode);	// Addressing mode
    
    private:
        
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Token(OpType token=T_NONE, PseudoOp which=P_NONE);

        /*********************************************************************\
        |* Return a string representation of the token
        \*********************************************************************/
        String toString(int64_t offset = NO_ADDRESS);

        /*********************************************************************\
        |* Get token info for a given operation
        \*********************************************************************/
        static TokenInfo parsePrefix(String txt);

        /*********************************************************************\
        |* Return the opcode for a given instruction and addressing mode, or -1
        \*********************************************************************/
        static int opcode(PseudoOp op, AddressingMode mode);

        /*********************************************************************\
        |* Clear out the token from any past use
        \*********************************************************************/
        void clear(void);
	};

#endif /* Token_h */
