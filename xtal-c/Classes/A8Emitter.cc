//
//  A8Emitter.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#include <sstream>
#include <iomanip>

#include "sharedDefines.h"

#include "ASTNode.h"
#include "A8Emitter.h"
#include "RegisterFile.h"
#include "Stringutils.h"
#include "SymbolTable.h"

#define PARENT_IS(x)	(parentAstOp == ASTNode::x)
#define REG				Register::RegType

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
A8Emitter::A8Emitter()
		  :Emitter()
	{
	}


/*****************************************************************************\
|* Destructor
\*****************************************************************************/
A8Emitter::~A8Emitter()
	{
	}
	
/*****************************************************************************\
|* Emit code
\*****************************************************************************/
Register A8Emitter::emit(ASTNode *node,
						 Register reg,
						 int parentAstOp,
						 String label)
	{
	Register left, right;
	Register none(Register::NO_REGISTER);
	
	// We now have specific AST node handling at the top
	switch (node->op())
		{
		case ASTNode::A_IF:
			return (_cgIfAST(node));
    
		case ASTNode::A_WHILE:
			return (_cgWhileAST(node));
    
		case ASTNode::A_GLUE:
			// Do each child statement, and free the
			// registers after each child
			emit(node->left(), none, node->op(), label);
			RegisterFile::clear();
			
			emit(node->right(), none, node->op(), label);
			RegisterFile::clear();
			return (none);
    
		case ASTNode::A_FUNCTION:
			{
			auto symbol = SYMTAB->table()[node->value().identifier];
			functionPreamble(symbol.name());
			emit(node->left(), none, node->op(), "");
			functionPostamble();
			return (none);
			}
		}
	
	if (node->left())
		left = emit(node->left(), none, node->op(), label);
		
	if (node->right())
		right = emit(node->right(), left, node->op(), label);

	switch (node->op())
		{
		case ASTNode::A_ASSIGN:			return right;
		case ASTNode::A_ADD:			return _cgAdd(left, right);
		case ASTNode::A_SUBTRACT:		return _cgSub(left, right);
		case ASTNode::A_MULTIPLY:		return _cgMul(left, right);
		case ASTNode::A_DIVIDE:			return _cgDiv(left, right);
		case ASTNode::A_EQ:
		case ASTNode::A_NE:
		case ASTNode::A_LT:
		case ASTNode::A_GT:
		case ASTNode::A_LE:
		case ASTNode::A_GE:
			if (PARENT_IS(A_IF) || PARENT_IS(A_WHILE))
				return _cgCompareAndJump(left, right, node->op(), label);
			else
				return _cgCompareAndSet(left, right, node->op());
				
		case ASTNode::A_INTLIT:
			{
			return _cgLoadInt(node->value().intValue);
			}
		case ASTNode::A_IDENT:
			{
			auto symbol = SYMTAB->table()[node->value().identifier];
			return _cgLoadGlobal(symbol);
			}
		case ASTNode::A_LVIDENT:
			{
			auto symbol = SYMTAB->table()[node->value().identifier];
			return _cgStoreGlobal(reg, symbol);
			}
		case ASTNode::A_PRINT:
			{
			printReg(left);
			RegisterFile::clear();
			return none;
			}
		case ASTNode::A_WIDEN:
			//_cgWiden(left, node->left()->type(), node->type());
			return left;
		case ASTNode::A_RETURN:
			_cgReturn(left, SYMTAB->functionId());
			return left;
		case ASTNode::A_FUNCCALL:
			return _cgCall(left, node->value().identifier);
			
		default:
			FATAL(ERR_AST_UNKNOWN_OPERATOR, "Unknown AST operator %d", node->op());
		}
	}

	
/*****************************************************************************\
|* Dump a register to CIO0
\*****************************************************************************/
void A8Emitter::printReg(Register r)
	{
	//String size = r.sizeAsString();
	fprintf(_ofp, "\tcall printReg %s\n", r.name().c_str());
	}
	

/*****************************************************************************\
|* Generate a global symbol
\*****************************************************************************/
void A8Emitter::genSymbol(int idx)
	{
	char buf[1024];
	
	Symbol symbol 	= SYMTAB->table()[idx];
	String name 	= symbol.name();
	
	if (symbol.pType() == PT_S32)
		snprintf(buf, 1024, "@S_%s:.word 0,0\n", name.c_str());
	else
		snprintf(buf, 1024, "@S_%s:.byte 0\n", name.c_str());
	append(buf, POSTAMBLE);
	}
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Generate a load-value-to-register
\*****************************************************************************/
Register A8Emitter::_cgLoadInt(int val)
	{
	REG type 	= ((val >= 0) && (val <= 255))
				? Register::UNSIGNED_1BYTE
				: Register::SIGNED_4BYTE;
	Register r	= _regs->allocate(type);
	
	fprintf(_ofp, "\tmove.%d #$%x %s\n", r.size(), val, r.name().c_str());
	return r;
	}

/*****************************************************************************\
|* Determine the size of a symbol
\*****************************************************************************/
static Register::RegType _symbolType(const Symbol& symbol)
	{
	Symbol s 	= (Symbol)symbol;
	Register::RegType type = Register::UNKNOWN;
	
	switch (s.pType())
		{
		case PT_U8:
			type = Register::UNSIGNED_1BYTE;
			break;
		case PT_S8:
			type = Register::SIGNED_1BYTE;
			break;
		case PT_S32:
			type = Register::SIGNED_4BYTE;
			break;
		}
	
	return type;
	}

/*****************************************************************************\
|* Generate a load-value-to-register
\*****************************************************************************/
Register A8Emitter::_cgLoadGlobal(const Symbol& symbol)
	{
	Symbol s 	= (Symbol)symbol;
	REG size	= _symbolType(symbol);
	
	if (size == Register::UNKNOWN)
		FATAL(ERR_TYPE, "Unknown type for symbol %s", s.name().c_str());
		
	Register r	= _regs->allocate(size);
	
	fprintf(_ofp, "\tmove.%d S_%s %s\n",
				r.size(),
				s.name().c_str(),
				r.name().c_str());
	return r;
	}

/*****************************************************************************\
|* Store a register into a global variable
\*****************************************************************************/
Register A8Emitter::_cgStoreGlobal(Register& r, const Symbol& symbol)
	{
	Symbol s 	= (Symbol)symbol;
	REG size	= _symbolType(symbol);

	/*************************************************************************\
    |* Do some zeroing checks if the register size and the symbol size do not
    |* match
    \*************************************************************************/
	if ((s.pType() == PT_S32) || (s.pType() == PT_S32))
		{
		if (r.size() == 1)
			fprintf(_ofp, "\tlda #0\n"
						  "\tsta r%d + 3\n"
						  "\tsta r%d + 2\n"
						  "\tsta r%d + 1\n",
						  r.identifier(),
						  r.identifier(),
						  r.identifier());
		else if (r.size() == 2)
			fprintf(_ofp, "\tlda #0\n"
						  "\tsta r%d + 3\n"
						  "\tsta r%d + 2\n",
						  r.identifier(),
						  r.identifier());
		}
	else if ((s.pType() == PT_S16) || (s.pType() == PT_U16))
		{
		if (r.size() == 1)
			fprintf(_ofp, "\tlda #0\n"
						  "\tsta r%d + 1\n",
						  r.identifier());
		}
		
	/*************************************************************************\
    |* Then store the data
    \*************************************************************************/
	switch (size)
		{
		case Register::SIGNED_1BYTE:
		case Register::UNSIGNED_1BYTE:
			fprintf(_ofp, "\tmove.1 %s S_%s\n",
						r.name().c_str(),
						s.name().c_str());
			break;
			
		case Register::SIGNED_2BYTE:
		case Register::UNSIGNED_2BYTE:
			fprintf(_ofp, "\tmove.2 %s S_%s\n",
						r.name().c_str(),
						s.name().c_str());
			break;
			
		case Register::SIGNED_4BYTE:
		case Register::UNSIGNED_4BYTE:
			fprintf(_ofp, "\tmove.4 %s S_%s\n",
						r.name().c_str(),
						s.name().c_str());
			break;
		
		default:
			FATAL(ERR_TYPE, "Unknown type for symbol %s", s.name().c_str());
		}
	
	return r;
	}

        
/*****************************************************************************\
|* Widen a register
\*****************************************************************************/
void A8Emitter::_cgWiden(Register& reg, int oldWidth, int newWidth)
	{
	if (oldWidth != newWidth)
		{
		switch (newWidth)
			{
			case PT_U8:
				reg.setType(Register::UNSIGNED_1BYTE);
				break;
			
			case PT_S32:
				reg.setType(Register::SIGNED_4BYTE);
				break;
			
			default:
				FATAL(ERR_TYPE, "Unknown type for reg %s", reg.name().c_str());
			}
		}
	}


/*****************************************************************************\
|* Add two registers, and release the first
\*****************************************************************************/
Register A8Emitter::_cgAdd(Register r1, Register r2)
	{
	// Make sure they're the same size before we operate
	_cgSameSize(r1, r2);

	Register result	= _regs->allocate(r1.type());

	String op 		= "\taddu.";
	int separator	= Register::UNSIGNED_4BYTE;
	if ((r1.type() > separator) || (r2.type() > separator))
		op = "\tadds.";
	op += result.sizeAsString();

	fprintf(_ofp, "%s %s %s %s\n",
		op.c_str(),
		r1.name().c_str(),
		r2.name().c_str(),
		result.name().c_str());

	_regs->free(r1);
	_regs->free(r2);
	return result;
	}

/*****************************************************************************\
|* Multiply two registers, and release the first
\*****************************************************************************/
Register A8Emitter::_cgMul(Register r1, Register r2)
	{
	// Make sure they're the same size before we operate
	_cgSameSize(r1, r2);

	Register result	= _regs->allocate(r1.type());

	String op 		= "\tmulu.";
	int separator	= Register::UNSIGNED_4BYTE;
	if ((r1.type() > separator) || (r2.type() > separator))
		op = "\tmuls.";
	op += result.sizeAsString();
	
	fprintf(_ofp, "%s %s %s %s\n", op.c_str(),
								   r1.name().c_str(),
								   r2.name().c_str(),
								   result.name().c_str());
	_regs->free(r1);
	_regs->free(r2);
	return result;
	}

/*****************************************************************************\
|* Subtract two registers, and release the first
\*****************************************************************************/
Register A8Emitter::_cgSub(Register r1, Register r2)
	{
	// Make sure they're the same size before we operate
	_cgSameSize(r1, r2);

	Register result	= _regs->allocate(r1.type());

	String op 		= "\tsubu.";
	int separator	= Register::UNSIGNED_4BYTE;
	if ((r1.type() > separator) || (r2.type() > separator))
		op = "\tsubs.";
	op += result.sizeAsString();

	fprintf(_ofp, "%s %s %s %s\n", op.c_str(),
								   r1.name().c_str(),
								   r2.name().c_str(),
								   result.name().c_str());

	_regs->free(r1);
	_regs->free(r2);
	return result;
	}
	
/*****************************************************************************\
|* Multiply two registers, and release the first
\*****************************************************************************/
Register A8Emitter::_cgDiv(Register r1, Register r2)
	{
	// Make sure they're the same size before we operate
	_cgSameSize(r1, r2);

	Register left	= _regs->allocate(r1.type());
	String op 		= "\tdivu.";
	int separator	= Register::UNSIGNED_4BYTE;
	if ((r1.type() > separator) || (r2.type() > separator))
		op = "\tdivs.";
	op += r1.sizeAsString();
	
	fprintf(_ofp, "%s %s %s %s\n", op.c_str(),
								   r1.name().c_str(),
								   r2.name().c_str(),
								   left.name().c_str());
	_regs->free(left);
	_regs->free(r2);
	return r1;
	}


/*****************************************************************************\
|* Compare two registers, exiting as soon as possible. We want to return the
|* value '1' if the specific type of comparison evaluates true, and zero
|* otherwise.
|*
|* This is done by doing a n-bit cmp, then depending on which operation we are
|* doing, issueing possibly multiple Bxx commands. The following table is
|* used:
|*
|*      Test			Unsigned ints				Signed ints
|*   -----------+-------------------------------+---------------------------
|*		 ==		|		BEQ there				|	BEQ there
|* 		 !=		|		BNE there				|	BNE there
|*		 <		|		BCC there				|	BMI there
|* 		 >		|		BEQ here  ; BCS there	|	BEQ here  ; BPL there
|* 		 <=		|		BCC there ; BEQ there 	|	BMI there ; BEQ there
|*		 >=		|		BCS there				|	BPL there
|*
|* Where 'here' and 'there' are defined thusly:
|*
|*		CMP $20			; Accumulator < location $20 ?
|*		BCC there		; No, continue execution
|* here:
|*		...
|*		CLC
|* 		BCC done
|* there:
|*		...
|* done:
|*		...
|*
\*****************************************************************************/
Register A8Emitter::_cgCompareAndSet(Register r1, Register r2, int how)
	{
	String criteria 	= "beq there";
	String alternate	= "bne done";
	
	switch (how)
		{
		case ASTNode::A_EQ:
			break;
		
		case ASTNode::A_NE:
			criteria 	= "bne there";
			alternate	= "beq done";
			break;
			
		case ASTNode::A_LT:
			criteria 	= "bmi there";
			alternate	= "bpl done";
			break;
			
		case ASTNode::A_GT:
			criteria	= "beq here\n\tbpl there";
			alternate	= "clc\n\tbcc done";
			break;
			
		case ASTNode::A_LE:
			criteria	= "bcc there\n\tbeq there";
			break;
			
		case ASTNode::A_GE:
			criteria	= "bcs there";
			alternate	= "bcc done";
			break;
		
		default:
			FATAL(ERR_EMIT, "Unknown branch condition [%d]", how);
		}
	
	
	fprintf(_ofp,	"\t.push context block cmp_%s 1\n"
					"\t_cmp32 %s,%s\n"
					"\t%s\n"
					"here:\n"
					"\tmove.4 #0 r2\n"
					"\t%s\n"
					"there:\n"
					"\tmove.4 #1 r2\n"
					"done:\n"
					"\t.pop context\n\n",
					randomString(8).c_str(),
					r1.name().c_str(),
					r2.name().c_str(),
					criteria.c_str(),
					alternate.c_str());
	
	_regs->free(r1);
	return r2;
	}

/*****************************************************************************\
|* Compare two registers and jump based on the result, exiting as soon as
|* possible.
|*
|* This is done by doing a n-bit cmp, then depending on which operation we are
|* doing, issueing possibly multiple Bxx commands. The same table as above is
|* used. Technically we would normally use the reverse operator, but since the
|* Bxx commands are so limited in scope, we use the inverse of the inverse, and
|* combine with a jmp
|*
|*      Test			Unsigned ints				Signed ints
|*   -----------+-------------------------------+---------------------------
|*		 ==		|		BEQ there				|	BEQ there
|* 		 !=		|		BNE there				|	BNE there
|*		 <		|		BCC there				|	BMI there
|* 		 >		|		BEQ here  ; BCS there	|	BEQ here  ; BPL there
|* 		 <=		|		BCC there ; BEQ there 	|	BMI there ; BEQ there
|*		 >=		|		BCS there				|	BPL there
|*
\*****************************************************************************/
Register A8Emitter::_cgCompareAndJump(Register r1,
									  Register r2,
									  int how,
									  String label)
	{
	Register none(Register::NO_REGISTER);
	String criteria 	= "beq skip\n";
	
	switch (how)
		{
		case ASTNode::A_EQ:
			break;
		
		case ASTNode::A_NE:
			criteria 	= "bne skip\n";
			break;
			
		case ASTNode::A_LT:
			criteria 	= "bmi skip\n";
			break;
			
		case ASTNode::A_GT:
			criteria	= "beq skip\n\tbpl skip\n";
			break;
			
		case ASTNode::A_LE:
			criteria	= "beq skip\n\tbmi skip\n";
			break;
			
		case ASTNode::A_GE:
			criteria	= "bpl skip\n";
			break;
		
		default:
			FATAL(ERR_EMIT, "Unknown branch condition [%d]", how);
		}
	
	/*************************************************************************\
	|* If the register widths differ, then use the larger of the two
	\*************************************************************************/
	_cgSameSize(r1, r2);
	
	/*************************************************************************\
	|* Write out the compare logic
	\*************************************************************************/
	fprintf(_ofp,	"\t_cmp%d %s,%s\n"
					"\t%s\n"
					"\tjmp %s\n"
					"skip:\n",
					r1.size()*8,
					r1.name().c_str(),
					r2.name().c_str(),
					criteria.c_str(),
					label.c_str());
	
	_regs->free(r1);
	return none;
	}

/*****************************************************************************\
|* Ensure two registers are the same size
\*****************************************************************************/
void A8Emitter::_cgSameSize(Register &r1, Register& r2)
	{
	/*************************************************************************\
	|* If the register widths differ, then use the larger of the two
	\*************************************************************************/
	int r1s = r1.size() * 8;
	int r2s = r2.size() * 8;
	
	if (r1s > r2s)
		{
		Register r = _regs->allocate(r1.type());
		fprintf(_ofp, "\t_clr%d r%d\n"
					  "\t_xfer%d r%d,r%d\n",
					  r1s,
					  r.identifier(),
					  r2s,
					  r2.identifier(),
					  r.identifier()
					  );
		_regs->free(r2);
		r2 = r;
		}
	else if (r2s > r1s)
		{
		Register r = _regs->allocate(r2.type());
		fprintf(_ofp, "\t_clr%d r%d\n"
					  "\t_xfer%d r%d,r%d\n",
					  r2s,
					  r.identifier(),
					  r1s,
					  r1.identifier(),
					  r.identifier()
					  );
		_regs->free(r1);
		r1 = r;
		}
	}

/*****************************************************************************\
|* Pop a context
\*****************************************************************************/
void A8Emitter::_cgPopContext(void)
	{
	fprintf(_ofp, "\t.pop context\n\n");
	}
	
/*****************************************************************************\
|* Push a context
\*****************************************************************************/
void A8Emitter::_cgPushContext(int type, String prefix)
	{
	static int idx = 0;
	
	idx ++;
	String ctx = "unknown";
	
	switch (type)
		{
		case C_FILE:
			ctx = "file";
			break;
		
		case C_MACRO:
			ctx = "macro";
			break;
		
		case C_FUNCTION:
			ctx = "function";
			break;
		
		case C_CLASS:
			ctx = "class";
			break;
		
		case C_BLOCK:
			ctx = "block";
			break;
		
		case C_IF:
			ctx = "if";
			break;
		
		case C_COMPARE:
			ctx = "compare";
			break;
		
		case C_WHILE:
			ctx = "while";
			break;
		
		default:
			break;
		}
		
	fprintf(_ofp, "\t.push context %s %s_%d 1\n",
			ctx.c_str(),
			prefix.c_str(),
			idx);
	}
	
/****************************************************************************\
|* Private method - jump to a label
\****************************************************************************/
void A8Emitter::_cgJump(String label)
	{
	// FIXME: Can we make this use Bxx ?
	fprintf(_ofp, "\tjmp %s\n", label.c_str());
	}
	
/****************************************************************************\
|* Private method - jump to a label
\****************************************************************************/
void A8Emitter::_cgLabel(String label)
	{
	fprintf(_ofp, "\n%s:\n", label.c_str());
	}
	
/****************************************************************************\
|* Private method - generate an IF-statement AST
\****************************************************************************/
Register A8Emitter::_cgIfAST(ASTNode *node)
	{
	Register none(Register::NO_REGISTER);

	_cgPushContext(C_IF, "if");

	// Generate the condition code followed
	// by a zero jump to the false label.
	emit(node->left(), none, node->op(), "ifNot");
	RegisterFile::clear();
	
	// Generate the true compound statement
	emit(node->mid(), none, node->op(), "ifNot");
	RegisterFile::clear();

	// If there is an optional ELSE clause,
	// generate the jump to skip to the end
	if (node->right())
		_cgJump("ifEnd");
	
	// Now the false label
	_cgLabel("ifNot");
	
	// Optional ELSE clause: generate the
	// false compound statement and the
	// end label
	if (node->right())
		{
		emit(node->right(), none, node->op(), "ifNot");
		RegisterFile::clear();
		_cgLabel("ifEnd");
		}
	
	_cgPopContext();
	return none;
	}
	
/****************************************************************************\
|* Private method - generate a WHILE-statement AST
\****************************************************************************/
Register A8Emitter::_cgWhileAST(ASTNode *node)
	{
	Register none(Register::NO_REGISTER);

	_cgPushContext(C_WHILE, "while");
	_cgLabel("start");

	// Generate the condition code followed
	// by a jump to the end label.
	emit(node->left(), none, node->op(), "end");
	RegisterFile::clear();
	
	// Generate the body compound statement
	emit(node->right(), none, node->op(), "");
	RegisterFile::clear();

	// Output the jump back to the start
	_cgJump("start");
	
	// Now the loop-exit label
	_cgLabel("end");
		
	_cgPopContext();
	return none;
	}

/*****************************************************************************\
|* Call a function
\*****************************************************************************/
Register A8Emitter::_cgCall(Register r1, int identifier)
	{
	Register r = _regs->allocate(r1.type());
	
	}
