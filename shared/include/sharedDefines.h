//
//  defines.h
//  xtal
//
//  Created by Simon Gornall on 11/28/22.
//

#ifndef sharedDefines_h
#define sharedDefines_h

/*****************************************************************************\
|* Context types
\*****************************************************************************/
typedef enum
	{
	C_FILE	= 0,
	C_MACRO,
	C_FUNCTION,
	C_CLASS,
	C_METHOD,
	C_BLOCK,
	C_IF,
	C_WHILE,
	C_COMPARE,
	C_UNKNOWN
	} ContextType;

// Primitive data types
enum
	{
	PT_OK		= 0,		// Used as a compatibility flag
	
	PT_NONE 	= 1,		// This AST node doesn't have a type
	PT_VOID,				// Void data type
	PT_S32,					// signed int, 32 bits
	PT_U32,					// unsigned int, 32 bits
	PT_S16,					// signed int, 16 bits
	PT_U16,					// unsigned int, 16 bits
	PT_S8,					// signed int, 8 bits
	PT_U8,					// unsigned char, 8 bits
	
	P_MAXVAL				// Last entry
	};

// Structural types (function or variable)
typedef enum
	{
	ST_VARIABLE	= 1,
	ST_FUNCTION
	} StructuralType;

#endif /* ! sharedDefines_h */
