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
#include "sharedDefines.h"

#include "Symbol.h"

#define SYMTAB					SymbolTable::sharedInstance()

class SymbolTable
	{
    NON_COPYABLE_NOR_MOVEABLE(SymbolTable)
 
	public:
		/********************************************************************\
		|* Returned value if we don't find a symbol
		\********************************************************************/
		static const int NOT_FOUND = -1;
		

		/********************************************************************\
		|* Help out the property definition
		\********************************************************************/
		typedef std::vector<Symbol> SymTable;
				
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GET(SymTable, table);					// The actual symbol table
 	GETSET(int, functionId, FunctionId);	// Which function we're in now
   
    private:

        static std::shared_ptr<SymbolTable> _instance;	// Shared instance

		/*********************************************************************\
        |* Constructor
        \*********************************************************************/
        explicit SymbolTable();
        
    public:

        /********************************************************************\
        |* Find a global variable and return the slot position or NOT_FOUND
        \********************************************************************/
		int find(const String& name);

        /********************************************************************\
        |* Return the symbol for the current function
        \********************************************************************/
		Symbol currentFunction(void);

        /********************************************************************\
        |* Add a global variable, and return the new slot position
        \********************************************************************/
		int add(const String& name, int pType, StructuralType sType);
		
        /**********************************************************************\
        |* This method returns the default global instance.          \**********************************************************************/
        static std::shared_ptr<SymbolTable> sharedInstance();
	};

#endif /* SymbolTable_h */
