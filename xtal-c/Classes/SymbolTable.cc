//
//  SymbolTable.cc
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
//

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
|* Find a global variable and return the slot position or -1
\****************************************************************************/
int SymbolTable::find(const String& name)
	{
	int idx = NOT_FOUND;
	for (int i=0; i<_table.size(); i++)
		if (_table[i].name() == name)
			{
			idx = i;
			break;
			}
	return idx;
	}
	

/****************************************************************************\
|* Add a global variable to the symbol table
\****************************************************************************/
int SymbolTable::add(const String& name,  int pType, StructuralType sType)
	{
	Symbol s(name, pType, sType);
	_table.push_back(s);
	return (int)(_table.size()-1);
	}
