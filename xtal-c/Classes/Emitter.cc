//
//  Emitter.cc
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
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
	_includes.insert(_printRegFile);
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
					  ".include %s\n"
					  "\n"
					  "%s\n"
					  "call main\n"
					  "rts\n",
					  __TIME__, __DATE__,
					  _printRegFile.c_str(),
					  _stdMacrosFile.c_str(),
					  _xtrt0.c_str(),
					  _preamble.c_str());
		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for preamble output!");
	}

/****************************************************************************\
|* Output a function preamble
|* load the stack and adjust the pointer
\****************************************************************************/
void Emitter::functionPreamble(String name)
	{
	if (_ofp != nullptr)
		{
		fprintf(_ofp, "; Begin function\n"
					  "; --------------\n"
					  "\n.function %s\n"
					  ";.clobber a,x,y\n"
					  "@%s:\n",
					  name.c_str(),
					  name.c_str());
		
		if (_stackOffset > 0)
			fprintf(_ofp, "\t_sub16i $%x," STACK_PTR "\n", _stackOffset);
		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for func preamble output!");
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
|* Output a function postamble
\****************************************************************************/
void Emitter::functionPostamble(int funcId)
	{
	if (_ofp != nullptr)
		{
		Symbol s = SYMTAB->at(funcId);
		cgLabel(s.endLabel());
		
		if (_stackOffset > 0)
			fprintf(_ofp, "\t_add16i $%x," STACK_PTR "\n", _stackOffset);
			
		fprintf(_ofp, "\trts\n"
					  ".endfunction\n"
					  "; -------------\n"
					  "; Function ends\n\n");
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
	_stackOffset = 0;
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


