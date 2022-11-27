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

#include "properties.h"
#include "macros.h"

class ASTNode
	{
	/*************************************************************************\
    |* AST Node tyoes
    \*************************************************************************/
	public:
		enum
			{
			A_NONE 		= -1,
			A_ADD,				// Add op
			A_SUBTRACT,			// Subtract op
			A_MULTIPLY,			// Multiply op
			A_DIVIDE,			// Divide op
			A_INTLIT,			// Integer literal
			A_IDENT,			// Identifier
			A_LVIDENT,			// L-value identifier
			A_ASSIGN,			// Assign operation
			
			A_MAXVAL			// Last entry
			};
			
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GETSET(int, op, Op);				// Operation to perform on this tree
    GETSET(ASTNode *, left, Left);		// Left child node
    GETSET(ASTNode *, right, Right);	// Right child node
    GETSET(int, intValue, IntValue);	// For A_INTLIT, Value of the integer
    GETSET(int, identifier, Identifier);// For A_IDENT, Symbol slot number
    
    private:
        
    public:
        /*********************************************************************\
        |* Generic constructor
        \*********************************************************************/
        explicit ASTNode(int op = A_NONE,
						 ASTNode *left = nullptr,
						 ASTNode *right = nullptr,
						 int intValue = 0);
		
        /*********************************************************************\
        |* Leaf-node constructor
        \*********************************************************************/
		explicit ASTNode(int op, int intValue);
		
        /*********************************************************************\
        |* Unary-node constructor
        \*********************************************************************/
		explicit ASTNode(int op, ASTNode *left, int intValue);
		
        /*********************************************************************\
        |* Destructor
        \*********************************************************************/
		~ASTNode(void);
		
        /*********************************************************************\
        |* Interpret the AST node using left->right traversal of the tree
        \*********************************************************************/
		int interpret(void);
		
        /*********************************************************************\
        |* Return a string representation of the AST node
        \*********************************************************************/
        String toString(void);
	};

#endif /* ASTNode_h */
