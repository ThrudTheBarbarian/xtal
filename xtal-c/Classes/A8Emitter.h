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

#include "Emitter.h"

class ASTNode;
class RegisterFile;
class Register;

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
        Register _cgLoadInt(int value);
        
		/*********************************************************************\
        |* Fetch a global var
        \*********************************************************************/
        Register _cgLoadGlobal(String name);
        
		/*********************************************************************\
        |* Store a global var
        \*********************************************************************/
        Register _cgStoreGlobal(Register& reg, String name);
        
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
        
       
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit A8Emitter();
		~A8Emitter();
        
        /*********************************************************************\
        |* Debugging - print a register
        \*********************************************************************/
        void printReg(Register r);

        /*********************************************************************\
        |* Generate the code
        \*********************************************************************/
        Register emit(ASTNode *node, Register reg);
	};

#endif /* ! A8Emitter_h */
