//
//  Function.cc
//  xtal-a
//
//  Created by Simon Gornall on 11/22/22.
//

#include "Function.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Function::Function()
	{
	reset();
	}
	
/****************************************************************************\
|* Reset the function so it can be re-defined
\****************************************************************************/
void Function::reset(void)
	{
	_name = "undefined";
	_lines.clear();
	_clobbers.clear();
	_used = false;
	}
