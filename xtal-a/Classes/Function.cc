//
//  Function.cc
//  xtal-a
//
//  Created by Simon Gornall on 11/22/22.
//

#include <ranges>

#include "Function.h"
#include "ContextMgr.h"

#define MAX_REGISTERS			(4*4*256)
#define REG_BASE				(0xC0)

#define CTXMGR					ContextMgr::sharedInstance()

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Function::Function()
	{
	reset();
	}
	
/****************************************************************************\
|* Reset the function so it can be re-defined
\****************************************************************************/
void Function::reset(void)
	{
	_name = "undefined";
	_lines.clear();
	_clobbers.clear();
	_used = false;
	_emitted = false;
	}
	
/****************************************************************************\
|* Generate a series of assembly instructions to preserve any regs that
|* this function clobbers
|*
|* o  Note that we don't push 's' or 'f' registers, even though they ought to
|*      be listed.
|* o  Note that all regs will be lower-case
\****************************************************************************/
void Function::enstack(StringList& assembly)
	{
	for (String regName : _clobbers)
		{
		if ((regName[0] == 's') || (regName[0] == 'f'))
			continue;
		if (regName == "a")
			assembly.push_back("pha");
		else if (regName == "x")
			{
			assembly.push_back("txa");
			assembly.push_back("pha");
			}
		else if (regName == "y")
			{
			assembly.push_back("tya");
			assembly.push_back("pha");
			}
		else
			{
			// FIXME: For the time being just push all 4 bytes of an 'r'
			// register but in time we ought to have a property on the
			// register to say what size it currently exists at
			int regId;
			sscanf(regName.c_str() + 1, "%d", &regId);
			if (regId < 0 || regId > MAX_REGISTERS)
				FATAL(ERR_FUNCTION, "Attempt to push reg '%s'\n%s",
					regName.c_str(), CTXMGR->location().c_str());
			int offset = REG_BASE + 4 * (regId % 16);
			for (int i=0; i<4; i++)
				{
				char buf[1024];
				snprintf(buf, 1024, "LDA $%x", offset + i);
				assembly.push_back(buf);
				assembly.push_back("PHA");
				}
			}
		}
	}
	
/****************************************************************************\
|* Generate a series of assembly instructions to restore any regs that
|* this function clobbers
|*
|* o  Note that we don't pull 's' or 'f' registers, even though they ought to
|*      be listed.
|* o  Note that all regs will be lower-case
\****************************************************************************/
void Function::destack(StringList& assembly)
	{
	for (size_t i=_clobbers.size(); i --> 0;)
		{
		String regName = _clobbers[i];
		
		if ((regName[0] == 's') || (regName[0] == 'f'))
			continue;
		if (regName == "a")
			assembly.push_back("pla");
		else if (regName == "x")
			{
			assembly.push_back("pla");
			assembly.push_back("tax");
			}
		else if (regName == "y")
			{
			assembly.push_back("pla");
			assembly.push_back("tay");
			}
		else
			{
			// FIXME: For the time being just pull all 4 bytes of an 'r'
			// register but in time we ought to have a property on the
			// register to say what size it currently exists at
			int regId;
			sscanf(regName.c_str() + 1, "%d", &regId);
			if (regId < 0 || regId > MAX_REGISTERS)
				FATAL(ERR_FUNCTION, "Attempt to pull reg '%s'\n%s",
					regName.c_str(), CTXMGR->location().c_str());
			int offset = REG_BASE + 4 * (regId % 16);
			for (int i=3; i>=0; i--)
				{
				char buf[1024];
				assembly.push_back("pla");
				snprintf(buf, 1024, "STA $%x", offset + i);
				assembly.push_back(buf);
				}
			}
		}
	}
