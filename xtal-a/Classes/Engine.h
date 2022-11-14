//
//  Engine.h
//  as8
//
//  Created by Thrud The Barbarian on 10/29/22.
//

#ifndef Engine_h
#define Engine_h

#include <cstdio>
#include <string>
#include <iostream>
#include <cctype>
#include <sstream>
#include <map>
#include <cmath>

#include "properties.h"
#include "macros.h"
#include "structures.h"


class Engine
	{
	/************************************************************************\
    |* Definitions
    \************************************************************************/
    public:
		typedef struct
			{
			String *name;			// Symbol name
			int type;				// VAR, UNDEF
			int64_t value;			// Constant value
			} Symbol;
		
		typedef std::map<String, Symbol> 	Symbols;
		typedef std::map<String, int>		LabelMap;

	/************************************************************************\
    |* Properties
    \************************************************************************/
    GET(Symbols, symbols);					// Current set of symbols
    GET(String, src);						// Current string source
    GET(int, at);							// Current position within string
    GETSET(int64_t, result, Result);		// Result of a computation
    GETSET(int, varType, VarType);			// Value of a VAR symbol
	GETSET(LabelMap, labelMap, LabelMap);	// Map of per-stem next-label ids
	
    private:
    
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Engine();
        
        /********************************************************************\
        |* Get the next character
        \********************************************************************/
        int _next(void);
        
        /********************************************************************\
        |* Are we at the end of the string
        \********************************************************************/
        bool _atEnd(int delta = 0);
        
        /********************************************************************\
        |* Put a character back if we went one-too-far
        \********************************************************************/
        void _putBack(void);
        
        /********************************************************************\
        |* Peek ahead and see what's coming
        \********************************************************************/
        int _peek(int delta = 0);
        
    public:
        /********************************************************************\
        |* Return an instance of the engine
        \********************************************************************/
        static Engine& getInstance()
			{
            static Engine    instance; 	// Guaranteed to be destroyed.
										// Instantiated on first use.
            return instance;
			}
			
        /********************************************************************\
        |* Evaluate an expression
        \********************************************************************/
		bool eval(String s);
			
        /********************************************************************\
        |* Emulate getc() for the string domain
        \********************************************************************/
        int getc(void);
			
        /********************************************************************\
        |* Emulate eof() for the string domain
        \********************************************************************/
        bool eof(void);
			
        /********************************************************************\
        |* Emulate unget() for the string domain
        \********************************************************************/
        void unget(void);
			
        /********************************************************************\
        |* parse and return a number
        \********************************************************************/
        int64_t parseNumber(void);
        
        /********************************************************************\
        |* Dump out named variables
        \********************************************************************/
        void dumpVars(void);
        
        /********************************************************************\
        |* Insert or update a symbol value
        \********************************************************************/
        void updateSymbol(String name, int64_t value);
        
        /********************************************************************\
        |* Return a symbol value if found. Does not create if not
        \********************************************************************/
        bool symbolValue(String name, int64_t &value);
        
        /********************************************************************\
        |* Dynamically look up a symbol
        \********************************************************************/
        int64_t dynamicLookup(String name);
        
        /********************************************************************\
        |* Return the string representing the next label for a given stem
        \********************************************************************/
        String nextLabel(String stem);
        
        /********************************************************************\
        |* Increment the label for a given stem
        \********************************************************************/
        void incLabel(String stem);
        
	};

#endif /* Engine_h */
