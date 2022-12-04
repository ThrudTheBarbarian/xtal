//
//  Register.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//

#include "sharedDefines.h"
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
|* Constructor
\****************************************************************************/
Register::Register(int identifier)
		 :_type(UNKNOWN)
		 ,_page(0)
		 ,_set(SET_UNKNOWN)
		 ,_offset(0)
		 ,_name("unknown_register")
		 ,_value(0)
		 ,_identifier(identifier)
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

/****************************************************************************\
|* Set the size using a primitive type
\****************************************************************************/
void Register::setPrimitiveType(int ptype)
	{
	switch (ptype)
		{
		case PT_S8:
			_type = SIGNED_1BYTE;
			break;
		case PT_U8:
			_type = UNSIGNED_1BYTE;
			break;
		case PT_S16:
			_type = SIGNED_2BYTE;
			break;
		case PT_U16:
			_type = UNSIGNED_2BYTE;
			break;
		case PT_S32:
			_type = SIGNED_4BYTE;
			break;
		case PT_U32:
			_type = UNSIGNED_4BYTE;
			break;
		default:
			FATAL(ERR_REG_NOEXIST, "Undefined type %d requested for reg %s",
				ptype, _name.c_str());
		}
	}
