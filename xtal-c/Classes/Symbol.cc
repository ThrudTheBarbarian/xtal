//
//  Symbol.cc
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
//

#include "Symbol.h"

/****************************************************************************\
|* Constructor
\****************************************************************************/
Symbol::Symbol()
	   :_name("")
	   ,_pType(PT_NONE)
	   ,_sType(ST_VARIABLE)
	   ,_endLabel("endFunc")
	{
	}

/****************************************************************************\
|* Constructor
\****************************************************************************/
Symbol::Symbol(const String& name,
			   int pType,
			   StructuralType sType,
			   String endLabel)
	   :_name(name)
	   ,_pType(pType)
	   ,_sType(sType)
	   ,_endLabel(endLabel)
	{
	}
