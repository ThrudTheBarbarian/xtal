//
//  Function.h
//  xtal-a
//
//  Created by Thrud The Barbarian on 11/22/22.
//

#ifndef Function_h
#define Function_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Function
	{
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GETSET(StringList, clobbers, Clobbers);		// which registers are clobbered
    GETSET(String, name, Name);					// Name of the function
    GETSET(StringList, lines, Lines);			// Function content
    GETSET(bool, used, Used);					// Whether this was called
    GETSET(bool, emitted, Emitted);				// Whether we've output this yet
    
    private:
        
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit Function(void);

        /*********************************************************************\
        |* Reset a function to be as-new
        \*********************************************************************/
		void reset(void);

        /*********************************************************************\
        |* Generate a series of assembly instructions to preserve any regs that
        |* this function clobbers
        \*********************************************************************/
		void enstack(StringList &assembly);

        /*********************************************************************\
        |* Generate a series of assembly instructions to preserve any regs that
        |* this function clobbers
        \*********************************************************************/
		void destack(StringList &assembly);
	};

typedef std::map<String, Function> FunctionMap;

#endif /* Function_h */
