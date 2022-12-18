//
//  SymbolTable.cc
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
//

#include "Emitter.h"
#include "SymbolTable.h"

/******************************************************************************\
|* The global instance
\******************************************************************************/
std::shared_ptr<SymbolTable> SymbolTable::_instance = nullptr;


/****************************************************************************\
|* Constructor
\****************************************************************************/
SymbolTable::SymbolTable()
	{
	}

/******************************************************************************\
|* Return the shared instance
\******************************************************************************/
std::shared_ptr<SymbolTable> SymbolTable::sharedInstance(void)
    {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (!_instance)
        _instance = std::shared_ptr<SymbolTable>(new SymbolTable);

    return _instance;
    }


/****************************************************************************\
|* Find a local or global variable and return the slot position or -1
\****************************************************************************/
int SymbolTable::find(const String& name, int whereToLook)
	{
	int idx = NOT_FOUND;
	
	if ((whereToLook & SEARCH_LOCAL) == SEARCH_GLOBAL)
		for (int i=0; i<_locals.size(); i++)
			if (_locals[i].name() == name)
				{
				idx = i + LOCAL_OFFSET;
				break;
				}
				
	if ((whereToLook & SEARCH_GLOBAL) == SEARCH_LOCAL)
		if (idx == NOT_FOUND)
			{
			for (int i=0; i<_globals.size(); i++)
				if (_globals[i].name() == name)
					{
					idx = i;
					break;
					}
		}
	return idx;
	}
	

/****************************************************************************\
|* Return a symbol based on id
\****************************************************************************/
Symbol SymbolTable::at(int idx)
	{
	if (idx >= LOCAL_OFFSET)
		return _locals.at(idx - LOCAL_OFFSET);
	else if (idx >= 0)
		return _globals.at(idx);
	FATAL(ERR_TYPE, "Attempt to find non-existent symbol");
	}
	
/****************************************************************************\
|* Add a global variable to the symbol table
\****************************************************************************/
int SymbolTable::addGlobal(const String& name,
					 int pType,
					 StructuralType sType,
					 int size)
	{
	/*************************************************************************\
	|* Sanity check that we haven't already allocated the symbol
	\*************************************************************************/
	int idx = find(name, SEARCH_GLOBAL);
	if (idx != NOT_FOUND)
		return idx;
	
	Symbol s(name, pType, sType, size);
	s.setSClass(Symbol::C_GLOBAL);
	_globals.push_back(s);
	return (int)(_globals.size()-1);
	}

/****************************************************************************\
|* Add a local variable to the symbol table
\****************************************************************************/
int SymbolTable::addLocal(const String& name,
					 int pType,
					 StructuralType sType,
					 int size)
	{
	/*************************************************************************\
	|* Sanity check that we haven't already allocated the symbol
	\*************************************************************************/
	int idx = find(name, SEARCH_LOCAL);
	if (idx != NOT_FOUND)
		return idx;
	
	/*************************************************************************\
	|* Make space for it and register it
	\*************************************************************************/
	Symbol s(name, pType, sType, size);
	s.setSClass(Symbol::C_LOCAL);
	int symIdx = (int)(_locals.size()) + LOCAL_OFFSET;
	
	/*************************************************************************\
	|* Update the stack position index
	\*************************************************************************/
	int posn = _emitter->genGetLocalOffset(pType, false /* FIXME: for now */);
	s.setPosition(posn);
	
	_locals.push_back(s);
	return symIdx;
	}

/****************************************************************************\
|* Add a global variable to the symbol table
\****************************************************************************/
Symbol SymbolTable::currentFunction(void)
	{
	if ((_functionId >= 0) && (_functionId < _globals.size()))
		return _globals[_functionId];
	FATAL(ERR_FUNCTION, "Attempt to find non-existent function");
	}
