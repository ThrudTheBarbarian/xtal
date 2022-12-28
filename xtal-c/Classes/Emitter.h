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
		/*********************************************************************\
		|* Add to preamble/postamble using these constants
		\*********************************************************************/
		typedef enum
			{
			PREAMBLE		= 0,
			POSTAMBLE
			} Location;

		/*********************************************************************\
		|* Function parameter passing region definition
		\*********************************************************************/
		static const int FN_PARAM_MIN			= 0xA0;
		static const int FN_PARAM_MAX			= 0xAF;
		
	protected:
		/*********************************************************************\
        |* Important macro files
        \*********************************************************************/
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

		/*********************************************************************\
        |* Stack manipulation
        \*********************************************************************/
        int _stackOffset;		// Stack pointer

		/*********************************************************************\
        |* Next function parameter location
        \*********************************************************************/
        int _fnParamAt;		// Actual memory location
		
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GET(RegisterFile *, regs);
    GETSET(FILE *, ofp, Ofp);
    GETSET(String, xtrt0, Xtrt0);		// XT runtime 0 setup file
    
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
        virtual void printReg(Register r, int type) = 0;

        /*********************************************************************\
        |* Generate the code
        \*********************************************************************/
        virtual Register emit(ASTNode *node,
							  Register reg,
							  int parentAstOp,
							  String label) =0;
	
        /*********************************************************************\
        |* Generate the code preamble
        \*********************************************************************/
        virtual void preamble(void);

        /*********************************************************************\
        |* Generate a function preamble
        \*********************************************************************/
        virtual void functionPreamble(int funcId);

        /*********************************************************************\
        |* Generate the code postamble
        \*********************************************************************/
        virtual void postamble(void);

        /*********************************************************************\
        |* Generate the code postamble
        \*********************************************************************/
        virtual void functionPostamble(int funcId);

        /*********************************************************************\
        |* Generate a global symbol by reference to the symbol table index
        \*********************************************************************/
        virtual void genSymbol(int symIdx) = 0;

        /*********************************************************************\
        |* Generate a global symbol by reference to the symbol table index
        \*********************************************************************/
        virtual int genGlobalString(String content) = 0;

        /*********************************************************************\
        |* Append text to one of the body parts
        \*********************************************************************/
        virtual void append(const String& what, Location where);

		/*********************************************************************\
        |* Emit a jump-to-a-label command
        \*********************************************************************/
        virtual void cgLabel(String label) = 0;

		/*********************************************************************\
        |* Reset the position of new local variables
        \*********************************************************************/
        void genResetLocals(void);

		/*********************************************************************\
        |* Reset the position of new local variables
        \*********************************************************************/
        int genGetLocalOffset(int type, bool isParam);

		/*********************************************************************\
        |* Return the next position of a function parameter
        \*********************************************************************/
        int functionParameterLocation(int type);
	};

#endif /* Emitter_h */
