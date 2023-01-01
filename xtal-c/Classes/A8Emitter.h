//
//  Emitter.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#ifndef A8Emitter_h
#define A8Emitter_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"
#include "sharedDefines.h"

#include "Emitter.h"
#include "Register.h"

class ASTNode;
class RegisterFile;
class Symbol;

class A8Emitter : public Emitter
	{
    NON_COPYABLE_NOR_MOVEABLE(A8Emitter)

	/*************************************************************************\
    |* Public consts
    \*************************************************************************/
    
    private:
        
		/*********************************************************************\
        |* Generate a register load immediate
        \*********************************************************************/
        Register _cgLoadInt(int value, int type = PT_S8);
        
		/*********************************************************************\
        |* Generate a global string load immediate
        \*********************************************************************/
        Register _cgLoadGlobalStr(int value);
        
		/*********************************************************************\
        |* Load the variable's value into a register
        \*********************************************************************/
        Register _cgLoadGlob(int identifier, int op);
        
		/*********************************************************************\
        |* Load the variable's value into a register
        \*********************************************************************/
        Register _cgLoadLocal(int identifier, int op);
        
		/*********************************************************************\
        |* Store a global var
        \*********************************************************************/
        Register _cgStoreGlobal(Register& reg, const Symbol& symbol);
        
		/*********************************************************************\
        |* Store a local var
        \*********************************************************************/
        Register _cgStoreLocal(Register& reg, const Symbol& symbol);
        
		/*********************************************************************\
        |* Store through a dereference of a pointer
        \*********************************************************************/
        Register _cgStoreDeref(Register& reg, Register &r2, int type);
        
		/*********************************************************************\
        |* Add 2 registers
        \*********************************************************************/
        Register _cgAdd(Register r1, Register r2);
        
		/*********************************************************************\
        |* Subtract 2 registers
        \*********************************************************************/
        Register _cgSub(Register r1, Register r2);
        
		/*********************************************************************\
        |* Multiply 2 registers
        \*********************************************************************/
        Register _cgMul(Register r1, Register r2);
        
		/*********************************************************************\
        |* Divide 2 registers
        \*********************************************************************/
        Register _cgDiv(Register r1, Register r2);
        
		/*********************************************************************\
        |* Test for equality
        \*********************************************************************/
        Register _cgEqual(Register r1, Register r2);
        
		/*********************************************************************\
        |* Test for inequality
        \*********************************************************************/
        Register _cgNotEqual(Register r1, Register r2);
        
		/*********************************************************************\
        |* Test for less than
        \*********************************************************************/
        Register _cgLessThan(Register r1, Register r2);
        
		/*********************************************************************\
        |* Test for greater than
        \*********************************************************************/
        Register _cgMoreThan(Register r1, Register r2);
        
		/*********************************************************************\
        |* Test for less than or equal to
        \*********************************************************************/
        Register _cgLessOrEq(Register r1, Register r2);
        
		/*********************************************************************\
        |* Test for greater than or equal to
        \*********************************************************************/
        Register _cgMoreOrEq(Register r1, Register r2);
        
		/*********************************************************************\
        |* Generic compare code
        \*********************************************************************/
        Register _cgCompare(Register r1, Register r2, int how);
        
		/*********************************************************************\
        |* Push a context for the assembler
        \*********************************************************************/
        void _cgPushContext(int type, String prefix);
        
		/*********************************************************************\
        |* Pop a context for the assembler
        \*********************************************************************/
        void _cgPopContext(void);
        
		/*********************************************************************\
        |* Emit a jump-to-a-label command
        \*********************************************************************/
        void _cgJump(String label);
        
		/*********************************************************************\
        |* Widen a register, this can possibly result in a new register
        |* allocation, if there isn't space in the current setup to do the
        |* widen
        \*********************************************************************/
        Register _cgWiden(Register& reg, int oldWidth, int newWidth);
        
		/*********************************************************************\
        |* Widen a register, this is the new register allocation path, if
        |* there wasn't space around the current register to do the widen
        \*********************************************************************/
        Register _cgAllocAndWiden(Register& reg, int oldWidth, int newWidth);
        
 			
		/*********************************************************************\
        |* Ensure two registers are the same size.
		\*********************************************************************/
		void _cgSameSize(Register &r1, Register& r2);
         
		/*********************************************************************\
        |* Handle a function call
        \*********************************************************************/
        Register _cgCall(int symIdx);
         
		/*********************************************************************\
        |* Handle a function call
        \*********************************************************************/
        void _cgReturn(Register r1, int funcId);
			
		/*********************************************************************\
        |* Generate an IF statement AST.
		\*********************************************************************/
		Register _cgIfAST(ASTNode *node);

		/*********************************************************************\
        |* Generate a WHILE statement AST.
		\*********************************************************************/
		Register _cgWhileAST(ASTNode *node);

		/*********************************************************************\
        |* Handle the compare part of an IF statement
		\*********************************************************************/
		Register _cgCompareAndJump(Register r1,
								   Register r2,
								   int how,
								   String label);

		/*********************************************************************\
        |* Handle comparisons outside of an IF statement
		\*********************************************************************/
		Register _cgCompareAndSet(Register r1, Register r2, int how);

		/*********************************************************************\
        |* Generate code to load the address of a global identifier into a
        |* register
		\*********************************************************************/
		Register _cgAddress(int identifier);

		/*********************************************************************\
        |* Dereference a pointer to get the value pointed to, and place in the
        |* same register
		\*********************************************************************/
		Register _cgDeref(Register r1, int type);
        
		/*********************************************************************\
        |* Shift a register by a constant amount
        \*********************************************************************/
        Register _cgShlConst(Register r1, int amount);
        
		/*********************************************************************\
        |* Perform an AND op on two registers
        \*********************************************************************/
        Register _cgAnd(Register r1, Register r2);
        
		/*********************************************************************\
        |* Perform an AND op on two registers
        \*********************************************************************/
        Register _cgOr(Register r1, Register r2);
        
		/*********************************************************************\
        |* Perform an AND op on two registers
        \*********************************************************************/
        Register _cgXor(Register r1, Register r2);
        
		/*********************************************************************\
        |* Perform a shift-left op on two registers
        \*********************************************************************/
        Register _cgShl(Register r1, Register r2);
        
		/*********************************************************************\
        |* Perform a shift-left op on two registers
        \*********************************************************************/
        Register _cgShr(Register r1, Register r2);
        
		/*********************************************************************\
        |* Negate a register
        \*********************************************************************/
        Register _cgNegate(Register r);

		/*********************************************************************\
        |* Invert the bits in a rgister
        \*********************************************************************/
        Register _cgInvert(Register r);

		/*********************************************************************\
        |* Logical not
        \*********************************************************************/
        Register _cgLogNot(Register r);

		/*********************************************************************\
        |* Logical true from expression
        \*********************************************************************/
        Register _cgBoolean(Register r, int parentOp, String label);

		/*********************************************************************\
        |* Extend a register if necessary
        \*********************************************************************/
        Register _cgExtendIfNeeded(Register r, int pType);

		/*********************************************************************\
        |* Generate a function call
        \*********************************************************************/
        Register _genFuncCall(ASTNode *node, String label);

		
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit A8Emitter();
		~A8Emitter();
        
        /*********************************************************************\
        |* Debugging - print a register
        \*********************************************************************/
        void printReg(Register r, int type);

        /*********************************************************************\
        |* Generate the code
        \*********************************************************************/
        Register emit(ASTNode *node,
					  Register reg,
					  int parentAstOp,
					  String label);

        /*********************************************************************\
        |* Generate a global symbol by reference to the symbol table index
        \*********************************************************************/
        virtual void genSymbol(int symIdx);

        /*********************************************************************\
        |* Generate a global string symbol
        \*********************************************************************/
        virtual int genGlobalString(String );

		/*********************************************************************\
        |* Emit a jump-to-a-label command
        \*********************************************************************/
        void cgLabel(String label);
        
	};

#endif /* ! A8Emitter_h */
