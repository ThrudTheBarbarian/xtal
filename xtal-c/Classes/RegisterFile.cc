//
//  RegisterFile.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#include "RegisterFile.h"
#include "sharedDefines.h"
#include "Types.h"

#define REG_PAGE_SPACE	(16*4)
#define MEM_EMPTY		(0xFFFFFFFF)

static uint32_t _regSpace[REG_PAGE_SPACE];		// Space to allocate to regs
static std::vector<Register>	_allocated;		// List of current registers
static FILE * _ofp;								// Asembly output file

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
RegisterFile::RegisterFile()
	{
	/************************************************************************\
    |* Clear the register-space
    \************************************************************************/
	clear();
	}

/*****************************************************************************\
|* Clear the register space
\*****************************************************************************/
void RegisterFile::clear(void)
	{
	//printf("** clear **\n");
	
	_allocated.clear();
	memset(_regSpace, MEM_EMPTY & 0xFF, sizeof(uint32_t)* REG_PAGE_SPACE);

	if (_ofp != nullptr)
		fprintf(_ofp, "\t.reg reset\n");
	}

/*****************************************************************************\
|* Set the assembly output file pointer
\*****************************************************************************/
void RegisterFile::setOutputFile(FILE *fp)
	{
	_ofp = fp;
	}

/*****************************************************************************\
|* Allocate a register and return the reference
\*****************************************************************************/
Register RegisterFile::allocateForPrimitiveType(int ptype)
	{
	Register::RegType type;
	
	switch (ptype & 0xFF)
		{
		case PT_S8:
			type = Register::SIGNED_1BYTE;
			break;
		case PT_U8:
			type = Register::UNSIGNED_1BYTE;
			break;
		case PT_S16:
			type = Register::SIGNED_2BYTE;
			break;
		case PT_U16:
			type = Register::UNSIGNED_2BYTE;
			break;
		case PT_S32:
			type = Register::SIGNED_4BYTE;
			break;
		case PT_U32:
			type = Register::UNSIGNED_4BYTE;
			break;
		default:
			FATAL(ERR_REG_NOEXIST, "Undefined type %d requested", ptype);
		}
		
	return allocate(type);
	}
	
/*****************************************************************************\
|* Allocate a register and return the reference
\*****************************************************************************/
Register RegisterFile::allocate(Register::RegType type)
	{
	Register r;
	r.setType(type);

	//printf("Allocating register for size %d\n", type & 0xff);
	//dump();
	/************************************************************************\
    |* Look for the first place we can place this reg
    \************************************************************************/
	int offset	= _findSpace(type & 0xFF);
	//printf("Allocated: %d\n\n\n", offset);
	
	if (offset >= 0)
		{
		int set = offset / 16;
		switch (set)
			{
			case 0:
				r.setSet(Register::SET_C0CF);
				break;
			case 1:
				r.setSet(Register::SET_D0DF);
				break;
			case 2:
				r.setSet(Register::SET_E0EF);
				break;
			case 3:
				r.setSet(Register::SET_F0FF);
				break;
			default:
				FATAL(ERR_REG_ALLOC, "Ran out of register space!");
			}
		r.setOffset(offset);
		
		char buf[1024];
		snprintf(buf, 1024, "r%d", offset);
		r.setName(buf);
		r.setIdentifier(offset);
		_populate(r);
		}
	else
		FATAL(ERR_REG_ALLOC, "Cannot allocate register");

	if (_ofp != nullptr)
		fprintf(_ofp, "\t.reg %s %d\n", r.name().c_str(), r.size());

	_allocated.push_back(r);
	return _allocated[_allocated.size()-1];
	}
	
/*****************************************************************************\
|* Widen a register
\*****************************************************************************/
bool RegisterFile::widen(Register& reg, int oldWidth, int newWidth)
	{
	if (oldWidth == newWidth)
		return true;
	
	int delta = Types::typeSize(newWidth) - Types::typeSize(oldWidth);
	if (delta < 0)
		return true;
	
	// Find out where we are, and see if there is sufficient space after the
	// current register
	int at = /*(reg.set() * 16) +*/ reg.offset() + reg.size();
	
	bool haveSpace = true;
	for (int i=0; i<delta; i++)
		if (_regSpace[i+at] != MEM_EMPTY)
			{
			haveSpace = false;
			break;
			}
	
	if (haveSpace)
		for (int i=0; i<delta; i++)
			_regSpace[i+at] = reg.offset();
	
	return haveSpace;
	}
	
/*****************************************************************************\
|* Deallocate a register
\*****************************************************************************/
void RegisterFile::free(Register& reg)
	{
	/************************************************************************\
    |* Clear out the map for this register
    \************************************************************************/
	_populate(reg, true);
	
	/************************************************************************\
    |* Remove this register from the register file
    \************************************************************************/
    int idx = 0;
    int found = -1;
    for (Register &r : _allocated)
		{
		if ((reg.page()   == r.page())
		&&  (reg.set()    == r.set())
		&&  (reg.offset() == r.offset()))
			{
			found = idx;
			break;
			}
		idx ++;
		}
	if (found >= 0)
		_allocated.erase(_allocated.begin() + found);
	else
		WARN("Cannot find %s in register file!", reg.toString().c_str());
	}
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Populate the map with the given register
\*****************************************************************************/
void RegisterFile::_populate(Register &r, bool clear)
	{
	int from 	= r.offset();
	int to		= from + (r.type() & 0xFF);
	
	for (int i=from; i<to; i++)
		_regSpace[i] = (clear) ? MEM_EMPTY : r.identifier();
	}

/*****************************************************************************\
|* Find space for a register if we need to
\*****************************************************************************/
int RegisterFile::_findSpace(int bytes)
	{
	int spots	= REG_PAGE_SPACE;
	bool found	= false;
	int offset 	= 0;
	
	for (int i=0; i<spots && (!found); i++)
		{
		switch (bytes)
			{
			case Register::UNSIGNED_1BYTE:	// unsigned doesn't matter
				if (_regSpace[offset] == MEM_EMPTY)
					found 	= true;
				break;
			
			case Register::UNSIGNED_2BYTE:	// unsigned doesn't matter
				if ((_regSpace[offset  ] == MEM_EMPTY) &&
					(_regSpace[offset+1] == MEM_EMPTY))
					found 	= true;
				break;
			
			case Register::UNSIGNED_4BYTE:	// unsigned doesn't matter
				if ((_regSpace[offset  ] == MEM_EMPTY) &&
					(_regSpace[offset+1] == MEM_EMPTY) &&
				    (_regSpace[offset+2] == MEM_EMPTY) &&
				    (_regSpace[offset+3] == MEM_EMPTY))
					found 	= true;
				break;
			
			default:
				FATAL(ERR_REG_ALLOC, "Unknown register type 0x%x", bytes);
			}
			
		if (!found)
			offset ++;
		}
	
	return (found) ? offset : -1;
	}

/*****************************************************************************\
|* Dump out a representation of the register map
\*****************************************************************************/
void RegisterFile::dump(void)
	{
	int idx = 0;
	for (int i=0; i<4; i++)
		{
		for (int j=0; j<16; j++)
			{
			uint32_t val = _regSpace[idx ++];
			if (val == MEM_EMPTY)
				printf(".. ");
			else
				printf("%2d ", val);
			}
		printf("\n");
		}
	printf("\n-----\n");
	for (Register &r : _allocated)
		printf("%s ", r.toString().c_str());
	printf("\n=====\n\n");
	}
