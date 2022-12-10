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

class ASTNode;
class RegisterFile;
class Register;
class Symbol;

class A8Emitter : public Emitter
	{
    NON_COPYABLE_NOR_MOVEABLE(A8Emitter)

	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    
    private:
		/*********************************************************************\
        |* Generate a register load immediate
        \*********************************************************************/
        Register _cgLoadInt(int value, int type = PT_U8);
        
		/*********************************************************************\
        |* Generate a global string load immediate
        \*********************************************************************/
        Register _cgLoadGlobalStr(int value);
        
		/*********************************************************************\
        |* Fetch a global var
        \*********************************************************************/
        Register _cgLoadGlobal(const Symbol& symbol);
        
		/*********************************************************************\
        |* Store a global var
        \*********************************************************************/
        Register _cgStoreGlobal(Register& reg, const Symbol& symbol);
        
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
        |* Widen a register
        \*********************************************************************/
        void _cgWiden(Register& reg, int oldWidth, int newWidth);
        
 			
		/*********************************************************************\
        |* Ensure two registers are the same size.
		\*********************************************************************/
		void _cgSameSize(Register &r1, Register& r2);
         
		/*********************************************************************\
        |* Handle a function call
        \*********************************************************************/
        Register _cgCall(Register r1, int identifier);
         
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
        virtual int genString(String );

		/*********************************************************************\
        |* Emit a jump-to-a-label command
        \*********************************************************************/
        void cgLabel(String label);
        
	};

#endif /* ! A8Emitter_h */
