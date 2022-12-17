//
//  Token.cc
//  xtal-a
//
//  Created by Thrud The Barbarian on 10/28/22.
//

#include "ContextMgr.h"
#include "Token.h"
#include "Stringutils.h"

#define CTXMGR					ContextMgr::sharedInstance()

static Token::TokenInfo _tokens[] = {
	{T_DIRECTIVE, 	P_INCLUDE,	"TOKEN_INCLUDE", ".include", 8},
	{T_DIRECTIVE, 	P_IF,		"TOKEN_IF     ", ".if",      3},
	{T_DIRECTIVE, 	P_ELSE,		"TOKEN_ELSE   ", ".else",    5},
	{T_DIRECTIVE, 	P_ENDIF,	"TOKEN_ENDIF  ", ".endif",   6},
	{T_DIRECTIVE, 	P_BYTE,		"TOKEN_BYTE   ", ".byte"   , 5},
	{T_DIRECTIVE, 	P_WORD,		"TOKEN_WORD   ", ".word"   , 5},
	{T_DIRECTIVE, 	P_ADDR,		"TOKEN_ADDR   ", ".addr"   , 5},
	{T_DIRECTIVE, 	P_ORG,		"TOKEN_ORG    ", ".org"	   , 4},
	{T_DIRECTIVE, 	P_SRCREF,	"TOKEN_SRCREF ", ""		   , 0},
	{T_DIRECTIVE, 	P_REG,	    "TOKEN_REG    ", ".reg"	   , 4},
	{T_DIRECTIVE, 	P_REG,	    "TOKEN_REG    ", ".reg"	   , 4},
	
	{T_META, 		P_MOVE,		"TOKEN_MOVE   ", "move"	   , 4},
	{T_META, 		P_ADD,		"TOKEN_ADD    ", "add"	   , 3},
	{T_META, 		P_DIV,		"TOKEN_DIV    ", "div"	   , 3},
	{T_META, 		P_MUL,		"TOKEN_MUL    ", "mul"	   , 3},
	{T_META, 		P_SUB,		"TOKEN_SUB    ", "sub"	   , 3},
	{T_META, 		P_CALL,		"TOKEN_CALL   ", "call"	   , 4},
	{T_META, 		P_EXEC,		"TOKEN_EXEC   ", "exec"	   , 4},
	
	{T_6502, 		P_ADC,		"TOKEN_ADC    ", "adc"	   , 3},
	{T_6502, 		P_AND,		"TOKEN_AND    ", "and"	   , 3},
	{T_6502, 		P_ASL,		"TOKEN_ASL    ", "asl"	   , 3},
	{T_6502, 		P_BCC,		"TOKEN_BCC    ", "bcc"	   , 3},
	{T_6502, 		P_BCS,		"TOKEN_BCS    ", "bcs"	   , 3},
	{T_6502, 		P_BEQ,		"TOKEN_BEQ    ", "beq"	   , 3},
	{T_6502, 		P_BMI,		"TOKEN_BMI    ", "bmi"	   , 3},
	{T_6502, 		P_BNE,		"TOKEN_BNE    ", "bne"	   , 3},
	{T_6502, 		P_BPL,		"TOKEN_BPL    ", "bpl"	   , 3},
	{T_6502, 		P_BVC,		"TOKEN_BVC    ", "bvc"	   , 3},
	{T_6502, 		P_BVS,		"TOKEN_BVS    ", "bvs"	   , 3},
	// Note that P_BRK & P_BIT are out-of-order on purpose, to make the
	// 'test for branch' a lot easier
	{T_6502, 		P_BRK,		"TOKEN_BRK    ", "brk"	   , 3},
	{T_6502, 		P_BIT,		"TOKEN_BIT    ", "bit"	   , 3},
	{T_6502, 		P_CLC,		"TOKEN_CLC    ", "clc"	   , 3},
	{T_6502, 		P_CLD,		"TOKEN_CLD    ", "cld"	   , 3},
	{T_6502, 		P_CLI,		"TOKEN_CLI    ", "cli"	   , 3},
	{T_6502, 		P_CLV,		"TOKEN_CLV    ", "clv"	   , 3},
	{T_6502, 		P_CMP,		"TOKEN_CMP    ", "cmp"	   , 3},
	{T_6502, 		P_CPX,		"TOKEN_CPX    ", "cpx"	   , 3},
	{T_6502, 		P_CPY,		"TOKEN_CPY    ", "cpy"	   , 3},
	{T_6502, 		P_DEC,		"TOKEN_DEC    ", "dec"	   , 3},
	{T_6502, 		P_DEX,		"TOKEN_DEX    ", "dex"	   , 3},
	{T_6502, 		P_DEY,		"TOKEN_DEY    ", "dey"	   , 3},
	{T_6502, 		P_EOR,		"TOKEN_EOR    ", "eor"	   , 3},
	{T_6502, 		P_INC,		"TOKEN_INC    ", "inc"	   , 3},
	{T_6502, 		P_INX,		"TOKEN_INX    ", "inx"	   , 3},
	{T_6502, 		P_INY,		"TOKEN_INY    ", "iny"	   , 3},
	{T_6502, 		P_JMP,		"TOKEN_JMP    ", "jmp"	   , 3},
	{T_6502, 		P_JSR,		"TOKEN_JSR    ", "jsr"	   , 3},
	{T_6502, 		P_LDA,		"TOKEN_LDA    ", "lda"	   , 3},
	{T_6502, 		P_LDX,		"TOKEN_LDX    ", "ldx"	   , 3},
	{T_6502, 		P_LDY,		"TOKEN_LDY    ", "ldy"	   , 3},
	{T_6502, 		P_LSR,		"TOKEN_LSR    ", "lsr"	   , 3},
	{T_6502, 		P_NOP,		"TOKEN_NOP    ", "nop"	   , 3},
	{T_6502, 		P_ORA,		"TOKEN_ORA    ", "ora"	   , 3},
	{T_6502, 		P_PHA,		"TOKEN_PHA    ", "pha"	   , 3},
	{T_6502, 		P_PHP,		"TOKEN_PHP    ", "php"	   , 3},
	{T_6502, 		P_PLA,		"TOKEN_PLA    ", "pla"	   , 3},
	{T_6502, 		P_PLP,		"TOKEN_PLP    ", "plp"	   , 3},
	{T_6502, 		P_ROL,		"TOKEN_ROL    ", "rol"	   , 3},
	{T_6502, 		P_ROR,		"TOKEN_ROR    ", "ror"	   , 3},
	{T_6502, 		P_RTI,		"TOKEN_RTI    ", "rti"	   , 3},
	{T_6502, 		P_RTS,		"TOKEN_RTS    ", "rts"	   , 3},
	{T_6502, 		P_SBC,		"TOKEN_SBC    ", "sbc"	   , 3},
	{T_6502, 		P_SEC,		"TOKEN_SEC    ", "sec"	   , 3},
	{T_6502, 		P_SED,		"TOKEN_SED    ", "sed"	   , 3},
	{T_6502, 		P_SEI,		"TOKEN_SEI    ", "sei"	   , 3},
	{T_6502, 		P_STA,		"TOKEN_STA    ", "sta"	   , 3},
	{T_6502, 		P_STX,		"TOKEN_STX    ", "stx"	   , 3},
	{T_6502, 		P_STY,		"TOKEN_STY    ", "sty"	   , 3},
	{T_6502, 		P_TAX,		"TOKEN_TAX    ", "tax"	   , 3},
	{T_6502, 		P_TAY,		"TOKEN_TAY    ", "tay"	   , 3},
	{T_6502, 		P_TSX,		"TOKEN_TSX    ", "tsx"	   , 3},
	{T_6502, 		P_TXA,		"TOKEN_TXA    ", "txa"	   , 3},
	{T_6502, 		P_TXS,		"TOKEN_TXS    ", "txs"	   , 3},
	{T_6502, 		P_TYA,		"TOKEN_TYA    ", "tya"	   , 3},
	
	{T_LABEL, 		P_LABEL,	"TOKEN_LABEL  ", "xxxx:"   , 5},
	{T_LLABEL,		P_LLABEL,	"TOKEN_LLABEL ", "xxxx;"   , 5},

	// Must be last
	{T_NONE, 		P_NONE, 	"[None]       ", "yy"		,2}};

static std::map<PseudoOp, Token::TokenInfo> _info;


static std::map<PseudoOp, std::map<AddressingMode, int> > _codes =
	{
	{P_ADC, {{A_IMMEDIATE,			0x69},
			 {A_ZEROPAGE, 			0x65},
			 {A_ZEROPAGE_XINDEX, 	0x75},
			 {A_ABSOLUTE, 			0x6D},
			 {A_ABSOLUTE_XINDEX, 	0x7D},
			 {A_ABSOLUTE_YINDEX, 	0x79},
			 {A_XINDEX_INDIRECT, 	0x61},
			 {A_INDIRECT_YINDEX, 	0x71}}},
	{P_AND, {{A_IMMEDIATE,			0x29},
			 {A_ZEROPAGE, 			0x25},
			 {A_ZEROPAGE_XINDEX, 	0x35},
			 {A_ABSOLUTE, 			0x2D},
			 {A_ABSOLUTE_XINDEX, 	0x3D},
			 {A_ABSOLUTE_YINDEX, 	0x39},
			 {A_XINDEX_INDIRECT, 	0x21},
			 {A_INDIRECT_YINDEX, 	0x31}}},
	{P_ASL, {{A_ACCUMULATOR,		0x0A},
			 {A_ZEROPAGE, 			0x06},
			 {A_ZEROPAGE_XINDEX, 	0x16},
			 {A_ABSOLUTE, 			0x0E},
			 {A_ABSOLUTE_XINDEX, 	0x1E}}},
	{P_BCC, {{A_RELATIVE,			0x90}}},
	{P_BCS, {{A_RELATIVE,			0xB0}}},
	{P_BEQ, {{A_RELATIVE,			0xF0}}},
	{P_BIT, {{A_ZEROPAGE, 			0x24},
			 {A_ABSOLUTE, 			0x2C}}},
	{P_BMI, {{A_RELATIVE,			0x30}}},
	{P_BNE, {{A_RELATIVE,			0xD0}}},
	{P_BPL, {{A_RELATIVE,			0x10}}},
	{P_BRK, {{A_IMPLIED,			0x00}}},
	{P_BVC, {{A_RELATIVE,			0x50}}},
	{P_BVS, {{A_RELATIVE,			0x70}}},
	{P_CLC, {{A_IMPLIED,			0x18}}},
	{P_CLD, {{A_IMPLIED,			0xD8}}},
	{P_CLI, {{A_IMPLIED,			0x58}}},
	{P_CLV, {{A_IMPLIED,			0xB8}}},
	{P_CMP, {{A_IMMEDIATE,			0xC9},
			 {A_ZEROPAGE, 			0xC5},
			 {A_ZEROPAGE_XINDEX, 	0xD5},
			 {A_ABSOLUTE, 			0xCD},
			 {A_ABSOLUTE_XINDEX, 	0xDD},
			 {A_ABSOLUTE_YINDEX, 	0xD9},
			 {A_XINDEX_INDIRECT, 	0xC1},
			 {A_INDIRECT_YINDEX, 	0xD1}}},
	{P_CPX, {{A_IMMEDIATE,			0xE0},
			 {A_ZEROPAGE, 			0xE4},
			 {A_ABSOLUTE, 			0xEC}}},
	{P_CPY, {{A_IMMEDIATE,			0xC0},
			 {A_ZEROPAGE, 			0xC4},
			 {A_ABSOLUTE, 			0xCC}}},
	{P_DEC, {{A_ZEROPAGE, 			0xC6},
			 {A_ZEROPAGE_XINDEX, 	0xD6},
			 {A_ABSOLUTE, 			0xCE},
			 {A_ABSOLUTE_XINDEX, 	0xDE}}},
	{P_DEX, {{A_IMPLIED,			0xCA}}},
	{P_DEY, {{A_IMPLIED,			0x88}}},
	{P_EOR, {{A_IMMEDIATE,			0x49},
			 {A_ZEROPAGE, 			0x45},
			 {A_ZEROPAGE_XINDEX, 	0x55},
			 {A_ABSOLUTE, 			0x4D},
			 {A_ABSOLUTE_XINDEX, 	0x5D},
			 {A_ABSOLUTE_YINDEX, 	0x59},
			 {A_XINDEX_INDIRECT, 	0x41},
			 {A_INDIRECT_YINDEX, 	0x51}}},
	{P_INC, {{A_ZEROPAGE, 			0xE6},
			 {A_ZEROPAGE_XINDEX, 	0xF6},
			 {A_ABSOLUTE, 			0xEE},
			 {A_ABSOLUTE_XINDEX, 	0xFE}}},
	{P_INX, {{A_IMPLIED,			0xE8}}},
	{P_INY, {{A_IMPLIED,			0xC8}}},
	{P_JMP, {{A_ABSOLUTE, 			0x4C},
			 {A_INDIRECT, 			0x6C}}},
	{P_JSR, {{A_ABSOLUTE, 			0x20}}},
	{P_LDA, {{A_IMMEDIATE,			0xA9},
			 {A_ZEROPAGE, 			0xA5},
			 {A_ZEROPAGE_XINDEX, 	0xB5},
			 {A_ABSOLUTE, 			0xAD},
			 {A_ABSOLUTE_XINDEX, 	0xBD},
			 {A_ABSOLUTE_YINDEX, 	0xB9},
			 {A_XINDEX_INDIRECT, 	0xA1},
			 {A_INDIRECT_YINDEX, 	0xB1}}},
	{P_LDX, {{A_IMMEDIATE,			0xA2},
			 {A_ZEROPAGE, 			0xA6},
			 {A_ZEROPAGE_YINDEX, 	0xB6},
			 {A_ABSOLUTE, 			0xAE},
			 {A_ABSOLUTE_YINDEX, 	0xBE}}},
	{P_LDY, {{A_IMMEDIATE,			0xA0},
			 {A_ZEROPAGE, 			0xA4},
			 {A_ZEROPAGE_XINDEX, 	0xB4},
			 {A_ABSOLUTE, 			0xAC},
			 {A_ABSOLUTE_XINDEX, 	0xBC}}},
	{P_LSR, {{A_ACCUMULATOR,		0x4A},
			 {A_ZEROPAGE, 			0x46},
			 {A_ZEROPAGE_XINDEX, 	0x56},
			 {A_ABSOLUTE, 			0x4E},
			 {A_ABSOLUTE_XINDEX, 	0x5E}}},
	{P_NOP, {{A_IMPLIED,			0xEA}}},
	{P_ORA, {{A_IMMEDIATE,			0x09},
			 {A_ZEROPAGE, 			0X05},
			 {A_ZEROPAGE_XINDEX, 	0x15},
			 {A_ABSOLUTE, 			0x0D},
			 {A_ABSOLUTE_XINDEX, 	0x1D},
			 {A_ABSOLUTE_YINDEX, 	0x19},
			 {A_XINDEX_INDIRECT, 	0x01},
			 {A_INDIRECT_YINDEX, 	0x11}}},
	{P_PHA, {{A_IMPLIED,			0x48}}},
	{P_PHP, {{A_IMPLIED,			0x08}}},
	{P_PLA, {{A_IMPLIED,			0x68}}},
	{P_PLP, {{A_IMPLIED,			0x28}}},
	{P_ROL, {{A_ACCUMULATOR,		0x2A},
			 {A_ZEROPAGE, 			0x26},
			 {A_ZEROPAGE_XINDEX, 	0x36},
			 {A_ABSOLUTE, 			0x2E},
			 {A_ABSOLUTE_XINDEX, 	0x3E}}},
	{P_ROR, {{A_ACCUMULATOR,		0x6A},
			 {A_ZEROPAGE, 			0x66},
			 {A_ZEROPAGE_XINDEX, 	0x76},
			 {A_ABSOLUTE, 			0x6E},
			 {A_ABSOLUTE_XINDEX, 	0x7E}}},
	{P_RTI, {{A_IMPLIED,			0x40}}},
	{P_RTS, {{A_IMPLIED,			0x60}}},
	{P_SBC, {{A_IMMEDIATE,			0xE9},
			 {A_ZEROPAGE, 			0XE5},
			 {A_ZEROPAGE_XINDEX, 	0xF5},
			 {A_ABSOLUTE, 			0xED},
			 {A_ABSOLUTE_XINDEX, 	0xFD},
			 {A_ABSOLUTE_YINDEX, 	0xF9},
			 {A_XINDEX_INDIRECT, 	0xE1},
			 {A_INDIRECT_YINDEX, 	0xF1}}},
	{P_SEC, {{A_IMPLIED,			0x38}}},
	{P_SED, {{A_IMPLIED,			0xF8}}},
	{P_SEI, {{A_IMPLIED,			0x78}}},
	{P_STA, {{A_ZEROPAGE, 			0x85},
			 {A_ZEROPAGE_XINDEX, 	0x95},
			 {A_ABSOLUTE, 			0x8D},
			 {A_ABSOLUTE_XINDEX, 	0x9D},
			 {A_ABSOLUTE_YINDEX, 	0x99},
			 {A_XINDEX_INDIRECT, 	0x81},
			 {A_INDIRECT_YINDEX, 	0x91}}},
	{P_STX, {{A_ZEROPAGE, 			0x86},
			 {A_ZEROPAGE_YINDEX, 	0x96},
			 {A_ABSOLUTE, 			0x8E}}},
	{P_STY, {{A_ZEROPAGE, 			0x84},
			 {A_ZEROPAGE_XINDEX, 	0x94},
			 {A_ABSOLUTE, 			0x8C}}},
	{P_TAX, {{A_IMPLIED,			0xAA}}},
	{P_TAY, {{A_IMPLIED,			0xA8}}},
	{P_TSX, {{A_IMPLIED,			0xBA}}},
	{P_TXA, {{A_IMPLIED,			0x8A}}},
	{P_TXS, {{A_IMPLIED,			0x9A}}},
	{P_TYA, {{A_IMPLIED,			0x98}}},
	};

#define HEX2 	toHexString(((int)_op2) * 256 + _op1, "$")
#define HEX1	toHexString(_op1, "$")

	
/*****************************************************************************\
|* Helper function to generate the map if it's not already populated
\*****************************************************************************/
void _populateMap(void)
	{
	if (_info.size() == 0)
		{
		int i = 0;
		forever
			{
			_info[_tokens[i].which] = _tokens[i];
			if (_tokens[i].type == T_NONE)
				break;
			i++;
			}
		}
	}

/****************************************************************************\
|* Constructor
\****************************************************************************/
Token::Token(OpType type, PseudoOp which)
		:_type(type)
		,_which(which)
		,_arg1("")
		,_arg2("")
		,_signType(S_UNUSED)
		,_bytes(0)
		,_addr(-1)
	{
	_size = INT_MIN;
	}
	
/*****************************************************************************\
|* Clear the token state
\*****************************************************************************/
void Token::clear(void)
	{
	_type 		= T_NONE;
	_which		= P_NONE;
	_size		= INT_MIN;
	_signType	= S_UNUSED;
	_arg1		= "";
	_arg2		= "";
	_bytes		= 0;
	_addr		= -1;
	}
	
/*****************************************************************************\
|* Return a string representation of the token
\*****************************************************************************/
String Token::toString(int64_t at)
	{
	String info = "";
	/*************************************************************************\
	|* Populate the map if it isn't already holding data
	\*************************************************************************/
	_populateMap();
	
	/*************************************************************************\
	|* Return the human-readable string
	\*************************************************************************/
	auto it = _info.find(_which);
	if (it == _info.end())
		{
		LOG("Cannot find token %d in the token info map", _which);
		it = _info.find(P_NONE);
		}
	
	if (it->second.type == T_6502)
		info += "\t";
	
	if (_which == P_BYTE)
		{
		info += ".byte ";
		String comma = "";
		for (int i=0; i<_data.size(); i++)
			{
			info += comma + toHexString(_data[i], "$") + " ";
			comma = ", ";
			if (((i+1) %10 == 0) && i < _data.size()-1)
				{
				comma = "";
				info += "\n.byte ";
				}
			}
		info += "\n";
		}
	else if ((_which == P_LABEL) || (_which == P_LLABEL))
		info += "\n" + _arg1+":";
	else if (_which == P_SRCREF)
		info += "\n; " + _arg1+"::";
	else
		info += it->second.prefix;
	
	switch (_addrMode)
		{
		case A_NONE:
			if (_addr > 0)
				info += " " + toHexString(_addr, "$") +"\n";
			break;
		
		case A_ACCUMULATOR:
			info += " A";
			break;
		
		case A_ABSOLUTE:
			info += " " + HEX2;
			break;
		
		case A_ABSOLUTE_XINDEX:
			info += " " + HEX2 + ",X";
			break;
		
		case A_ABSOLUTE_YINDEX:
			info += " " + HEX2 + ",Y";
			break;
		
		case A_IMMEDIATE:
			info += " #" + HEX1;
			break;
		
		case A_IMPLIED:
			break;
		
		case A_INDIRECT:
			info += " (" + HEX2 +")";
			break;
		
		case A_XINDEX_INDIRECT:
			info += " (" + HEX1 +",X)";
			break;
		
		case A_INDIRECT_YINDEX:
			info += " (" + HEX1 +"),Y";
			break;
		
		case A_RELATIVE:
			{
			int delta= _op1;
			
			String offset = std::to_string(delta);
			if (_op1 > 128)
				{
				delta = 256 - _op1;
				offset = "-" + std::to_string(delta);
				}
				
			info += " " + CTXMGR->identifier()+"_"+_arg1 + " ; {" + offset;
			if (at != NO_ADDRESS)
				info += " : " + toHexString((int)(at + 2 + delta), "$");
			info += "}";
			
			break;
			}
		
		case A_ZEROPAGE:
			info +=  " " + HEX1;
			break;
		
		case A_ZEROPAGE_XINDEX:
			info +=  " " + HEX1 + ",X";
			break;
		
		case A_ZEROPAGE_YINDEX:
			info +=  " " + HEX1 + ",Y";
			break;
		
		default:
			FATAL(ERR_EMIT, "Unknown addressing mode!\n%s",
				CTXMGR->location().c_str());
		}
	return info;
	}

	
/*****************************************************************************\
|* Return token info for a given string
\*****************************************************************************/
Token::TokenInfo Token::parsePrefix(String s)
	{
	/*************************************************************************\
	|* Populate the map if it isn't already holding data
	\*************************************************************************/
	_populateMap();
	
	for (Elements<PseudoOp, Token::TokenInfo> kv : _info)
		if (kv.value.length > 0)
			if (s.substr(0, kv.value.length) == kv.value.prefix)
				return kv.value;
		
	return _info[P_NONE];
	}
	
/*****************************************************************************\
|* Return the opcode for a given instruction and addressing mode, or -1
\*****************************************************************************/
int Token::opcode(PseudoOp op, AddressingMode mode)
	{
	int result = -1;
	
	auto i = _codes.find(op);
	if (i != _codes.end())
		{
		auto j = (i->second).find(mode);
		if (j != (i->second).end())
			result = j->second;
		}
		
	return result;
	}
