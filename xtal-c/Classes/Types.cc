//
//  Types.cc
//  xtal-c
//
//  Created by Simon Gornall on 11/30/22.
//

#include "Types.h"
#include "ASTNode.h"

static int _sizes[] =
				{
				0,		// PT_OK,
				0,		// PT_NONE
				0,		// PT_VOID
				1,		// PT_S8,
				1,		// PT_S_8
				2,		// PT_S16
				2,		// PT_U16
				4,		// PT_S32
				4,		// PT_U32
				0,		// PT_MAXVAL
				};

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Types::Types()
	{
	}



/*****************************************************************************\
|* Static method -  Given two primitive types, return true if they are
|* compatible, false otherwise.
|*
|* Also return either zero or an A_WIDEN operation if one has to be widened to
|* match the other.
|*
|* If onlyright is true, only widen left to right
\*****************************************************************************/
bool Types::areCompatible(int line, int &left, int &right, bool onlyRight)
	{
	// Same types are compatible
	if (left == right)
		{
		left = right = PT_OK;
		return true;
		}

	// Get the sizes for each type
	int leftSize	= typeSize(left, line);
	int rightSize	= typeSize(right, line);

	// Types with zero-size are not compativle
	if ((leftSize == 0) || (rightSize == 0))
		return false;

	// Widen PT_?8 to PT_?32 as appropriate
	if (leftSize < rightSize)
		{
		left = ASTNode::A_WIDEN;
		right = PT_OK;
		return true;
		}
	else if (rightSize < leftSize)
		{
		if (onlyRight)
			return false;
		left 	= PT_OK;
		right 	= ASTNode::A_WIDEN;
		return true;
		}
	
	// Anything left is the same size, thus compatible
	left = right = PT_OK;
	return true;
	}

/*****************************************************************************\
|* Static method - return a type's size in bytes
\*****************************************************************************/
int Types::typeSize(int type, int line)
	{
	if ((type < PT_VOID) || (type >= PT_MAXVAL))
		{
		if (line > 0)
			{
			FATAL(ERR_TYPE, "Illegal type %d at line %d", type, line);
			}
		else
			{
			FATAL(ERR_TYPE, "Illegal type %d", type);
			}
		}
	return _sizes[type];
	}

/*****************************************************************************\
|* Static method - Given a primitive type, return the type which is a pointer
|*                 to it
\*****************************************************************************/
int Types::pointerTo(int type, int line)
	{
	int ptrType = PT_NONE;
	
	switch (type)
		{
		case PT_VOID:
			ptrType = PT_VOIDPTR;
			break;
    
		case PT_U8:
			ptrType = PT_U8PTR;
			break;
			
		case PT_S8:
			ptrType = PT_S8PTR;
			break;
    
		case PT_U16:
			ptrType = PT_U16PTR;
			break;
			
		case PT_S16:
			ptrType = PT_S16PTR;
			break;

		case PT_U32:
			ptrType = PT_U32PTR;
			break;
			
		case PT_S32:
			ptrType = PT_S32PTR;
			break;
		
		default:
			if (line > 0)
				{
				FATAL(ERR_TYPE, "Illegal type %d for pointer deref at line %d",
					  type, line);
				}
			else
				{
				FATAL(ERR_TYPE, "Illegal type %d for pointer deref", type);
				}
		}
		
	return (ptrType);
	}

/*****************************************************************************\
|* Static method - For a primitive pointer, return the type which it points to
\*****************************************************************************/
int Types::valueAt(int ptrType, int line)
	{
	int type = PT_NONE;
	
	switch (ptrType)
		{
		case PT_VOIDPTR:
			type = PT_VOID;
			break;
    
		case PT_U8PTR:
			type = PT_U8;
			break;
			
		case PT_S8PTR:
			type = PT_S8;
			break;
    
		case PT_U16PTR:
			type = PT_U16;
			break;
			
		case PT_S16PTR:
			type = PT_S16;
			break;

		case PT_U32PTR:
			type = PT_U32;
			break;
			
		case PT_S32PTR:
			type = PT_S32;
			break;
		
		default:
			if (line > 0)
				{
				FATAL(ERR_TYPE, "Illegal type %d for type deref at line %d",
					  ptrType, line);
				}
			else
				{
				FATAL(ERR_TYPE, "Illegal type %d for type deref", ptrType);
				}
		}
		
	return (type);
	}
