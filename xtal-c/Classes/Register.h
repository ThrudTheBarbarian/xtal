//
//  Register.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#ifndef Register_h
#define Register_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Register
	{
	public:
		typedef enum
			{
			UNSIGNED_1BYTE	= 1,
			UNSIGNED_2BYTE	= 2,
			UNSIGNED_4BYTE	= 4,
			SIGNED_1BYTE	= 0x101,
			SIGNED_2BYTE	= 0x102,
			SIGNED_4BYTE	= 0x104,
			UNKNOWN			= 0x200
			} RegType;
		
		typedef enum
			{
			SET_C0CF	= 0,
			SET_D0DF,
			SET_E0EF,
			SET_F0FF,
			SET_UNKNOWN
			} RegSet;
			
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GETSET(RegType, type, Type);				// Storage space, (un)signed
    GETSET(int, page, Page);					// Which page index we're on
    GETSET(RegSet, set, Set);					// Which set we're in
    GETSET(int, offset, Offset);				// Offset within set
    GETSET(String, name, Name);					// Name of this register
    GETSET(int64_t, value, Value);				// Cope with all int values
    GETSET(uint32_t, identifier, Identifier);	// Register identififer in map
    
    private:
        
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Register();

        /********************************************************************\
        |* return a description of the register
        \********************************************************************/
		String toString(void);

        /********************************************************************\
        |* return the size of the register, 1, 2 or 4
        \********************************************************************/
        int size(void);
        String sizeAsString(void);
        
	};

#endif /* Register_h */
