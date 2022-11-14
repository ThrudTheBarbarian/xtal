//
//  Macro.h
//  as8
//
//  Created by Thrud The Barbarian on 11/6/22.
//

#ifndef Macro_h
#define Macro_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Macro
	{ 
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GETSET(String, name, Name);			// Macro name, used to invoke
    GETSET(StringList, lines, Lines);	// Macro content
    GETSET(int, args, Args);			// Number of arguments expected
    
    private:
        
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Macro();

        /********************************************************************\
        |* clear everything out
        \********************************************************************/
		void reset(void);
	};

typedef std::map<String, Macro> MacroMap;

#endif /* Macro_h */
