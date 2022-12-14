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
#include "sharedDefines.h"
#include "Stringutils.h"
#include "SymbolTable.h"
#include "Types.h"

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
			return _cgIfAST(node);
    
		case ASTNode::A_WHILE:
			return _cgWhileAST(node);
    
		case ASTNode::A_FUNCCALL:
			return _genFuncCall(node, label);
    
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
			int funcId  = node->value().identifier;
			Symbol& symbol = SYMTAB->at(funcId);
			if (symbol.pType() == PT_NONE)
				FATAL(ERR_TYPE, "Unknown function for id %d", funcId);

			functionPreamble(funcId);
			emit(node->left(), none, node->op(), "");
			functionPostamble(funcId);
			return (none);
			}
		}
	
	if (node->left())
		left = emit(node->left(), none, node->op(), label);
		
	if (node->right())
		right = emit(node->right(), none, node->op(), label);

	switch (node->op())
		{
		case ASTNode::A_ADD:			return _cgAdd(left, right);
		case ASTNode::A_SUBTRACT:		return _cgSub(left, right);
		case ASTNode::A_MULTIPLY:		return _cgMul(left, right);
		case ASTNode::A_DIVIDE:			return _cgDiv(left, right);
		case ASTNode::A_AND:			return _cgAnd(left, right);
		case ASTNode::A_OR:				return _cgOr(left, right);
		case ASTNode::A_XOR:			return _cgXor(left, right);
		case ASTNode::A_LSHIFT:			return _cgShl(left, right);
		case ASTNode::A_RSHIFT:			return _cgShr(left, right);
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
			return _cgLoadInt((int)node->value().intValue);
			}
				
		case ASTNode::A_STRLIT:
			{
			return _cgLoadGlobalStr((int)node->value().intValue);
			}
		case ASTNode::A_IDENT:
			{
			if (node->isRValue() || (parentAstOp == ASTNode::A_DEREF))
				{
				int nodeId = node->value().identifier;
				Symbol& sym = SYMTAB->at(nodeId);
				if (sym.pType() == PT_NONE)
					FATAL(ERR_TYPE, "Unknown identifier for id %d", nodeId);

				if (sym.sClass() == C_LOCAL)
					return _cgLoadLocal(nodeId, node->op());
				else
					return _cgLoadGlob(nodeId, node->op());
				}
			else
				return none;
			}
		case ASTNode::A_ASSIGN:
			// Are we assigning to an identifier, or through a pointer
			switch (node->right()->op())
				{
				case ASTNode::A_IDENT:
					{
					int nodeId = node->right()->value().identifier;
					Symbol& sym = SYMTAB->at(nodeId);
					if (sym.pType() == PT_NONE)
						FATAL(ERR_TYPE, "Unknown identifier for id %d", nodeId);
					
					if (sym.sClass() == C_LOCAL)
						return (_cgStoreLocal(left, sym));
					else
						return _cgStoreGlobal(left, sym);
					}
				case ASTNode::A_DEREF:
					return _cgStoreDeref(left, right, node->right()->type());
				
				default:
					FATAL(ERR_PARSE, "Can't ASSIGN in emitter, op=%d",
									 node->op());
				}
		case ASTNode::A_DEREF:
			// if we're an r-value, dereference to get the value we point
			// at, otherwise leave it for A_ASSIGN to store through the
			// pointer
			if (node->isRValue())
				return _cgDeref(left, node->left()->type());
			else
				return left;
		case ASTNode::A_PRINT:
			{
			printReg(left, node->type());
			RegisterFile::clear();
			return none;
			}
		case ASTNode::A_WIDEN:
			left = _cgWiden(left, node->left()->type(), node->type());
			return left;
		case ASTNode::A_RETURN:
			_cgReturn(left, SYMTAB->functionId());
			return left;
//		case ASTNode::A_FUNCCALL:
//			return _cgCall(left, node->value().identifier);
		case ASTNode::A_ADDR:
			return _cgAddress(node->value().identifier);
		case ASTNode::A_SCALE:
			// Use a shift if the scale value is a known power of 2
			switch (node->value().size)
				{
				case 1:		return left;
				case 2:		return _cgShlConst(left, 1);
				case 4:		return _cgShlConst(left, 2);
				default:
					right = _cgLoadInt(node->value().size);
					right.setType(left.type());
					return _cgMul(left, right);
				}
			break;

		case ASTNode::A_POSTINC:
		case ASTNode::A_POSTDEC:
			// Load the variable's value into a register, then increment it
			{
			int nodeId = node->value().identifier;
			Symbol& sym = SYMTAB->at(nodeId);
			if (sym.pType() == PT_NONE)
				FATAL(ERR_TYPE, "Unknown post-op identifier for id %d", nodeId);
			if (sym.sClass() == C_LOCAL)
				return _cgLoadLocal(nodeId, node->op());
			
			return _cgLoadGlob(nodeId, node->op());
			}
		case ASTNode::A_PREINC:
		case ASTNode::A_PREDEC:
			// Load and increment the variable's value into a register
			{
			int nodeId = node->left()->value().identifier;
			Symbol& sym = SYMTAB->at(nodeId);
			if (sym.pType() == PT_NONE)
				FATAL(ERR_TYPE, "Unknown pre-op identifier for id %d", nodeId);
			if (sym.sClass() == C_LOCAL)
				return _cgLoadLocal(nodeId, node->op());
			
			return _cgLoadGlob(nodeId, node->op());
			}

		case ASTNode::A_NEGATE:
			return _cgNegate(left);

		case ASTNode::A_INVERT:
			return _cgInvert(left);

		case ASTNode::A_LOGNOT:
			return _cgLogNot(left);

		case ASTNode::A_TOBOOL:
			// If the parent AST node is an A_IF or A_WHILE, generate
			// a compare followed by a jump. Otherwise, set the register
			// to 0 or 1 based on it's zeroeness or non-zeroeness
			return (_cgBoolean(left, parentAstOp, label));

		default:
			FATAL(ERR_AST_UNKNOWN_OPERATOR, "Unknown AST operator %d", node->op());
		}
	}

	
/****************************************************************************\
|* Method - jump to a label
\****************************************************************************/
void A8Emitter::cgLabel(String label)
	{
	fprintf(_ofp, "\n%s:\n", label.c_str());
	}
	
/*****************************************************************************\
|* Dump a register to CIO0
\*****************************************************************************/
void A8Emitter::printReg(Register r, int type)
	{
	switch (type)
		{
		case PT_U8PTR:
			fprintf(_ofp, "\tcall printStr %s\n", r.name().c_str());
			break;
			
		default:
			fprintf(_ofp, "\tcall printReg %s\n", r.name().c_str());
			break;
		}
	}
	

/*****************************************************************************\
|* Determine the size of a symbol
\*****************************************************************************/
static Register::RegType _symbolSize(const Symbol& symbol)
	{
	Symbol s 	= (Symbol)symbol;
	int symType	= s.pType();
	Register::RegType type = Register::UNKNOWN;
	
	if (symType > 0xFF)	// Is a pointer type
		return Register::UNSIGNED_2BYTE;
	
	switch (symType)
		{
		case PT_S8:
			type = Register::SIGNED_1BYTE;
			break;
		case PT_U8:
			type = Register::UNSIGNED_1BYTE;
			break;
		case PT_S16:
			type = Register::SIGNED_2BYTE;
			break;
		case PT_U16:
			type = Register::UNSIGNED_2BYTE;
			break;
		case PT_S32:
			type = Register::SIGNED_4BYTE;
			break;
		case PT_U32:
			type = Register::UNSIGNED_4BYTE;
			break;
		
		default:
			FATAL(ERR_TYPE, "Illegal type 0x%x", symType);
		}
	
	return type;
	}

/*****************************************************************************\
|* Generate a global symbol
\*****************************************************************************/
void A8Emitter::genSymbol(int idx)
	{
	char buf[1024];
	
	Symbol symbol 	= SYMTAB->at(idx);
	if (symbol.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown identifier for id %d", idx);
	if (symbol.sClass() != C_GLOBAL)
		return;
		
	String name 	= symbol.name();
	snprintf(buf, 1024, "@S_%s:\n", name.c_str());
	append(buf, POSTAMBLE);
	
	for (int i=0; i<symbol.size(); i++)
		{
		String str;
		
		switch (_symbolSize(symbol))
			{
			case Register::UNSIGNED_4BYTE:
			case Register::SIGNED_4BYTE:
				str = "\t.word 0,0\n";
				break;
			case Register::UNSIGNED_2BYTE:
			case Register::SIGNED_2BYTE:
				str = "\t.word 0\n";
				break;
			case Register::UNSIGNED_1BYTE:
			case Register::SIGNED_1BYTE:
				str = "\t.byte 0\n";
				break;
			default:
				{
				FATAL(ERR_TYPE, "Unknown size for symbol %s", symbol.name().c_str());
				}
			}
		append(str, POSTAMBLE);
		}
	}

/*****************************************************************************\
|* Generate a global string symbol
\*****************************************************************************/
int A8Emitter::genGlobalString(String content)
	{
	static int stringId = 0;
	char name[1024];
	snprintf(name, 1024, "str_%d", stringId++);
	
	int symIdx = SYMTAB->addGlobal(name,
								  PT_U8PTR,
								  ST_ARRAY,
								  (int)content.length());
	
	char buf[1024];
	snprintf(buf, 1024, "@S_%s:\n", name);
	append(buf, POSTAMBLE);
	
	String define = ".byte ";
	String comma  = "";
	
	for (int i=0; i<(int)content.length(); i++)
		{
		define += comma + toHexString(content[i], "$");
		comma   = ",";
		}
	define += ",0\n";
	
	append(define, POSTAMBLE);
	return symIdx;
	}
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Generate a load-value-to-register
\*****************************************************************************/
Register A8Emitter::_cgLoadInt(int val, int primitiveType)
	{
	(void)primitiveType;
	
	REG type 	= ((val >= 0) && (val <= 255))   ? Register::UNSIGNED_1BYTE
				: ((val >= -128) && (val <= 127))   ? Register::SIGNED_1BYTE
				: ((val >= 0) && (val <= 65535)) ? Register::UNSIGNED_2BYTE
				: ((val >= -32768) && (val <= 32767)) ? Register::SIGNED_2BYTE
				: (val > 2147483647) ? Register::UNSIGNED_4BYTE
				: Register::SIGNED_4BYTE;
	Register r	= _regs->allocate(type);
	
	fprintf(_ofp, "\tmove.%d #$%x %s\n", r.size(), val, r.name().c_str());
	return r;
	}

/*****************************************************************************\
|* Generate a load-value-to-register
\*****************************************************************************/
Register A8Emitter::_cgLoadGlobalStr(int val)
	{
	Register r	= _regs->allocate(Register::UNSIGNED_2BYTE);
	Symbol& s	= SYMTAB->at(val);
	if (s.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown identifier for id %d", val);
	
	fprintf(_ofp, "\tmove.2 #S_%s %s\n",s.name().c_str(), r.name().c_str());
	return r;
	}
	
/*****************************************************************************\
|* Store a register into a global variable
\*****************************************************************************/
Register A8Emitter::_cgStoreGlobal(Register& r, const Symbol& symbol)
	{
	Symbol s 	= (Symbol)symbol;
	REG size	= _symbolSize(symbol);

	/*************************************************************************\
    |* Do some zeroing checks if the register size and the symbol size do not
    |* match
    \*************************************************************************/
	_cgExtendIfNeeded(r, s.pType());
	
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
|* Store a register into a global variable
\*****************************************************************************/
Register A8Emitter::_cgStoreLocal(Register& r, const Symbol& symbol)
	{
	Symbol s 	= (Symbol)symbol;

	/*************************************************************************\
    |* If the register width and the symbol width differ, allocate a new
    |* register (since they are packed in there, now) of the same size as
    |* the symbol and sign/zero extend it
    \*************************************************************************/
    if (s.pType() > 0xFF)
    
	r = _cgExtendIfNeeded(r, s.pType());
	
	/*************************************************************************\
    |* We now have a sign-or-zero-extended value in 'r'. Need to find the
    |* location of the local variable and store the data from 'r' into that
    |* address
    \*************************************************************************/
	Register sp 		= _regs->allocateForPrimitiveType(PT_U16);
	const char * spName = sp.name().c_str();
	
	if (s.position() == 0)
		fprintf(_ofp, 	"\t_xfer16 " STACK_PTR ",%s\n" ,spName);
	else
		fprintf(_ofp, 	"\t_xfer16 " STACK_PTR ",%s\n"
						"\t_add16i %d,%s\n"
						,spName,
						s.position(), spName);

	/*************************************************************************\
    |* Store the data in 'r' to the address in 'sp'
    \*************************************************************************/
	r = _cgStoreDeref(r, sp, s.pType());
	_regs->free(sp);
	
	return r;
	}

/*****************************************************************************\
|* Widen a register
\*****************************************************************************/
Register A8Emitter::_cgWiden(Register& reg, int oldWidth, int newWidth)
	{
//	int oSize = Types::typeSize(oldWidth);
//	const char *regName = reg.name().c_str();
//	bool isSigned		= (oldWidth == PT_S8)
//					   || (oldWidth == PT_S16)
//					   || (oldWidth == PT_S32);

//	if (_regs->widen(reg, oldWidth, newWidth) == false)
//		return _cgAllocAndWiden(reg, oldWidth, newWidth);
		
	if (oldWidth != newWidth)
		{
		reg = _cgExtendIfNeeded(reg, newWidth);
//		switch (newWidth)
//			{
//			case PT_S8:
//				reg.setType(Register::SIGNED_1BYTE);
//				break;
//
//			case PT_U8:
//				reg.setType(Register::UNSIGNED_1BYTE);
//				break;
//
//			case PT_S16:
//			case PT_U16:
//				if (isSigned == false)
//					fprintf(_ofp, "\tlda #$0 ; 1u->2*\n"
//								  "\tsta %s+1\n",
//								  regName);
//				else
//					fprintf(_ofp, "\t.push context block widen_%s 1\n"
//								  "\tlda #0 ; 1s -> 2*\n"
//								  "\tbit %s\n"
//								  "\tbpl zeroExtend\n"
//								  "\tlda #$ff\n"
//								  "zeroExtend:\n"
//							      "\tsta %s+1\n"
//								  "\t.pop context\n",
//								  randomString(6).c_str(),
//								  regName, regName);
//				reg.setType(newWidth == PT_S16 ? Register::SIGNED_2BYTE
//											   : Register::UNSIGNED_2BYTE);
//				break;
//
//
//			case PT_U32:
//			case PT_S32:
//				if (oSize == 1)
//					{
//					if (isSigned == false)
//						fprintf(_ofp, "\tlda #0 ; 1u->4*\n"
//									  "\tsta %s+1\n"
//									  "\tsta %s+2\n"
//									  "\tsta %s+3\n",
//									  regName, regName, regName);
//
//					else
//						fprintf(_ofp, "\t.push context block widen_%s 1\n"
//									  "\tlda #0 ; 1s->4*\n"
//									  "\tbit %s\n"
//									  "\tbpl zeroExtend\n"
//									  "\tlda #$ff\n"
//									  "zeroExtend:\n"
//									  "\tsta %s+1\n"
//									  "\tsta %s+2\n"
//									  "\tsta %s+3\n"
//									  "\t.pop context\n",
//									  randomString(6).c_str(),
//									  regName, regName, regName, regName);
//					}
//				else if (oSize == 2)
//					{
//					if (isSigned == false)
//						fprintf(_ofp, "\tlda #0 ; 2u->4*\n"
//								      "\tsta %s+2\n"
//								      "\tsta %s+3\n",
//								      regName, regName);
//					else
//						fprintf(_ofp, "\t.push context block widen_%s 1\n"
//								      "\tlda #0; 2s->4*\n"
//								      "\tbit %s+1\n"
//								      "\tbpl zeroExtend\n"
//								      "\tlda #$ff\n"
//								      "zeroExtend:\n"
//							          "\tsta %s+2\n"
//								      "\tsta %s+3\n"
//								      "\t.pop context\n",
//								      randomString(6).c_str(),
//								      regName, regName, regName);
//					}
//				reg.setType(newWidth == PT_S32 ? Register::SIGNED_4BYTE
//											   : Register::UNSIGNED_4BYTE);
//				break;
//
//			default:
//				FATAL(ERR_TYPE, "Unknown type for reg %s", reg.name().c_str());
//			}
		}
	
	return reg;
	}

        
/*****************************************************************************\
|* Widen a register
\*****************************************************************************/
Register A8Emitter::_cgAllocAndWiden(Register& reg, int oldWidth, int newWidth)
	{
	int oSize 			= Types::typeSize(oldWidth);
	bool isSigned		= (oldWidth == PT_S8)
					   || (oldWidth == PT_S16)
					   || (oldWidth == PT_S32);

	Register nReg 		= _regs->allocateForPrimitiveType(newWidth);
	
	const char * oName 	= reg.name().c_str();
	const char * nName 	= nReg.name().c_str();
	
	switch (newWidth)
		{
		case PT_U8:
		case PT_S8:
			fprintf(_ofp, "\tlda %s ; move\n"
						  "\tsta %s\n",
						  oName, nName);
			break;
		
		case PT_S16:
		case PT_U16:
			if (isSigned == false)
				fprintf(_ofp, "\tlda %s; move 1u->2*\n"
							  "\tsta %s\n"
							  "\tlda #$0 \n"
							  "\tsta %s+1\n",
							  oName, nName, nName);
			else
				fprintf(_ofp, "\t.push context block widen_%s 1\n"
							  "lda %s; move 1s -> 2*\n"
							  "sta %s\n"
							  "\tlda #0 \n"
							  "\tbit %s\n"
							  "\tbpl zeroExtend\n"
							  "\tlda #$ff\n"
							  "zeroExtend:\n"
							  "\tsta %s+1\n"
							  "\t.pop context\n",
							  randomString(6).c_str(),
							  oName, nName, oName, nName);
			reg.setType(newWidth == PT_S16 ? Register::SIGNED_2BYTE
										   : Register::UNSIGNED_2BYTE);
			break;
			
		
		case PT_U32:
		case PT_S32:
			if (oSize == 1)
				{
				if (isSigned == false)
					fprintf(_ofp, "\tlda %s; move 1u->4*\n"
								  "\tsta %s\n"
								  "\tlda #0 \n"
								  "\tsta %s+1\n"
								  "\tsta %s+2\n"
								  "\tsta %s+3\n",
								  oName, nName, nName, nName, nName);

				else
					fprintf(_ofp, "\t.push context block widen_%s 1\n"
								  "\tlda %s; move 1s->4*\n"
								  "\tsta %s\n"
								  "\tlda #0 \n"
								  "\tbit %s\n"
								  "\tbpl zeroExtend\n"
								  "\tlda #$ff\n"
								  "zeroExtend:\n"
								  "\tsta %s+1\n"
								  "\tsta %s+2\n"
								  "\tsta %s+3\n"
								  "\t.pop context\n",
								  randomString(6).c_str(),
								  oName, nName, oName, nName, nName, nName);
				}
			else if (oSize == 2)
				{
				if (isSigned == false)
					fprintf(_ofp, "\tlda %s; move 2u->4*\n"
								  "\tsta %s\n"
								  "\tlda %s\n"
								  "\tsta %s\n"
								  "\tlda #0 \n"
								  "\tsta %s+2\n"
								  "\tsta %s+3\n",
								  oName, nName,
								  oName, nName,
								  nName, nName);
				else
					fprintf(_ofp, "\t.push context block widen_%s 1\n"
								  "\tlda %s; move 2s->4*\n"
								  "\tsta %s\n"
								  "\tlda %s\n"
								  "\tsta %s\n"
								  "\tlda #0 \n"
								  "\tbit %s+1\n"
								  "\tbpl zeroExtend\n"
								  "\tlda #$ff\n"
								  "zeroExtend:\n"
								  "\tsta %s+2\n"
								  "\tsta %s+3\n"
								  "\t.pop context\n",
								  randomString(6).c_str(),
								  oName, nName,
								  oName, nName,
								  oName,
								  nName, nName);
				}
			reg.setType(newWidth == PT_S32 ? Register::SIGNED_4BYTE
										   : Register::UNSIGNED_4BYTE);
			break;
		
		default:
			FATAL(ERR_TYPE, "Unknown type for reg %s", reg.name().c_str());
		}
	
	_regs->free(reg);
	return nReg;
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

	if (!_regs->free(r1))
		fprintf(_ofp, "; *** Failed to free register %s\n", r1.name().c_str());
	if (!_regs->free(r2))
		fprintf(_ofp, "; *** Failed to free register %s\n", r2.name().c_str());
	
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
	Register r3			= _regs->allocate(r1.type());
	
	int size			= r1.size();
	int bits			= 8*size;
	
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
					"\t_cmp%d %s,%s\n"
					"\t%s\n"
					"here:\n"
					"\tmove.%d #0 %s\n"
					"\t%s\n"
					"there:\n"
					"\tmove.%d #1 %s\n"
					"done:\n"
					"\t.pop context\n\n",
					randomString(8).c_str(),
					bits,
					r1.name().c_str(),
					r2.name().c_str(),
					criteria.c_str(),
					size,
					r3.name().c_str(),
					alternate.c_str(),
					size,
					r3.name().c_str());
	
	_regs->free(r1);
	_regs->free(r2);
	return r3;
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
	cgLabel("ifNot");
	
	// Optional ELSE clause: generate the
	// false compound statement and the
	// end label
	if (node->right())
		{
		emit(node->right(), none, node->op(), "ifNot");
		RegisterFile::clear();
		cgLabel("ifEnd");
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
	cgLabel("start");

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
	cgLabel("end");
		
	_cgPopContext();
	return none;
	}

/*****************************************************************************\
|* Call a function
\*****************************************************************************/
Register A8Emitter::_cgCall(int symIdx)
	{
	Symbol& symbol = SYMTAB->at(symIdx);
	if (symbol.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown identifier for id %d", symIdx);

	fprintf(_ofp, "\tcall %s\n", symbol.name().c_str());

	if (symbol.pType() == PT_VOID)
		{
		Register none(Register::NO_REGISTER);
		return none;
		}

	// Get a new out-register
	Register r = _regs->allocateForPrimitiveType(symbol.pType());
	
	
	// Move the returned value of the function to the register we created
	int fId = 0;
	const char *reg = r.name().c_str();
	
	// Zero-fill the values in case they're used later
	switch (Types::typeSize(symbol.pType()))
		{
		case 4:
			fprintf(_ofp, "\t_xfer32 f%d, %s\n", fId, reg);
			break;
			
		case 2:
			fprintf(_ofp, "\t_xfer16 f%d, %s\n",
						  fId, reg);
			break;
		case 1:
			fprintf(_ofp, "\tlda f%d\n"
						  "\tsta %s\n",
						  fId, reg);
			break;
		}
	return r;
	}

/*****************************************************************************\
|* Return a value from a function
\*****************************************************************************/
void A8Emitter::_cgReturn(Register r1, int funcId)
	{
	Symbol& s = SYMTAB->at(funcId);
	if (s.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown function identifier for id %d", funcId);
	
	switch (s.pType())
		{
		case PT_U8:
		case PT_S8:
			fprintf(_ofp, "\tmove.1 %s f0\n", r1.name().c_str());
			break;
		
		case PT_U16:
		case PT_S16:
			fprintf(_ofp, "\tmove.2 %s f0\n", r1.name().c_str());
			break;
		
		case PT_U32:
		case PT_S32:
			fprintf(_ofp, "\tmove.4 %s f0\n", r1.name().c_str());
			break;
		
		default:
			FATAL(ERR_FUNCTION, "Incorrect return type");
			break;
		}
	_cgJump(s.endLabel());
	}

/*****************************************************************************\
|* Generate code to load the address of a global identifier into a register
\*****************************************************************************/
Register A8Emitter::_cgAddress(int identifier)
	{
	// Get a new out-register
	Register r 	= _regs->allocate(Register::UNSIGNED_2BYTE);
	
	// Fetch the symbol
	Symbol s 	= SYMTAB->at(identifier);
	if (s.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown address identifier for id %d", identifier);
	if (s.sClass() == C_LOCAL)
		{
		if (s.position() != 0)
			fprintf(_ofp, "\tmove.2 " STACK_PTR " %s; offset = %d\n"
					  "\t_add16i $%x,%s\n"
					, r.name().c_str(), s.position()
					, s.position(), r.name().c_str());
		else
			fprintf(_ofp, "\tmove.2 " STACK_PTR " %s ; offset = 0\n"
					, r.name().c_str());
		}
	else
		fprintf(_ofp, "\tmove.2 #S_%s %s\n",
						s.name().c_str(),
						r.name().c_str());
	return r;
	}

/*****************************************************************************\
|* Generate code to load the value of the pointer into the same register
\*****************************************************************************/
Register A8Emitter::_cgDeref(Register r1, int type)
	{
	// Get a new out-register
	Register r 	= _regs->allocateForPrimitiveType(type & 0xff);

	fprintf(_ofp, "\tldy #0; _cgDeref (%d)\n", type);
	for (int i=0; i<r.size(); i++)
		{
		fprintf(_ofp, "\tlda (%s),y\n"
					  "\tsta %s+%d\n",
					  r1.name().c_str(),
					  r.name().c_str(),
					  i);
		if (i < r.size()-1)
			fprintf(_ofp, "\tiny\n");
		}

	_regs->free(r1);
	return r;
	}

/*****************************************************************************\
|* Store a register into a global variable
\*****************************************************************************/
Register A8Emitter::_cgStoreDeref(Register& r1, Register& r2, int type)
	{
	const char *name1 = r1.name().c_str();
	const char *name2 = r2.name().c_str();
	
	switch (type)
		{
		case PT_S8:
		case PT_U8:
			fprintf(_ofp, "\tlda %s ; cgStoreDeref 1\n"
						  "\tldy #0\n"
						  "\tsta (%s),y\n",
						  name1, name2);
			break;
			
		case PT_S16:
		case PT_U16:
		case PT_S8PTR:
		case PT_U8PTR:
		case PT_S16PTR:
		case PT_U16PTR:
		case PT_S32PTR:
		case PT_U32PTR:
			fprintf(_ofp, "\tlda %s ; cgStoreDeref 2\n"
						  "\tldy #0\n"
						  "\tsta (%s),y\n"
						  "\tiny\n"
						  "\tlda %s+1\n"
						  "\tsta (%s),y\n",
						  name1, name2, name1, name2);
			break;
			
		case PT_S32:
		case PT_U32:
			fprintf(_ofp, "\tlda %s ; cgStoreDeref 4\n"
						  "\tldy #0\n"
						  "\tsta (%s),y\n"
						  "\tiny\n"
						  "\tlda %s+1\n"
						  "\tsta (%s),y\n"
						  "\tiny\n"
						  "\tlda %s+2\n"
						  "\tsta (%s),y\n"
						  "\tiny\n"
						  "\tlda %s+3\n"
						  "\tsta (%s),y\n",
						  name1, name2, name1, name2,
						  name1, name2, name1, name2);
			break;
		
		default:
			FATAL(ERR_TYPE, "Can't deref through type %d", type);
		}
	return r1;
	}

/*****************************************************************************\
|* Shift a register by a constant
\*****************************************************************************/
Register A8Emitter::_cgShlConst(Register r1, int amount)
	{
	int size = r1.size() * 8;		// bits not bytes
	
	for (int i=0; i<amount; i++)
		fprintf(_ofp, "\t_asl%d %s,%s\n",
					  size,
					  r1.name().c_str(),
					  r1.name().c_str());
	return r1;
	}

/*****************************************************************************\
|* Perform an AND
\*****************************************************************************/
Register A8Emitter::_cgAnd(Register r1, Register r2)
	{
	int size = r1.size()*8;		// will be the same for both
	
	fprintf(_ofp, "\t_and%d %s,%s\n",
					  size,
					  r1.name().c_str(),
					  r2.name().c_str());
	_regs->free(r1);
	return r2;
	}

/*****************************************************************************\
|* Perform an OR
\*****************************************************************************/
Register A8Emitter::_cgOr(Register r1, Register r2)
	{
	int size = r1.size()*8;		// will be the same for both
	
	fprintf(_ofp, "\t_or%d %s,%s\n",
					  size,
					  r1.name().c_str(),
					  r2.name().c_str());
	_regs->free(r1);
	return r2;
	}

/*****************************************************************************\
|* Perform an XOR
\*****************************************************************************/
Register A8Emitter::_cgXor(Register r1, Register r2)
	{
	int size = r1.size()*8;		// will be the same for both
	
	fprintf(_ofp, "\t_eor%d %s,%s\n",
					  size,
					  r1.name().c_str(),
					  r2.name().c_str());
	_regs->free(r1);
	return r2;
	}
	
/*****************************************************************************\
|* Shift a register by a constant, only copes with left-shift by <255 steps
\*****************************************************************************/
Register A8Emitter::_cgShl(Register r1, Register r2)
	{
	int size 			= r1.size() * 8;		// bits not bytes
	const char *reg1	= r1.name().c_str();
	const char *reg2	= r2.name().c_str();
	
	fprintf(_ofp,	"\t.push context block shl_%s 1\n"
					"\tldx %s\n"
					"loop:\n"
					"\t_asl%d %s,%s\n"
					"\tdex\n"
					"\tbne loop\n"
					"\t.pop context\n",
					randomString(8).c_str(),
					reg2,
					size, reg1, reg1);
	_regs->free(r2);
	return r1;
	}
	
/*****************************************************************************\
|* Shift a register by a constant, only copes with left-shift by <255 steps
\*****************************************************************************/
Register A8Emitter::_cgShr(Register r1, Register r2)
	{
	int size 			= r1.size() * 8;		// bits not bytes
	const char *reg1	= r1.name().c_str();
	const char *reg2	= r2.name().c_str();
	
	fprintf(_ofp,	"\t.push context block shr_%s 1\n"
					"\tldx %s\n"
					"loop:\n"
					"\t_lsr%d %s,%s\n"
					"\tdex\n"
					"\tbne loop\n"
					"\t.pop context\n",
					randomString(8).c_str(),
					reg2,
					size, reg1, reg1);
	_regs->free(r2);
	return r1;
	}
	
/*****************************************************************************\
|* Load a symbol into a register, given an identifier. Return the register
|* If the operation is pre/post inc/dec also perform that operation
\*****************************************************************************/
Register A8Emitter::_cgLoadLocal(int identifier, int op)
	{
	Symbol s  			= SYMTAB->at(identifier);
	if (s.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown local identifier for id %d", identifier);
	
	/*************************************************************************\
    |* Get a new register, U16PTR to do stack arithmetic with
    \*************************************************************************/
	Register r			= _regs->allocateForPrimitiveType(PT_U16);
	const char *rTemp	= r.name().c_str();
	
	fprintf(_ofp, "\n\n; _cgloadLocal %s %d\n", s.name().c_str(), op);
	
	/*************************************************************************\
    |* Get the address of the symbol relative to the current stack pointer
    \*************************************************************************/
	if (s.position() == 0)
		fprintf(_ofp, 	"\t_xfer16 " STACK_PTR ",%s\n" ,rTemp);
	else
		fprintf(_ofp, 	"\t_xfer16 " STACK_PTR ",%s\n"
						"\t_add16i %d,%s\n"
						,rTemp,
						s.position(), rTemp);
	
	/*************************************************************************\
    |* Dereference this pointer into a new register the same size as the symbol
    \*************************************************************************/
	int objType 		= (s.pType() > 0xFF) ? PT_U16PTR : s.pType();
	
	Register var 		= _cgDeref(r, objType);
	const char *name	= var.name().c_str();
	
	/*************************************************************************\
    |* If we have no post/pre inc/dec then we're done. Return the register
    \*************************************************************************/
	switch (op)
		{
		case ASTNode::A_PREINC:
		case ASTNode::A_PREDEC:
		case ASTNode::A_POSTINC:
		case ASTNode::A_POSTDEC:
			break;
		
		default:
			return var;
			break;
		}
		
	/*************************************************************************\
    |* Handle the pre/post inc/dec
    \*************************************************************************/
	switch (s.pType())
		{
		case PT_U8:
		case PT_S8:
			if (op == ASTNode::A_PREINC)
				{
				fprintf(_ofp, "\tinc %s\n"
							  "\t_xferp8 %s,%s\n"
							, name, name, rTemp);
				_cgStoreLocal(var, s);
				}
			else if (op == ASTNode::A_PREDEC)
				{
				fprintf(_ofp, "\tdec %s\n"
							  "\t_xferp8 %s,%s\n"
							, name, name, rTemp);
				_cgStoreLocal(var, s);
				}
			else if (op == ASTNode::A_POSTINC)
				fprintf(_ofp, "\t_inc8p %s,1\n", rTemp);
			else if (op == ASTNode::A_POSTDEC)
				fprintf(_ofp, "\t_dec8p %s,1\n", rTemp);
			break;
		
		case PT_U16:
		case PT_S16:
		case PT_U8PTR:
		case PT_S8PTR:
			if (op == ASTNode::A_PREINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n"
							  "\t_xferp16 %s,%s\n"
							, name, name, rTemp);
				_cgStoreLocal(var, s);
				}
			else if (op == ASTNode::A_PREDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n"
							  "\t_xferp16 %s,%s\n"
							, name, name, rTemp);
				_cgStoreLocal(var, s);
				}
			else if (op == ASTNode::A_POSTINC)
				fprintf(_ofp, "\t_inc16p %s,1\n", rTemp);
			else if (op == ASTNode::A_POSTDEC)
				fprintf(_ofp, "\t_dec16p %s,1\n", rTemp);
			break;
		
		case PT_U32:
		case PT_S32:
			if (op == ASTNode::A_PREINC)
				fprintf(_ofp, "\t_inc32 %s\n"
							  "\t_xferp32 %s,%s\n"
							, name, name, rTemp);
			else if (op == ASTNode::A_PREDEC)
				fprintf(_ofp, "\t_dec32 %s\n"
							  "\t_xferp32 %s,%s\n"
							, name, name, rTemp);
			else if (op == ASTNode::A_POSTINC)
				fprintf(_ofp, "\t_inc32p %s,1\n", rTemp);
			else if (op == ASTNode::A_POSTDEC)
				fprintf(_ofp, "\t_dec32p %s,1\n", rTemp);
			break;
		
		case PT_U16PTR:
		case PT_S16PTR:
			if (op == ASTNode::A_PREINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n"
							  "\t_inc16 %s\n"
							  "\t_xferp16 %s,%s\n"
							, name, name, name, rTemp);
				}
			else if (op == ASTNode::A_PREDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n"
							  "\t_dec16 %s\n"
							  "\t_xferp16 %s,%s\n"
							, name, name, name, rTemp);
				}
			else if (op == ASTNode::A_POSTINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				}
			else if (op == ASTNode::A_POSTDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				}
			break;
		
		case PT_S32PTR:
		case PT_U32PTR:
			if (op == ASTNode::A_PREINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n"
							  "\t_inc16 %s\n"
							  "\t_inc16 %s\n"
							  "\t_inc16 %s\n"
							  "\t_xferp32 %s,%s\n"
							  , name, name, name, name
							  , name, rTemp);
				}
			else if (op == ASTNode::A_PREDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n"
							  "\t_dec16 %s\n"
							  "\t_dec16 %s\n"
							  "\t_dec16 %s\n"
							  "\t_xferp16 %s,%s\n"
							, name, name, name, name
							, name, rTemp);
				}
			else if (op == ASTNode::A_POSTINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				}
			else if (op == ASTNode::A_POSTDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				}
			break;
		
		default:
			FATAL(ERR_TYPE, "Unknown type %d in loadGlobal", s.pType());
		}
	
	return var;
	}
	
/*****************************************************************************\
|* Load a symbol into a register, given an identifier. Return the register
|* If the operation is pre/post inc/dec also perform that operation
\*****************************************************************************/
Register A8Emitter::_cgLoadGlob(int identifier, int op)
	{
	Symbol s  			= SYMTAB->at(identifier);
	if (s.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown global identifier for id %d", identifier);
		
	Register r 			= _regs->allocateForPrimitiveType(s.pType());
	
	String symName		= "S_"+s.name();
	if (s.sClass() == C_PARAM)
		symName = toHexString(s.location(), "$");
		
	const char *name 	= (char *) symName.c_str();
	const char *reg		= r.name().c_str();
	
	fprintf(_ofp, "\n;_cgLoadGlob %s %d\n", name, op);
	
	switch (s.pType())
		{
		case PT_U8:
		case PT_S8:
			if (op == ASTNode::A_PREINC)
				fprintf(_ofp, "\tinc %s\n", name);
			else if (op == ASTNode::A_PREDEC)
				fprintf(_ofp, "\tdec %s\n", name);
			
			fprintf(_ofp, "\t_xfer8 %s,%s\n", name, reg);
			
			if (op == ASTNode::A_POSTINC)
				fprintf(_ofp, "\tinc %s\n", name);
			else if (op == ASTNode::A_POSTDEC)
				fprintf(_ofp, "\tdec %s\n", name);
			break;
		
		case PT_U16:
		case PT_S16:
		case PT_U8PTR:
		case PT_S8PTR:
			if (op == ASTNode::A_PREINC)
				fprintf(_ofp, "\t_inc16 %s\n", name);
			else if (op == ASTNode::A_PREDEC)
				fprintf(_ofp, "\t_dec16 %s\n", name);
			
			fprintf(_ofp, "\t_xfer16 %s,%s\n", name, reg);
			
			if (op == ASTNode::A_POSTINC)
				fprintf(_ofp, "\t_inc16 %s\n", name);
			else if (op == ASTNode::A_POSTDEC)
				fprintf(_ofp, "\t_dec16 %s\n", name);
			break;
		
		case PT_U32:
		case PT_S32:
			if (op == ASTNode::A_PREINC)
				fprintf(_ofp, "\t_inc32 %s\n", name);
			else if (op == ASTNode::A_PREDEC)
				fprintf(_ofp, "\t_dec32 %s\n", name);
			
			fprintf(_ofp, "\t_xfer32 %s,%s\n", name, reg);
			
			if (op == ASTNode::A_POSTINC)
				fprintf(_ofp, "\t_inc32 %s\n", name);
			else if (op == ASTNode::A_POSTDEC)
				fprintf(_ofp, "\t_dec32 %s\n", name);
			break;
		
		case PT_U16PTR:
		case PT_S16PTR:
			if (op == ASTNode::A_PREINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				}
			else if (op == ASTNode::A_PREDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				}
				
			fprintf(_ofp, "\t_xfer16 %s,%s\n", name, reg);
			
			if (op == ASTNode::A_POSTINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				}
			else if (op == ASTNode::A_POSTDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				}
			break;
		
		case PT_S32PTR:
		case PT_U32PTR:
			if (op == ASTNode::A_PREINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				}
			else if (op == ASTNode::A_PREDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				}
				
			fprintf(_ofp, "\t_xfer16 %s,%s\n", name, reg);
			
			if (op == ASTNode::A_POSTINC)
				{
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				fprintf(_ofp, "\t_inc16 %s\n", name);
				}
			else if (op == ASTNode::A_POSTDEC)
				{
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				fprintf(_ofp, "\t_dec16 %s\n", name);
				}
			break;
		
		default:
			FATAL(ERR_TYPE, "Unknown type %d in loadGlobal", s.pType());
		}
		
	return r;
	}
	
/*****************************************************************************\
|* Negate a value
\*****************************************************************************/
Register A8Emitter::_cgNegate(Register r)
	{
	Register::RegType type = r.type();
	
	switch (r.type())
		{
		case Register::UNSIGNED_1BYTE:
			type = Register::SIGNED_1BYTE;
			break;
		
		case Register::UNSIGNED_2BYTE:
			type = Register::SIGNED_2BYTE;
			break;
		
		case Register::UNSIGNED_4BYTE:
			type = Register::SIGNED_4BYTE;
			break;
		
		default:
			break;
		}
	
	Register r2 = _regs->allocate(type);
	
	int size = r.size()*8;		// will be the same for both
	fprintf(_ofp, "\t_neg%d %s,%s\n",
					  size,
					  r.name().c_str(),
					  r2.name().c_str());
	_regs->free(r);
	return r2;
	}
	
/*****************************************************************************\
|* Invert a value
\*****************************************************************************/
Register A8Emitter::_cgInvert(Register r)
	{
	int size = r.size()*8;		// will be the same for both
	
	fprintf(_ofp, "\t_not%d %s,%s\n",
					  size,
					  r.name().c_str(),
					  r.name().c_str());
	return r;
	}
	
/*****************************************************************************\
|* Compute the logical NOT of a register (ie: test for it being 0, if so
|* return 1, else return 0
\*****************************************************************************/
Register A8Emitter::_cgLogNot(Register r)
	{
	const char *reg		= r.name().c_str();

	fprintf(_ofp, "\t.push context block lognot_%s 1\n",
				  randomString(8).c_str());

	switch (r.size())
		{
		case 1:
			fprintf(_ofp, "\t_cmpi8 %s,0\n"
						  "\tbeq isTrue\n"
						  "\tlda #0\n"
						  "\tsta %s\n"
						  "\tbeq testDone\n"
						  "isTrue:\n"
						  "\tlda #1\n"
						  "\tsta %s\n"
						  "testDone:\n",
						  reg, reg, reg);
			break;
		
		case 2:
			fprintf(_ofp, "\t_cmpi16 %s,0\n"
						  "\tbeq isTrue\n"
						  "\tlda #0\n"
						  "\tsta %s\n"
						  "\tsta %s+1\n"
						  "\tbeq testDone\n"
						  "isTrue:\n"
						  "\tlda #1\n"
						  "\tsta %s\n"
						  "\tlda #0\n"
						  "\tsta %s+1\n"
						  "testDone:\n",
						  reg, reg, reg, reg, reg);
			break;
		
		case 4:
			fprintf(_ofp, "\t_cmpi32 %s,0\n"
						  "\tbeq isTrue\n"
						  "\tlda #0\n"
						  "\tsta %s\n"
						  "\tsta %s+1\n"
						  "\tsta %s+2\n"
						  "\tsta %s+3\n"
						  "\tbeq testDone\n"
						  "isTrue:\n"
						  "\tlda #1\n"
						  "\tsta %s\n"
						  "\tlda #0\n"
						  "\tsta %s+1\n"
						  "\tsta %s+2\n"
						  "\tsta %s+3\n"
						  "testDone:\n",
						  reg, reg, reg, reg, reg,
						  reg, reg, reg, reg);
			break;
		}
	
	fprintf(_ofp, ".pop context\n");
	
	return r;
	}
	
/*****************************************************************************\
|* If the parent AST node is an A_IF or A_WHILE, generate a compare followed
|* by a jump. Otherwise, set the register to 0 or 1 based on it's zeroeness
|* or non-zeroeness
\*****************************************************************************/
Register A8Emitter::_cgBoolean(Register r, int parentOp, String label)
	{
	String jmp 			= label+"_skip_"+randomString(8);
	const char *reg		= r.name().c_str();
	
	switch (r.size())
		{
		case 1:
			fprintf(_ofp, "\t_cmpi8 %s,0\n", reg);
			if ((parentOp == ASTNode::A_IF) || (parentOp == ASTNode::A_WHILE))
				fprintf(_ofp, "\tbne %s\n"
						      "\tjmp %s\n"
						      "%s:\n",
						      jmp.c_str(), label.c_str(), jmp.c_str());
			else
				fprintf(_ofp, "\tlda #1\n"
						      "\tsta %s\n",
						      reg);
			break;
		
		case 2:
			fprintf(_ofp, "\t_cmpi16 %s,0\n", reg);
			if ((parentOp == ASTNode::A_IF) || (parentOp == ASTNode::A_WHILE))
				fprintf(_ofp, "\tbne %s\n"
						      "\tjmp %s\n"
						      "%s:\n",
						      jmp.c_str(), label.c_str(), jmp.c_str());
			else
				fprintf(_ofp, "\tlda #1\n"
						      "\tsta %s\n"
							  "\tlda #0\n"
						      "\tsta %s+1\n",
						      reg, reg);
			break;
		
		case 4:
			fprintf(_ofp, "\t_cmpi8 %s,0\n", reg);
			if ((parentOp == ASTNode::A_IF) || (parentOp == ASTNode::A_WHILE))
				fprintf(_ofp, "\tbne %s\n"
						      "\tjmp %s\n"
						      "%s:\n",
						      jmp.c_str(), label.c_str(), jmp.c_str());
			else
				fprintf(_ofp, "\tlda #1\n"
						      "\tsta %s\n"
							  "\tlda #0\n"
						      "\tsta %s+1\n"
						      "\tsta %s+2\n"
						      "\tsta %s+3\n",
						      reg, reg, reg, reg);
			break;
		
		}
	
	return r;
	}

	
/*****************************************************************************\
|* Invert a value
\*****************************************************************************/
Register A8Emitter::_cgExtendIfNeeded(Register r, int pType)
	{
	/*************************************************************************\
    |* If the register width and the symbol width differ, allocate a new
    |* register (since they are packed in there, now) of the same size as
    |* the symbol and sign/zero extend it
    \*************************************************************************/
	if (Types::typeSize(pType) != r.size())
		{
		Register copy 	= _regs->allocateForPrimitiveType(pType);
		int type		= pType & 0xFF;
		
		if ((type == PT_U32) || (type ==PT_S32))
			{
			if (r.size() == 1)
				{
				if (r.type() > 0xFF)	// Signed!
					{
					fprintf(_ofp, "\t.push context block lextend_%s 1\n"
								  "\tlda #0\n"
								  "\tbit r%d\n"
								  "\tbpl zeroExtend\n"
								  "\tlda #$ff\n"
								  "zeroExtend:\n"
								  "\tsta r%d + 3\n"
								  "\tsta r%d + 2\n"
								  "\tsta r%d + 1\n"
								  "\t.pop context\n",
								  randomString(6).c_str(),
								  r.identifier(),
								  copy.identifier(),
								  copy.identifier(),
								  copy.identifier());
					}
				else
					{
					fprintf(_ofp, "\tlda #0\n"
								  "\tsta r%d + 3\n"
								  "\tsta r%d + 2\n"
								  "\tsta r%d + 1\n",
								  copy.identifier(),
								  copy.identifier(),
								  copy.identifier());
					}
				}
				
			else if (r.size() == 2)
				{
				if (r.type() > 0xFF) 	// Signed!
					{
					fprintf(_ofp, "\t.push context block lextend_%s 1\n"
							  "\tlda #0\n"
							  "\tbit r%d+1\n"
							  "\tbpl zeroExtend\n"
							  "\tlda #$ff\n"
							  "zeroExtend:\n"
							  "\tsta r%d + 2\n"
							  "\tsta r%d + 3\n"
							  "\t.pop context\n",
							  randomString(6).c_str(),
							  r.identifier(),
							  copy.identifier(),
							  copy.identifier());
					}
				else
					{
					fprintf(_ofp, "\tlda #0\n"
								  "\tsta r%d + 2\n"
								  "\tsta r%d + 3\n",
								  copy.identifier(),
								  copy.identifier());
					}
				}
			}
		else if ((type = PT_U16) || (type == PT_S16))
			{
			if (r.size() == 1)
				{
				if (r.type() > 0xFF)	// Signed!
					{
					fprintf(_ofp, "\t.push context block lextend_%s 1\n"
								  "\tlda #0\n"
								  "\tbit r%d\n"
								  "\tbpl zeroExtend\n"
								  "\tlda #$ff\n"
								  "zeroExtend:\n"
								  "\tsta r%d + 1\n"
								  "\t.pop context\n",
								  randomString(6).c_str(),
								  r.identifier(),
								  copy.identifier());
					}
				else
					{
					fprintf(_ofp, "\tlda #0\n"
								  "\tsta r%d + 1\n",
								  copy.identifier());
					}
				}
			}

		
		/*********************************************************************\
		|* Then copy the actual data into 'copy'
		\*********************************************************************/
		int copySize = (r.size() < Types::typeSize(pType))
					 ? r.size()
					 : Types::typeSize(pType);
					 
		switch (copySize)
			{
			case 1:
				fprintf(_ofp, "\tmove.1 %s %s\n",
							r.name().c_str(),
							copy.name().c_str());
				break;
				
			case 2:
				fprintf(_ofp, "\tmove.2 %s %s\n",
							r.name().c_str(),
							copy.name().c_str());
				break;
				
			case 4:
				fprintf(_ofp, "\tmove.4 %s %s\n",
							r.name().c_str(),
							copy.name().c_str());
				break;
			
			default:
				FATAL(ERR_TYPE, "Unknown register-size in extend");
			}
			
		/*********************************************************************\
		|* Associate 'r' with 'copy'
		\*********************************************************************/
		_regs->free(r);
		copy.setPrimitiveType(pType);
		return copy;
		}
		
	return r;
	}
	
/*****************************************************************************\
|* Copy any function arguments to the correct memory, and call a function
\*****************************************************************************/
Register A8Emitter::_genFuncCall(ASTNode *node, String label)
	{
	ASTNode *gluetree 	= node->left();
	Register none(Register::NO_REGISTER);

	StringList saves;
	StringList moves;
	StringList restore;

	_fnParamAt = FN_PARAM_MIN;

	/************************************************************************\
    |* If there is a list of arguments, walk this list
	\************************************************************************/
	int sv = FN_PARAM_MIN;
	while (gluetree)
		{
		// Calculate the expression's value
		Register reg 	= emit(gluetree->right(), none, gluetree->op(), label);
		int at			= functionParameterLocation(reg.primitiveType());
		
		switch (reg.size())
			{
			case 1:
				saves.push_back("\t_xferpn8"
							    +toHexString(sv, " $")
							    +",SP"
							    +toHexString(sv-FN_PARAM_MIN, ",$"));
				restore.insert(restore.begin(),
								"\t_xferbn8"
							    +toHexString(sv, " $")
							    +",SP"
							    +toHexString(sv-FN_PARAM_MIN, ",$"));
				sv ++;
				moves.push_back("\tmove.1 " + reg.name() + toHexString(at, " $"));
				break;
			
			case 2:
				saves.push_back("\t_xferpn16"
							    +toHexString(sv, " $")
							    +",SP"
							    +toHexString(sv-FN_PARAM_MIN, ",$"));
				restore.insert(restore.begin(),
								"\t_xferbn16"
							    +toHexString(sv, " $")
							    +",SP"
							    +toHexString(sv-FN_PARAM_MIN, ",$"));
				sv +=2;
				moves.push_back("\tmove.2 " + reg.name() + toHexString(at, " $"));
				break;
			
			case 4:
				saves.push_back("\t_xferpn32"
							    +toHexString(sv, " $")
							    +",SP"
							    +toHexString(sv-FN_PARAM_MIN, ",$"));
				restore.insert(restore.begin(),
								"\t_xferbn32"
							    +toHexString(sv, " $")
							    +",SP"
							    +toHexString(sv-FN_PARAM_MIN, ",$"));
				sv +=4;
				moves.push_back("\tmove.4 " + reg.name() + toHexString(at, " $"));
				break;
			
			default:
				FATAL(ERR_PARSE, "Unknown register size %d", reg.size());
			}
			
		gluetree = gluetree->left();
		}

	/************************************************************************\
    |* Make sure we're not going over
	\************************************************************************/
	if (sv >= FN_PARAM_MAX)
		FATAL(ERR_FUNCTION, "Cannot call function with >%d bytes of args",
							FN_PARAM_MAX - FN_PARAM_MIN+1);

	/************************************************************************\
    |* Show some info in the assembly
	\************************************************************************/
	int bytes = sv-FN_PARAM_MIN;

	int nodeId = node->value().identifier;
	Symbol sFn = SYMTAB->at(nodeId);
	if (sFn.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown function identifier for id %d", nodeId);

	const char * fnName = sFn.name().c_str();
	fprintf(_ofp, "\n; Function call %s [%d bytes]\n;\n", fnName, bytes);
	

	/************************************************************************\
    |* See if we need to preserve the function args - there's no need if we
    |* are in main()
	\************************************************************************/
	if (SYMTAB->currentFunction().name() != "main")
		{
		// Adjust the stack pointer down by the number of bytes we need
		if (bytes > 0)
			fprintf(_ofp, "\t;push stack by %d\n"
						  "\t_sub16i %d,SP\n", bytes, bytes);
			
		// Copy the function register data to the stack storage
		for (String cmd : saves)
			fprintf(_ofp, "%s\n", cmd.c_str());
		}
		
	/************************************************************************\
    |* Update the function-calling args
	\************************************************************************/
	for (String cmd : moves)
		fprintf(_ofp, "%s\n", cmd.c_str());

	/************************************************************************\
    |* Call the function and return its result
	\************************************************************************/
	Register ret = _cgCall(node->value().identifier);


	/************************************************************************\
    |* Restore the stashed values if we preserved them
	\************************************************************************/
	if (SYMTAB->currentFunction().name() != "main")
		{
		// Copy the function register data to the stack storage
		for (String cmd : restore)
			fprintf(_ofp, "%s\n", cmd.c_str());

		// Adjust the stack pointer down by the number of bytes we used
		if (bytes > 0)
			fprintf(_ofp, "\t; pop stack by %d\n"
						  "\t_add16i %d,SP\n", bytes, bytes);
		}
		
	//RegisterFile::clear();
	return ret;
	}

