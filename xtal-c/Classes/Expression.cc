//
//  Expression.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include "ASTNode.h"
#include "Expression.h"
#include "Scanner.h"
#include "Statement.h"
#include "SymbolTable.h"
#include "Token.h"
#include "Types.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Expression::Expression()
	{
	}

/*****************************************************************************\
|* Primary expression resolution
\*****************************************************************************/
ASTNode * Expression::primary(Scanner &scanner, Token &token,  int &line)
	{
	ASTNode *node;
	int identifier;
	
	switch (token.token())
		{
		case Token::T_INTLIT:
			if ((token.intValue() >= 0) && (token.intValue() <= 255))
				node = new ASTNode(ASTNode::A_INTLIT, PT_U8, token.intValue());
			else if ((token.intValue() >= -128) && (token.intValue() <= 127))
				node = new ASTNode(ASTNode::A_INTLIT, PT_S8, token.intValue());
			else
				node = new ASTNode(ASTNode::A_INTLIT, PT_S32, token.intValue());
			break;

		case Token::T_IDENT:
			{
			// This could be a variable or a function, scan in the next
			// token to find out
			scanner.scan(token, line);
			
			// If it's a '(' then we have a function call
			if (token.token() == Token::T_LPAREN)
				return funcCall(scanner, token, line);
			
			// It wasn't, so reject this just-scanned token for next time
			scanner.reject(token);
			
			// Check that this identifier exists
			identifier = SYMTAB->find(scanner.text());
			if (identifier == SymbolTable::NOT_FOUND)
				FATAL(ERR_PARSE,
					"Unknown variable [%s] on line %d",
						scanner.text().c_str(), line);


			// Make a leaf AST node for it
			int pType = SYMTAB->table()[identifier].pType();
			node = new ASTNode(ASTNode::A_IDENT, pType, identifier);
			break;
			}
			
		default:
			FATAL(ERR_AST_SYNTAX, "Syntax error on line %d", line);
		}
		
	scanner.scan(token, line);
	return node;
	}

/*****************************************************************************\
|* Helper to determine if it's an arithmetic operation
\*****************************************************************************/
static int isArith(int tokentype, int line)
	{
	if (tokentype > Token::T_NONE && tokentype < Token::T_INTLIT)
		return(tokentype);
	FATAL(ERR_AST_SYNTAX, "Syntax error on line %d, token %d", line, tokentype);
	}

/*****************************************************************************\
|* Binary expression resolution. Return an AST tree whose root is a binary
|* operator.
\*****************************************************************************/
ASTNode * Expression::binary(Scanner &scanner,
							     Token &token,
								 int &line,
								 int previousPrecedence)
	{
	/*************************************************************************\
    |* Get the integer literal on the left
    \*************************************************************************/
	ASTNode *left = primary(scanner, token, line);
	
	/*************************************************************************\
    |* If we hit a semicolon or ), just return the left node
    \*************************************************************************/
    int tokenType = token.token();
	if ((tokenType == Token::T_SEMICOLON) || (tokenType == Token::T_RPAREN))
		return left;
	
	/*************************************************************************\
    |* Handle precedence: while the precedence of this token > that of the
    |* previous precedence, ...
    \*************************************************************************/
	while (Token::precedence(tokenType) > previousPrecedence)
		{
		/*********************************************************************\
		|* Convert the token into a node-type and scan the next token
		\*********************************************************************/
		scanner.scan(token, line);
		
		/*********************************************************************\
		|* Recursively get the right-hand tree based on precedence
		\*********************************************************************/
		ASTNode *right = binary(scanner,
								  token,
								  line,
								  Token::precedence(tokenType));

 		/*********************************************************************\
		|* Check to make sure that the types are compatible
		\*********************************************************************/
		int leftType 	= left->type();
		int rightType 	= right->type();
		if (! Types::areCompatible(line, leftType, rightType))
			FATAL(ERR_TYPE, "Incompatible types at line %d", line);
		
 		/*********************************************************************\
		|* Widen either side if required. type vars are A_WIDEN|0 now
		\*********************************************************************/
		if (leftType)
			left = new ASTNode(leftType, right->type(), left, 0);
		if (rightType)
			right = new ASTNode(rightType, left->type(), right, 0);

		/*********************************************************************\
		|* Join that sub-tree with ours, convert the token into an ASTnode.
		\*********************************************************************/
		left = new ASTNode(isArith(tokenType, line),
						   left->type(),
						   left,
						   nullptr,
						   right,
						   0);

		/*********************************************************************\
		|* Update the details of the current token, if we hit a semicolon,
		|* or right-parentheses, return just the left node
		\*********************************************************************/
		tokenType = token.token();
		if ((tokenType == Token::T_SEMICOLON) || (tokenType == Token::T_RPAREN))
			return left;
		}
	/*************************************************************************\
    |* Return the tree we have when the precedence is lower
    \*************************************************************************/
	return left;
	}

/*****************************************************************************\
|* function call resolution. Return an AST tree whose root is a binary
|* operator.
\*****************************************************************************/
ASTNode * Expression::funcCall(Scanner &scanner, Token &token, int &line)
	{
	/*************************************************************************\
    |* Find the function
    |* FIXME: Check the structural type as well
    \*************************************************************************/
	String funcName = scanner.text();
	int identifier 	= SYMTAB->find(scanner.text());
	if (identifier == SymbolTable::NOT_FOUND)
		FATAL(ERR_PARSE, "Unknown function [%s] on line %d",
						funcName.c_str(), line);
	
	/*************************************************************************\
    |* Eat the left parenthesis
    \*************************************************************************/
    Statement::leftParen(scanner, token, line);
    
	/*************************************************************************\
    |* Pase the following expression
    \*************************************************************************/
	ASTNode *tree = binary(scanner, token, line, 0);
	
	/*************************************************************************\
    |* Build the function-call AST node, store the function's return type as
    |* this node's type. Also record the function's symbol-id
    \*************************************************************************/
	Symbol s	= SYMTAB->table()[identifier];
	tree 		= new ASTNode(ASTNode::A_FUNCCALL,
							  s.pType(),
							  tree,
							  identifier);
	/*************************************************************************\
    |* Eat the right parenthesis
    \*************************************************************************/
    Statement::rightParen(scanner, token, line);
    
	return tree;
	}
	
