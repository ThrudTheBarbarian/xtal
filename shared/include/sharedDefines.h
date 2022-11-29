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
	C_COMPARE,
	C_UNKNOWN
	} ContextType;


#endif /* ! sharedDefines_h */
