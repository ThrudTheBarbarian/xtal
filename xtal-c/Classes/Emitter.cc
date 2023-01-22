//
//  Emitter.cc
//  xtal-c
//
//  Created by Thrud The Barbarian on 11/26/22.
//

#include <sstream>
#include <iomanip>


#include "ASTNode.h"
#include "Emitter.h"
#include "RegisterFile.h"
#include "SymbolTable.h"
#include "Types.h"

/****************************************************************************\
|* Constructor
\****************************************************************************/
Emitter::Emitter()
		:_preamble("")
		,_postamble("")
		,_ofp(nullptr)
	{
	_regs 			= new RegisterFile();
	_xtrt0			= "xtrt0.s";
	_stackOffset	= 0;
	_fnParamAt		= FN_PARAM_MIN;
	}

/****************************************************************************\
|* Destructor
\****************************************************************************/
Emitter::~Emitter()
	{
	delete _regs;
	}

/****************************************************************************\
|* Output a default preamble
\****************************************************************************/
void Emitter::preamble(void)
	{
	_includes.insert(_stdMacrosFile);
	
	RegisterFile::setOutputFile(_ofp);
	RegisterFile::clear();
	
	if (_ofp != nullptr)
		{
		fprintf(_ofp, "; Assembly code produced at %s on %s\n"
					  ";\n"
					  ";\n"
					  "; Assembly begins\n"
					  "; ---------------\n"
					  ".include %s\n"
					  ".include %s\n"
					  "\n"
					  "%s\n"
					  "call main\n"
					  "rts\n",
					  __TIME__, __DATE__,
					  _stdMacrosFile.c_str(),
					  _xtrt0.c_str(),
					  _preamble.c_str());
		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for preamble output!");
	}
	
/****************************************************************************\
|* Output a function postamble
\****************************************************************************/
void Emitter::postamble(void)
	{
	if (_ofp != nullptr)
		{
		fprintf(_ofp, "%s\n"
					  "; -------------\n"
					  "; Assembly ends\n\n",
					  _postamble.c_str());
		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for postamble output!");
	}


/****************************************************************************\
|* Output a function preamble
|* load the stack and adjust the pointer
\****************************************************************************/
void Emitter::functionPreamble(int funcId)
	{
	if (_ofp != nullptr)
		{
		Symbol s 			= SYMTAB->at(funcId);
		if (s.pType() == PT_NONE)
			FATAL(ERR_TYPE, "Unknown function for id %d", funcId);
			
		const char *name	= s.name().c_str();
		
        /*********************************************************************\
        |* deal with any arguments to the function arg space
        \*********************************************************************/
		int argId = funcId + 1;
		while (SYMTAB->isValid(argId))
			{
			/*****************************************************************\
			|* Find the symbols in the global table that demarcate the
			|* function signature
			\*****************************************************************/
			Symbol& arg = SYMTAB->at(argId);
			if (arg.pType() == PT_NONE)
				FATAL(ERR_TYPE, "Unknown argument for id %d", argId);
			if (arg.sClass() != C_PARAM)
				break;
			
			/*****************************************************************\
			|* Look up the local argument of the same name, and place it in
			|* global memory (in the function argument space)
			\*****************************************************************/
			int localIdx = SYMTAB->find(arg.name(), SymbolTable::SEARCH_LOCAL);
			Symbol& localArg = SYMTAB->at(localIdx);
			if (localArg.pType() == PT_NONE)
				FATAL(ERR_TYPE, "Unknown local argument for id %d", localIdx);

			int where = -1;
			switch (arg.pType())
				{
				case PT_U8:
				case PT_S8:
				case PT_U16:
				case PT_S16:
				case PT_U32:
				case PT_S32:
					where = functionParameterLocation(localArg.pType());
					localArg.setLocation(where);
					break;
				
				case PT_U8PTR:
				case PT_U16PTR:
				case PT_U32PTR:
				case PT_S8PTR:
				case PT_S16PTR:
				case PT_S32PTR:
					// FIXME: We ought to be able to make this a no-op since
					// it's a pointer.
					break;
				
				default:
					FATAL(ERR_TYPE, "Trying to pass unknown type to '%s'",
									 name);
				}
			
			argId++;
			}


        /*********************************************************************\
        |* Write out the function
        \*********************************************************************/
		fprintf(_ofp, "; Begin function\n"
					  "; --------------\n"
					  "\n.function %s\n"
					  ";.clobber a,x,y\n"
					  "@%s:\n",
					  name,
					  name);
		
		// Manipulate the stack if necessary
		if (_stackOffset > 0)
			fprintf(_ofp, "\t_sub16i $%x," STACK_PTR "\n", _stackOffset);

		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for func preamble output!");
	}

/****************************************************************************\
|* Output a function postamble
\****************************************************************************/
void Emitter::functionPostamble(int funcId)
	{
	if (_ofp != nullptr)
		{
		Symbol s = SYMTAB->at(funcId);
		if (s.pType() == PT_NONE)
			FATAL(ERR_TYPE, "Unknown function for id %d", funcId);

		cgLabel(s.endLabel());
		
		if (_stackOffset > 0)
			fprintf(_ofp, "\t_add16i $%x," STACK_PTR "\n", _stackOffset);
			
		fprintf(_ofp, "\trts\n"
					  ".endfunction\n"
					  "; -------------\n"
					  "; Function ends\n\n");

			
		// We're done with the locals for this function now
		SYMTAB->freeLocalSymbols();
		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for fn postamble output!");
	}
	

/****************************************************************************\
|* Append text to either preamble or postamble
\****************************************************************************/
void Emitter::append(const String &what, Location where)
	{
	switch (where)
		{
		case PREAMBLE:
			_preamble += what;
			break;
		
		case POSTAMBLE:
			_postamble += what;
			break;
		
		default:
			FATAL(ERR_RUNTIME, "Append requested to unknown destination");
		}
	}


	
/*****************************************************************************\
|* Reset the position of new local variables
\*****************************************************************************/
void Emitter::genResetLocals(void)
	{
	_stackOffset 	= 0;
	_fnParamAt 		= FN_PARAM_MIN;
	}
	
/*****************************************************************************\
|* Get the location of the next local variable. Decrement the current offset
|* by the correct amount of bytes
\*****************************************************************************/
int Emitter::genGetLocalOffset(int type, bool isParam)
	{
	(void) isParam; // Currently unused
	
	int currentOffset = _stackOffset;
	_stackOffset += Types::typeSize(type);
	return currentOffset;
	}


	
/*****************************************************************************\
|* Get the location of the next local variable. Decrement the current offset
|* by the correct amount of bytes
\*****************************************************************************/
int Emitter::functionParameterLocation(int type)
	{
	int size = Types::typeSize(type);
	if (size + _fnParamAt > FN_PARAM_MAX)
		FATAL(ERR_RUNTIME, "Cannot use more than 16 bytes of function params");
	
	int location = _fnParamAt;
	_fnParamAt += size;
	return location;
	}
	
