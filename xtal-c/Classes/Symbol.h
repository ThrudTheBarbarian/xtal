//
//  Symbol.h
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
//

#ifndef Symbol_h
#define Symbol_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Symbol
	{
	public:
        /*********************************************************************\
        |* Max length of a symbol in the input
        \*********************************************************************/
		static const int TEXTLEN	= 512;
		
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GETSET(String, name, Name);			// Symbol name
    
    private:
        
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Symbol();
		explicit Symbol(const String& name);
		
        /********************************************************************\
        |* 
        \********************************************************************/
	};

#endif /* Symbol_h */
