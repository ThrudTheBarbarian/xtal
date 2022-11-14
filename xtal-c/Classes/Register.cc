//
//  Register.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#include "Register.h"

/****************************************************************************\
|* Constructor
\****************************************************************************/
Register::Register()
		 :_type(UNKNOWN)
		 ,_page(0)
		 ,_set(SET_UNKNOWN)
		 ,_offset(0)
		 ,_name("unknown_register")
		 ,_value(0)
		 ,_identifier(0)
	{
	}

/****************************************************************************\
|* Return a string description of the register
\****************************************************************************/
String Register::toString(void)
	{
	String set  = (_set == SET_C0CF) ? "C"
				: (_set == SET_D0DF) ? "D"
				: (_set == SET_E0EF) ? "E" : "F";
				
	return _name+" : {" + std::to_string(_page)+","
			+ set + ","
			+ std::to_string(_type) + ","
			+ std::to_string(_offset)+"}";
			
	}

/****************************************************************************\
|* Return the size of the register: 1, 2 or 4
\****************************************************************************/
int Register::size()
	{
	if (_type == UNKNOWN)
		FATAL(ERR_REG_NOEXIST, "Attempt to parse unknown register size");
	
	return _type & 0xFF;
	}


/****************************************************************************\
|* Return the size of the register: 1, 2 or 4
\****************************************************************************/
String Register::sizeAsString()
	{
	return std::to_string(size());
	}

