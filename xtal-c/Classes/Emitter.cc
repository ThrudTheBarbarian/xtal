//
//  Emitter.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#include <sstream>
#include <iomanip>

#include "ASTNode.h"
#include "Emitter.h"
#include "RegisterFile.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Emitter::Emitter()
		:_output("")
	{
	_regs = new RegisterFile();

	/*************************************************************************\
	|* Create default preamble
	\*************************************************************************/
	std::stringstream ss;
	ss  << "; Assembly code produced at "
		<< __TIME__ << " on " << __DATE__ << "\n;\n"
		<< ";\n; Assembly begins\n; ----\n\n";
	_preamble = ss.str();

	/*************************************************************************\
	|* Create default postamble
	\*************************************************************************/
	_postamble = "; ----\n; Assembly complete\n\n";
	}


/*****************************************************************************\
|* Destructor
\*****************************************************************************/
Emitter::~Emitter()
	{
	delete _regs;
	}
	
/*****************************************************************************\
|* Emit code
\*****************************************************************************/
Register Emitter::emit(ASTNode *node)
	{
	Register left, right;
	
	if (node->left())
		left = emit(node->left());
		
	if (node->right())
		right = emit(node->right());

	switch (node->op())
		{
		case ASTNode::A_ADD:			return _cgAdd(left, right);
		case ASTNode::A_SUBTRACT:		return _cgSub(left, right);
		case ASTNode::A_MULTIPLY:		return _cgMul(left, right);
		case ASTNode::A_DIVIDE:		return _cgDiv(left, right);
		case ASTNode::A_INTLIT:		return _cgLoad(node->intValue());
		
		default:
			FATAL(ERR_AST_UNKNOWN_OPERATOR, "Unknown operator %d", node->op());
		}
	}

	
/*****************************************************************************\
|* Dump a register to CIO0
\*****************************************************************************/
void Emitter::printReg(Register r)
	{
	if (_includes.find(_printRegFile) == _includes.end())
		{
		_includes.insert(_printRegFile);
		_preamble += ".include " + _printRegFile + "\n";
		}
		
	std::stringstream ss;
	String size = r.sizeAsString();
	ss << "\tcall printReg " << r.name() << std::endl;
	_output += ss.str();
	}
	
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Generate a load-value-to-register
\*****************************************************************************/
Register Emitter::_cgLoad(int val)
	{
	Register r	= _regs->allocate(Register::SIGNED_4BYTE);
	String size = r.sizeAsString();
	
	std::stringstream ss;
	ss 	<< "\tmove." << size << " #$" << std::hex << val << " "
		<< r.name() << std::endl;
	_output += ss.str();
	return r;
	}

/*****************************************************************************\
|* Add two registers, and release the first
\*****************************************************************************/
Register Emitter::_cgAdd(Register r1, Register r2)
	{
	String op 		= "\taddu.";
	int separator	= Register::UNSIGNED_4BYTE;
	if ((r1.type() > separator) || (r2.type() > separator))
		op = "\tadds.";
	op += r2.sizeAsString();

	std::stringstream ss;
	ss 	<< op << " "
		<< r1.name() << " "
		<< r2.name() << std::endl;
	_output += ss.str();
	_regs->free(r1);
	return r2;
	}

/*****************************************************************************\
|* Multiply two registers, and release the first
\*****************************************************************************/
Register Emitter::_cgMul(Register r1, Register r2)
	{
	std::stringstream ss;

	String op 		= "mulu.";
	int separator	= Register::UNSIGNED_4BYTE;
	if ((r1.type() > separator) || (r2.type() > separator))
		op = "muls.";
	op += r2.sizeAsString();
	
	ss << "\t" << op << " " << r1.name() << " " << r2.name() << std::endl;
	_output += ss.str();
	_regs->free(r1);
	return r2;
	}

/*****************************************************************************\
|* Subtract two registers, and release the first
\*****************************************************************************/
Register Emitter::_cgSub(Register r1, Register r2)
	{
	String op 		= "\tsubu.";
	int separator	= Register::UNSIGNED_4BYTE;
	if ((r1.type() > separator) || (r2.type() > separator))
		op = "\tsubs.";
	op += r2.sizeAsString();

	std::stringstream ss;
	ss 	<< op << " "
		<< r1.name() << " "
		<< r2.name() << std::endl;
	_output += ss.str();
	_regs->free(r1);
	return r2;
	}
	
/*****************************************************************************\
|* Multiply two registers, and release the first
\*****************************************************************************/
Register Emitter::_cgDiv(Register r1, Register r2)
	{
	std::stringstream ss;
	String op 		= "divu.";
	int separator	= Register::UNSIGNED_4BYTE;
	if ((r1.type() > separator) || (r2.type() > separator))
		op = "divs.";
	op += r2.sizeAsString();
	
	ss << "\t" << op << " " << r1.name() << " " << r2.name() << std::endl;
	_output += ss.str();
	_regs->free(r1);
	return r2;
	}

