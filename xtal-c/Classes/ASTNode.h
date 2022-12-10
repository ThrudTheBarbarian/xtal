//
//  ASTNode.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#ifndef ASTNode_h
#define ASTNode_h

#include <cstdio>
#include <string>

#include "macros.h"
#include "properties.h"
#include "sharedDefines.h"


class ASTNode
	{
	/*************************************************************************\
    |* AST Node tyoes
    \*************************************************************************/
	public:
		enum
			{
			A_NONE 		= -1,
			A_ASSIGN	= 1,	// Assign operation (must = Token::T_ASSIGN)
			
			A_LOGOR,			// Logical OR
			A_LOGAND,			// Logical AND
			A_OR,				// Mumeric OR
			A_XOR,				// Numeric XOR
		
			A_AND,				// Numeric AND

			A_EQ,				// ==
			A_NE,				// !=
			A_LT,				// <
			A_GT,				// >
			A_LE,				// <=
			A_GE,				// >=

			A_LSHIFT,			// <<
			A_RSHIFT,			// >>
			
			A_ADD,				// Add  (align with Token::T_PLUS)
			A_SUBTRACT,			// Subtract
			A_MULTIPLY,			// Multiply
			A_DIVIDE,			// Divide
			
			A_INTLIT,			// Integer literal
			A_STRLIT,			// String literal
			
			A_IDENT,			// Identifier
			
			A_PRINT, 			// Print statement node
			A_GLUE, 			// Weld trees together node
			A_IF,				// If statement node
			A_WHILE,			// While statement node
			A_FUNCTION,			// Function node
			A_WIDEN,			// Widen the type
			
			A_RETURN,			// Function return opcode
			A_FUNCCALL,			// Call-point for a function
			
			A_DEREF, 			// Dereference op
			A_ADDR,				// Address-of op
			A_SCALE,			// Scale the offset by the type size
			
			A_PREINC,			// ++x
			A_PREDEC,			// --x
			A_POSTINC,			// x++
			A_POSTDEC,			// x--
			A_NEGATE,			// !x
			A_INVERT,			// ~x

			A_LOGNOT,			// Logical not
			A_TOBOOL,			// logical truth for expression
			
			A_MAXVAL			// Last normal entry
			};
				
	typedef union
		{
		int		intValue;		// For A_INTLIT, Value of the integer
		int 	identifier;		// For A_IDENT, Symbol slot number
		int		size;			// For A_SCALE, how much to scale by
		} Value;
	
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GETSET(int, op, Op);				// Operation to perform on this tree
    GETSET(ASTNode *, left, Left);		// Left child node
    GETSET(ASTNode *, mid, Mid);		// Middle child node
    GETSET(ASTNode *, right, Right);	// Right child node
    GETSET(Value, value, Value);		// Type-specific value
	GETSET(int, type, Type);			// Primitive type for this node
    GETSET(bool, isRValue, IsRValue);	// Whether this is an r-value node
    
    private:
        
    public:
        /*********************************************************************\
        |* Generic constructor
        \*********************************************************************/
        explicit ASTNode(int op         	= A_NONE,
						 int type			= PT_NONE,
						 ASTNode *left  	= nullptr,
						 ASTNode *mid   	= nullptr,
						 ASTNode *right 	= nullptr,
						 int intValue   	= 0);
		
        /*********************************************************************\
        |* Leaf-node constructor
        \*********************************************************************/
		explicit ASTNode(int op, int type, int intValue);
		
        /*********************************************************************\
        |* Unary-node constructor
        \*********************************************************************/
		explicit ASTNode(int op, int type, ASTNode *left, int value);
		
        /*********************************************************************\
        |* Destructor
        \*********************************************************************/
		~ASTNode(void);
		
        /*********************************************************************\
        |* Interpret the AST node using left->right traversal of the tree
        \*********************************************************************/
		int interpret(void);
		
        /*********************************************************************\
        |* Dump out the tree from this node down
        \*********************************************************************/
		void dump(ASTNode *node = nullptr, int label = 0, int level = 0);
		
        /*********************************************************************\
        |* Return a string representation of the AST node
        \*********************************************************************/
        String toString(void);
	};

#endif /* ASTNode_h */
