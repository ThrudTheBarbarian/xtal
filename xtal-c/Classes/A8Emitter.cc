//
//  A8Emitter.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#include <sstream>
#include <iomanip>

#include "ASTNode.h"
#include "A8Emitter.h"
#include "RegisterFile.h"
#include "Stringutils.h"
#include "SymbolTable.h"

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
Register A8Emitter::emit(ASTNode *node, Register reg, int parentAstOp)
	{
	Register left, right;
	Register none(Register::NO_REGISTER);
	
	// We now have specific AST node handling at the top
	switch (node->op())
		{
		case ASTNode::A_IF:
			return (_genIfAst(node));
    
		case ASTNode::A_GLUE:
			// Do each child statement, and free the
			// registers after each child
			emit(node->left(), none, node->op());
			RegisterFile::clear();
			
			emit(node->right(), none, node->op());
			RegisterFile::clear();
			return (none);
		}
	
	if (node->left())
		left = emit(node->left(), none, node->op());
		
	if (node->right())
		right = emit(node->right(), left, node->op());

	switch (node->op())
		{
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
			if (parentAstOp == ASTNode::A_IF)
				return _cgCompareAndJump(node->op(), left, right, reg);
			else
				return _cgCompareAndSet(node-op(), left, right);
				
		case ASTNode::A_ASSIGN:			return right;
		case ASTNode::A_INTLIT:
			{
			return _cgLoadInt(node->value().intValue);
			}
		case ASTNode::A_IDENT:
			{
			auto symbol = SYMTAB->table()[node->value().identifier];
			return _cgLoadGlobal(symbol.name());
			}
		case ASTNode::A_LVIDENT:
			{
			auto symbol = SYMTAB->table()[node->value().identifier];
			return _cgStoreGlobal(reg, symbol.name());
			}
		case ASTNode::A_PRINT:
			{
			printReg(left);
			RegisterFile::clear();
			return none;
			}
		
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
void A8Emitter::genSymbol(const String& name)
	{
	char buf[1024];
	snprintf(buf, 1024, "@S_%s: .word 0,0\n", name.c_str());
	append(buf, POSTAMBLE);
	}
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Generate a load-value-to-register
\*****************************************************************************/
Register A8Emitter::_cgLoadInt(int val)
	{
	Register r	= _regs->allocate(Register::SIGNED_4BYTE);
	String size = r.sizeAsString();
	
	fprintf(_ofp, "\tmove.%d #$%x %s\n", r.size(), val, r.name().c_str());
	return r;
	}

/*****************************************************************************\
|* Generate a load-value-to-register
\*****************************************************************************/
Register A8Emitter::_cgLoadGlobal(String name)
	{
	Register r	= _regs->allocate(Register::SIGNED_4BYTE);
	String size = r.sizeAsString();
	
	fprintf(_ofp, "\tmove.%d S_%s %s\n",
				r.size(),
				name.c_str(),
				r.name().c_str());
	return r;
	}

/*****************************************************************************\
|* Store a register into a global variable
\*****************************************************************************/
Register A8Emitter::_cgStoreGlobal(Register& r, String name)
	{
	String size = r.sizeAsString();
	
	fprintf(_ofp, "\tmove.%d %s S_%s\n",
				r.size(),
				r.name().c_str(),
				name.c_str());
	return r;
	}

/*****************************************************************************\
|* Add two registers, and release the first
\*****************************************************************************/
Register A8Emitter::_cgAdd(Register r1, Register r2)
	{
	Register result	= _regs->allocate(Register::SIGNED_4BYTE);

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
	std::stringstream ss;
	Register result	= _regs->allocate(Register::SIGNED_4BYTE);

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
	Register result	= _regs->allocate(Register::SIGNED_4BYTE);

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
	Register left	= _regs->allocate(Register::SIGNED_4BYTE);
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
Register A8Emitter::_cgCompare(Register r1, Register r2, int how)
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
|* Test for equality
\*****************************************************************************/
Register A8Emitter::_cgEqual(Register r1, Register r2)
	{
	return _cgCompare(r1, r2, ASTNode::A_EQ);
	}

/*****************************************************************************\
|* Test for inequality
\*****************************************************************************/
Register A8Emitter::_cgNotEqual(Register r1, Register r2)
	{
	return _cgCompare(r1, r2, ASTNode::A_NE);
	}

/*****************************************************************************\
|* Test for less than
\*****************************************************************************/
Register A8Emitter::_cgLessThan(Register r1, Register r2)
	{
	return _cgCompare(r1, r2, ASTNode::A_LT);
	}

/*****************************************************************************\
|* Test for greater than
\*****************************************************************************/
Register A8Emitter::_cgMoreThan(Register r1, Register r2)
	{
	return _cgCompare(r1, r2, ASTNode::A_GT);
	}

/*****************************************************************************\
|* Test for less than or equal to
\*****************************************************************************/
Register A8Emitter::_cgLessOrEq(Register r1, Register r2)
	{
	return _cgCompare(r1, r2, ASTNode::A_LE);
	}

/*****************************************************************************\
|* Test for greater than or equal to
\*****************************************************************************/
Register A8Emitter::_cgMoreOrEq(Register r1, Register r2)
	{
	return _cgCompare(r1, r2, ASTNode::A_GE);
	}

