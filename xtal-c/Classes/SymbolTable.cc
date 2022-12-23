//
//  SymbolTable.cc
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
//

#include "Emitter.h"
#include "SymbolTable.h"
#include "Types.h"

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
	
	if ((whereToLook & SEARCH_LOCAL) == SEARCH_LOCAL)
		for (int i=0; i<_locals.size(); i++)
			{
			if (_locals[i].name() == name)
				{
				idx = i + LOCAL_OFFSET;
				break;
				}
			}
			
	if ((whereToLook & SEARCH_GLOBAL) == SEARCH_GLOBAL)
		if (idx == NOT_FOUND)
			{
			for (int i=0; i<_globals.size(); i++)
				{
				if (_globals[i].sClass() == Symbol::C_PARAM)
					continue;
				if (_globals[i].name() == name)
					{
					idx = i;
					break;
					}
				}
		}
	return idx;
	}
	

/****************************************************************************\
|* Return whether a given symbol ID is held
\****************************************************************************/
bool SymbolTable::isValid(int symbolId)
	{
	if (symbolId >= LOCAL_OFFSET)
		return _locals.size() > (symbolId - LOCAL_OFFSET);
	return _globals.size() > symbolId;
	}


/****************************************************************************\
|* Return a symbol based on id
\****************************************************************************/
Symbol& SymbolTable::at(int idx)
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
					 int size,
					 Symbol::Storage storageClass)
	{
	/*************************************************************************\
	|* Sanity check that we haven't already allocated the symbol
	\*************************************************************************/
	int idx = find(name, SEARCH_GLOBAL);
	if (idx != NOT_FOUND)
		return idx;
	
	Symbol s(name, pType, sType, size);
	s.setSClass(storageClass);
	_globals.push_back(s);
	return (int)(_globals.size()-1);
	}

/****************************************************************************\
|* Add a local variable to the symbol table
\****************************************************************************/
int SymbolTable::addLocal(const String& name,
					 int pType,
					 StructuralType sType,
					 bool isParam,
					 int size)
	{
	int symIdx = NOT_FOUND;
	Symbol s(name, pType, sType, size);
	
	/*************************************************************************\
	|* Sanity check that we haven't already allocated the symbol. If so,
	|* return an error
	\*************************************************************************/
	int idx = find(name, SEARCH_LOCAL);
	if (idx != NOT_FOUND)
		return NOT_FOUND;
	
	/*************************************************************************\
	|* If this is a parameter, then also add it to the globals list as a
	|* C_PARAM, to build the function prototype
	\*************************************************************************/
	if (isParam)
		{
		/*********************************************************************\
		|* Make space for it and register it
		\*********************************************************************/
		s.setSClass(Symbol::C_PARAM);
		symIdx = (int)(_locals.size()) + LOCAL_OFFSET;
		
		/*********************************************************************\
		|* Add a global C_PARAM type, to build the function prototype
		\*********************************************************************/
		this->addGlobal(name, pType, sType, size, Symbol::C_PARAM);
		}
	else
		{
		/*********************************************************************\
		|* Make space for it and register it
		\*********************************************************************/
		s.setSClass(Symbol::C_LOCAL);
		symIdx = (int)(_locals.size()) + LOCAL_OFFSET;
		
		/*********************************************************************\
		|* Update the stack position index
		\*********************************************************************/
		int posn = _emitter->genGetLocalOffset(pType, false);
		s.setPosition(posn);
		}
		
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

/****************************************************************************\
|* Free the local symbol table
\****************************************************************************/
void SymbolTable::freeLocalSymbols(void)
	{
	_locals.clear();
	}
