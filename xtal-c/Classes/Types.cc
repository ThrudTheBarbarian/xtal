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
|* Static method - determine if a type is an integer type or not
\*****************************************************************************/
bool Types::isInt(int type)
	{
	bool isInt = false;
	
	switch (type)
		{
		case PT_S8:
		case PT_U8:
		case PT_S16:
		case PT_U16:
		case PT_S32:
		case PT_U32:
			isInt = true;
			break;
		}
	return isInt;
	}

/*****************************************************************************\
|* Static method - determine if a type is a pointer type or not
\*****************************************************************************/
bool Types::isPointer(int type)
	{
	bool isPtr = false;
	
	switch (type)
		{
		case PT_S8PTR:
		case PT_U8PTR:
		case PT_S16PTR:
		case PT_U16PTR:
		case PT_S32PTR:
		case PT_U32PTR:
		case PT_VOIDPTR:
			isPtr = true;
			break;
		}
	return isPtr;
	}
	
/*****************************************************************************\
|* Static method - modify a type within the AST tree
\*****************************************************************************/
ASTNode * Types::modify(ASTNode *tree, int rType, int op)
	{
	int lType = tree->type();
	
	// Compare scalar int types
	if (isInt(lType) && isInt(rType))
		{
		// If both types are the same, there's nothing to do
		if (lType == rType)
			return tree;
		
		// Get the sizes for each type
		int lSize = typeSize(lType);
		int rSize = typeSize(rType);
		
		// Is the tree's size too big ?
		if (lSize > rSize)
			return nullptr;
		
		// Widen to the right
		if (rSize > lSize)
			return new ASTNode(ASTNode::A_WIDEN, rType, tree, 0);
		}

	// For pointers on the left...
	if (isPointer(lType))
		{
		// if we have the same type on the right, and not doing a binary
		// operation (op == 0), we're ok
		if ((op == 0) && (lType == rType))
			return tree;
		}
	
	// We can only scale when it's an A_ADD or A_SUBTRACT operation
	if ((op == ASTNode::A_ADD) || (op == ASTNode::A_SUBTRACT))
		{
		// If left is int-type, right is pointer-type, and the size
		// of the original type is >1, scale the left
		if (isInt(lType) && isPointer(rType))
			{
			int rSize = typeSize(valueAt(rType));
			if (rSize > 1)
				return new ASTNode(ASTNode::A_SCALE, rType, tree, rSize);
			}
		}
	
	// If we get here, the types aren't compatible
	return nullptr;
	}
	
/*****************************************************************************\
|* Static method - return a type's size in bytes
\*****************************************************************************/
int Types::typeSize(int type, int line)
	{
	int range = type & 0xFF;
	
	if ((range < PT_VOID) || (range >= PT_MAXVAL))
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
	if (type >= 0x100)
		return 4;
		
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
