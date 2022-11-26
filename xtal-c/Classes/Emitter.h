//
//  Emitter.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#ifndef Emitter_h
#define Emitter_h

#include <cstdio>
#include <string>
#include <set>

#include "properties.h"
#include "macros.h"

class ASTNode;
class RegisterFile;
class Register;

class Emitter
	{
    NON_COPYABLE_NOR_MOVEABLE(Emitter)

	/*************************************************************************\
    |* Strings used
    \*************************************************************************/
	const String _printRegFile	= "printReg.s";
	const String _stdMacrosFile	= "stdmacros.s";
	
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GET(RegisterFile *, regs);
    GET(String, output);
    GET(String, preamble);
    GET(String, postamble);
    
    private:
		/*********************************************************************\
        |* List of included files already mentioned
        \*********************************************************************/
		std::set<String>	_includes;
		
		/*********************************************************************\
        |* Generate a register load immediate
        \*********************************************************************/
        Register _cgLoad(int value);
        
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
        explicit Emitter();
		~Emitter();

        /*********************************************************************\
        |* Generate a preamble
        \*********************************************************************/
        void emitPreamble(void);
        
        /*********************************************************************\
        |* Generate a postamble
        \*********************************************************************/
        void emitPostamble(void);
        
        /*********************************************************************\
        |* Debugging - print a register
        \*********************************************************************/
        void printReg(Register r);

        /*********************************************************************\
        |* Generate the code
        \*********************************************************************/
        Register emit(ASTNode *node);
	};

#endif /* Emitter_h */
