//
//  SymbolTable.h
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
//

#ifndef SymbolTable_h
#define SymbolTable_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

#include "Symbol.h"

#define SYMTAB					SymbolTable::sharedInstance()->table()

class SymbolTable
	{
    NON_COPYABLE_NOR_MOVEABLE(SymbolTable)
 
	typedef std::vector<Symbol> SymTable;
	
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GET(SymTable, table);			// The actual symbol table
    
    private:

        static std::shared_ptr<SymbolTable> _instance;	// Shared instance

		/*********************************************************************\
        |* Constructor
        \*********************************************************************/
        explicit SymbolTable();
        
    public:

        /********************************************************************\
        |* Find a global variable and return the slot position or -1
        \********************************************************************/
		int find(const String& name);

        /********************************************************************\
        |* Add a global variable, and return the new slot position
        \********************************************************************/
		int add(const String& name);
		
        /**********************************************************************\
        |* This method returns the default global instance.          \**********************************************************************/
        static std::shared_ptr<SymbolTable> sharedInstance();
	};

#endif /* SymbolTable_h */
