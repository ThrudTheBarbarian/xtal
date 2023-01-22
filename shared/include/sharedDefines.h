//
//  defines.h
//  xtal
//
//  Created by Thrud The Barbarian on 11/28/22.
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
	PT_S8,					// signed int, 8 bits
	PT_U8,					// unsigned char, 8 bits
	PT_S16,					// signed int, 16 bits
	PT_U16,					// unsigned int, 16 bits
	PT_S32,					// signed int, 32 bits
	PT_U32,					// unsigned int, 32 bits
	
	PT_VOIDPTR	= 0x102,	// Pointer to void	.. 258
	PT_S8PTR,				// Pointer to signed 8-bit value
	PT_U8PTR,				// Pointer to unsigned 8-bit value
	PT_S16PTR,				// Pointer to signed 16-bit value
	PT_U16PTR,				// Pointer to unsigned 16-bit value
	PT_S32PTR,				// Pointer to signed 32-bit value
	PT_U32PTR,				// Pointer to unsigned 32-bit value
	
	PT_MAXVAL				// Last entry
	};

// Structural types (function, variable, array)
typedef enum
	{
	ST_NONE		= -1,
	ST_VARIABLE	= 1,
	ST_FUNCTION,
	ST_ARRAY
	} StructuralType;

typedef enum
	{
	C_GLOBAL	= 1,		// Obviously, a global symbol
	C_LOCAL,				// And a local symbol
	C_PARAM 				// A parameter for a function
	} Storage;

#define STACK_PTR			"SP"			// Stack pointer address ($90,$91)


#include "Locator.h"
#define NC_FILE_START		"nc:file-start"
#define NC_LINE_INC			"nc:line-inc"

#endif /* ! sharedDefines_h */
