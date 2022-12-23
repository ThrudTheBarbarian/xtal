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
		
        /*********************************************************************\
        |* Types of symbol
        \*********************************************************************/
		typedef enum
			{
			C_GLOBAL				= 1,	// Obviously, a global symbol
			C_LOCAL,						// And a local symbol
			C_PARAM 						// A parameter for a function
			} Storage;
		
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GETSET(String, name, Name);				// Symbol name
	GETSET(int, pType, PType);				// Primitive type for symbol
	GETSET(StructuralType, sType, SType);	// Structural type for symbol
	GETSET(String, endLabel, EndLabel);		// Label at the end of the function
	GETSET(int, size, Size);				// Number of elements in array
	GETSET(Storage, sClass, SClass);		// Storage class of the symbol
	GETSET(int, position, Position);		// For locals, -ve offset from FP
	GETSET(int, numParams, NumParams);		// For function, # params
	GETSET(int, location, Location);		// For fn-arg, address

    private:
        
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Symbol();
		explicit Symbol(const String& name,
						int pType,
						StructuralType sType,
						int size=1,
						String endLabel = "endFunc"
						);
	};

#endif /* Symbol_h */
