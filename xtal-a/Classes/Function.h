//
//  Function.h
//  xtal-a
//
//  Created by Simon Gornall on 11/22/22.
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
	};

typedef std::map<String, Function> FunctionMap;

#endif /* Function_h */
