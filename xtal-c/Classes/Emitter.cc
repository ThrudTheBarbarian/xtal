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

/****************************************************************************\
|* Constructor
\****************************************************************************/
Emitter::Emitter()
		:_ofp(nullptr)
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
	
	if (_ofp != nullptr)
		{
		fprintf(_ofp, "; Assembly code produced at %s on %s\n"
					  ";\n"
					  ";\n"
					  "; Assembly begins\n"
					  "; ---------------\n"
					  ".include %s\n"
					  ".include %s\n"
					  "\n",
					  __TIME__, __DATE__,
					  _printRegFile.c_str(),
					  _stdMacrosFile.c_str());
		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for preamble output!");
	}

/****************************************************************************\
|* Output a default postamble
\****************************************************************************/
void Emitter::postamble(void)
	{
	if (_ofp != nullptr)
		{
		fprintf(_ofp, "\trts\n"
					  "\n"
					  "; ----\n"
					  "; Assembly ends\n\n");
		}
	else
		FATAL(ERR_OUTPUT, "No file handle available for preamble output!");
	}
	
