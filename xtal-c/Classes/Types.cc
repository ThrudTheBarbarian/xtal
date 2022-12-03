//
//  Types.cc
//  xtal-c
//
//  Created by Simon Gornall on 11/30/22.
//

#include "Types.h"
#include "ASTNode.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Types::Types()
	{
	}
	
/*****************************************************************************\
|* Given two primitive types, return true if they are compatible, false
|* otherwise. Also return either zero or an A_WIDEN operation if one has to
|* be widened to match the other.
|*
|* If onlyright is true, only widen left to right
\*****************************************************************************/
bool Types::areCompatible(int &left, int &right, bool onlyRight)
	{
	// Void is incompatible with everything
	if ((left == PT_VOID) || (right == PT_VOID))
		return false;
	
	// Same types are compatible
	if (left == right)
		{
		left = right = PT_OK;
		return true;
		}
	
	// Widen PT_?8 to PT_?32 as appropriate
	if (((left == PT_U8) || (left == PT_S8)) && (right == PT_S32))
		{
		left = ASTNode::A_WIDEN;
		right = PT_OK;
		return true;
		}
	else if ((left == PT_S32) && ((right == PT_U8) || (right == PT_S8)))
		{
		if (onlyRight)
			return false;
		left 	= PT_OK;
		right 	= ASTNode::A_WIDEN;
		return true;
		}
	
	// Anything left is compatible
	left = right = PT_OK;
	return true;
	}
