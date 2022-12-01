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

#include "macros.h"
#include "properties.h"
#include "sharedDefines.h"

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
    GETSET(String, name, Name);				// Symbol name
	GETSET(int, pType, PType);				// Primitive type for symbol
	GETSET(StructuralType, sType, SType);	// Structural type for symbol
	
    private:
        
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Symbol();
		explicit Symbol(const String& name, int pType, StructuralType sType);
		
        /********************************************************************\
        |* 
        \********************************************************************/
	};

#endif /* Symbol_h */
