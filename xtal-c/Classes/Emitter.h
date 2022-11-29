//
//  Emitter.h
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
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
    
    public:
		typedef enum
			{
			PREAMBLE		= 0,
			POSTAMBLE
			} Location;


		// These should match what the assembler uses for its context
		// definitions
	/*************************************************************************\
    |* Strings used
    \*************************************************************************/
	protected:
		/*********************************************************************\
        |* Important macro files
        \*********************************************************************/
		const String _printRegFile	= "printReg.s";
		const String _stdMacrosFile	= "stdmacros.s";
		
		/*********************************************************************\
        |* Information to be appended to the preamble or postamble
        \*********************************************************************/
		String		_preamble;			// Extra text for the preamble
		String		_postamble;			// Extra text for the postamble
		
		/*********************************************************************\
        |* List of included files already mentioned
        \*********************************************************************/
		std::set<String>	_includes;
	
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GET(RegisterFile *, regs);
    GETSET(FILE *, ofp, Ofp);
    
    private:

    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Emitter();
		virtual ~Emitter();
		
        /*********************************************************************\
        |* Debugging - print a register
        \*********************************************************************/
        virtual void printReg(Register r) = 0;

        /*********************************************************************\
        |* Generate the code
        \*********************************************************************/
        virtual Register emit(ASTNode *node, Register reg, int parentAstOp) =0;

        /*********************************************************************\
        |* Generate the code preamble
        \*********************************************************************/
        virtual void preamble(void);

        /*********************************************************************\
        |* Generate the code postamble
        \*********************************************************************/
        virtual void postamble(void);

        /*********************************************************************\
        |* Generate a global symbol
        \*********************************************************************/
        virtual void genSymbol(const String& name) = 0;

        /*********************************************************************\
        |* Append text to one of the body parts
        \*********************************************************************/
        virtual void append(const String& what, Location where);
	};

#endif /* Emitter_h */
