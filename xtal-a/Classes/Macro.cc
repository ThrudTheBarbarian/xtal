//
//  Macro.cc
//  xtal-a
//
//  Created by Thrud The Barbarian on 11/6/22.
//

#include "Macro.h"

/****************************************************************************\
|* Constructor
\****************************************************************************/
Macro::Macro()
	  :_name("undefined")
	  ,_args(0)
	{}

/****************************************************************************\
|* Reset the macro so it can be re-defined
\****************************************************************************/
void Macro::reset(void)
	{
	_name = "undefined";
	_lines.clear();
	_args = 0;
	}
