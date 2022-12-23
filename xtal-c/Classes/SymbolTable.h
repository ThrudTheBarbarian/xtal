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

class Emitter;
class SymbolTable
	{
    NON_COPYABLE_NOR_MOVEABLE(SymbolTable)
 
	public:
		/********************************************************************\
		|* Returned value if we don't find a symbol
		\********************************************************************/
		static const int NOT_FOUND 		= -1;
		
		/********************************************************************\
		|* Minimum local-symbol index for easy disambiguation
		\********************************************************************/
		static const int LOCAL_OFFSET	= 1000000;

		/********************************************************************\
		|* Help out the property definition
		\********************************************************************/
		typedef std::vector<Symbol> SymTable;

        /*********************************************************************\
        |* Where to look for symbols
        \*********************************************************************/
		enum
			{
			SEARCH_GLOBAL				= 1,
			SEARCH_LOCAL,
			SEARCH_BOTH
			};

	/************************************************************************\
    |* Properties
    \************************************************************************/
    GET(SymTable, globals);					// The globals symbol table
    GET(SymTable, locals);					// The locals symbol table
 	GETSET(int, functionId, FunctionId);	// Which function we're in now
	GETSET(Emitter*, emitter, Emitter);		// Emitter for stack manipulation
	
    private:

        static std::shared_ptr<SymbolTable> _instance;	// Shared instance

		/*********************************************************************\
        |* Constructor
        \*********************************************************************/
        explicit SymbolTable();
        
    public:

        /********************************************************************\
        |* Find a global or local variable and return the slot position or
        |* NOT_FOUND. A local index will have a value > LOCAL_OFFSET
        \********************************************************************/
		int find(const String& name, int where = SEARCH_BOTH);

        /********************************************************************\
        |* Return the symbol for the current function
        \********************************************************************/
		Symbol currentFunction(void);

        /********************************************************************\
        |* Return whether a given symbol ID is held
        \********************************************************************/
		bool isValid(int symbolId);

        /********************************************************************\
        |* Return the symbol for a given index
        \********************************************************************/
		Symbol& at(int idx);

        /********************************************************************\
        |* Free the local symbol table
        \********************************************************************/
		void freeLocalSymbols(void);

        /********************************************************************\
        |* Add a global variable, and return the new slot position
        \********************************************************************/
		int addGlobal(const String& name,
					  int pType,
					  StructuralType sType,
					  int size,
					  Symbol::Storage storageClass = Symbol::C_GLOBAL);

        /********************************************************************\
        |* Add a local variable, and return the new slot position
        \********************************************************************/
		int addLocal(const String& name,
					  int pType,
					  StructuralType sType,
					  bool isParam,
					  int size);

        /**********************************************************************\
        |* This method returns the default global instance.          \**********************************************************************/
        static std::shared_ptr<SymbolTable> sharedInstance();
	};

#endif /* SymbolTable_h */
