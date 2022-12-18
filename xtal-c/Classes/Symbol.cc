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
	   ,_sClass(C_GLOBAL)
	   ,_position(0)
	{
	}

/****************************************************************************\
|* Constructor
\****************************************************************************/
Symbol::Symbol(const String& name,
			   int pType,
			   StructuralType sType,
			   int size,
			   String endLabel)
	   :_name(name)
	   ,_pType(pType)
	   ,_sType(sType)
	   ,_endLabel(endLabel)
	   ,_size(size)
	   ,_sClass(C_GLOBAL)
	   ,_position(0)
	{
	}
	
