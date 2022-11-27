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
		enum
			{
			PREAMBLE		= 0,
			CODEBODY,
			POSTAMBLE
			};
			
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
        virtual Register emit(ASTNode *node) = 0;

        /*********************************************************************\
        |* Generate the code preamble
        \*********************************************************************/
        virtual void preamble(void);

        /*********************************************************************\
        |* Generate the code postamble
        \*********************************************************************/
        virtual void postamble(void);
	};

#endif /* Emitter_h */
