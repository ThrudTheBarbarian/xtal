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
Register A8Emitter::emit(ASTNode *node, Register reg)
	{
	Register left, right;
	Register none(Register::NO_REGISTER);
	
	
	if (node->left())
		left = emit(node->left(), none);
		
	if (node->right())
		right = emit(node->right(), left);

	switch (node->op())
		{
		case ASTNode::A_ADD:			return _cgAdd(left, right);
		case ASTNode::A_SUBTRACT:		return _cgSub(left, right);
		case ASTNode::A_MULTIPLY:		return _cgMul(left, right);
		case ASTNode::A_DIVIDE:			return _cgDiv(left, right);
		case ASTNode::A_INTLIT:			return _cgLoadInt(node->intValue());
		case ASTNode::A_ASSIGN:			return right;
		case ASTNode::A_IDENT:
			{
			auto symbol = SYMTAB[node->identifier()];
			return _cgLoadGlobal(symbol.name());
			}
		case ASTNode::A_LVIDENT:
			{
			auto symbol = SYMTAB[node->identifier()];
			return _cgStoreGlobal(reg, symbol.name());
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

