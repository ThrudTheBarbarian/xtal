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

/****************************************************************************\
|* Constructor
\****************************************************************************/
Emitter::Emitter()
		:_preamble("")
		,_postamble("")
		,_ofp(nullptr)
	{
	_regs = new RegisterFile();
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
					  "\n"
					  "%s\n"
					  "call main\n"
					  "rts\n",
					  __TIME__, __DATE__,
					  _printRegFile.c_str(),
					  _stdMacrosFile.c_str(),
					  _preamble.c_str());
		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for preamble output!");
	}

/****************************************************************************\
|* Output a function preamble
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
					  name.c_str()
					  );
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
		Symbol s = SYMTAB->table()[funcId];
		cgLabel(s.endLabel());
		
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


