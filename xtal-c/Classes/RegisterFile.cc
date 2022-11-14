//
//  RegisterFile.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#include "RegisterFile.h"

#define REG_PAGE_SPACE	(16*4)

static uint32_t _lastReg;						// Last register used
static uint32_t _regSpace[REG_PAGE_SPACE];		// Space to allocate to regs
static std::vector<Register>	_allocated;		// List of current registers


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
	_allocated.clear();
	memset(_regSpace, 0, sizeof(uint32_t)* REG_PAGE_SPACE);
	_lastReg = 1;
	}

/*****************************************************************************\
|* Allocate a register and return the reference
\*****************************************************************************/
Register RegisterFile::allocate(Register::RegType type)
	{
	Register r;
	r.setType(type);

	/************************************************************************\
    |* Look for the first place we can place this reg
    \************************************************************************/
	int offset	= _findSpace(type & 0xFF);
	
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
		r.setOffset(offset % 16);
		
		char buf[1024];
		snprintf(buf, 1024, "r%d", _lastReg);
		r.setName(buf);
		r.setIdentifier(_lastReg);
		_populate(r);
		_lastReg ++;
		}
	else
		FATAL(ERR_REG_ALLOC, "Cannot allocate register");
		
	_allocated.push_back(r);
	return _allocated[_allocated.size()-1];
	}


/*****************************************************************************\
|* Return a register by index
\*****************************************************************************/
Register RegisterFile::registerAt(int idx)
	{
	if ((idx >= 0) && (idx < _allocated.size()))
		return _allocated[idx];
	FATAL(ERR_REG_NOEXIST, "Cannot find register %d", idx);
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
	int offset = (r.set() == Register::SET_C0CF)	? 0
			   : (r.set() == Register::SET_D0DF)	? 16
			   : (r.set() == Register::SET_E0EF)	? 32
			   : 									  48;
			   
	
	int from 	= offset + r.offset();
	int to		= from + (r.type() & 0xFF);
	
	for (int i=from; i<to; i++)
		_regSpace[i] = (clear) ? 0 : r.identifier();
	}

/*****************************************************************************\
|* Find space for a register if we need to
\*****************************************************************************/
int RegisterFile::_findSpace(int bytes)
	{
	int spots	= REG_PAGE_SPACE / bytes;
	bool found	= false;
	int offset 	= 0;
	
	for (int i=0; i<spots && (!found); i++)
		{
		switch (bytes)
			{
			case Register::UNSIGNED_1BYTE:	// unsigned doesn't matter
				if (_regSpace[offset] == 0)
					found 	= true;
				else
					offset ++;
				break;
			
			case Register::UNSIGNED_2BYTE:	// unsigned doesn't matter
				if ((_regSpace[offset] == 0) && (_regSpace[offset+1] == 0))
					found 	= true;
				else
					offset += 2;
				break;
			
			case Register::UNSIGNED_4BYTE:	// unsigned doesn't matter
				if ((_regSpace[offset] == 0)   && (_regSpace[offset+1] == 0)
				 && (_regSpace[offset+2] == 0) && (_regSpace[offset+3] == 0))
					found 	= true;
				else
					offset += 4;
				break;
			
			default:
				FATAL(ERR_REG_ALLOC, "Unknown register type 0x%x", bytes);
			}
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
			printf("%d ", _regSpace[idx++]);
		printf("\n");
		}
	printf("\n-----\n");
	for (Register &r : _allocated)
		printf("%s ", r.toString().c_str());
	printf("\n=====\n\n");
	}
