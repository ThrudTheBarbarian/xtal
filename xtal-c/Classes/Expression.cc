//
//  Expression.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include "ASTNode.h"
#include "Expression.h"
#include "Scanner.h"
#include "SymbolTable.h"
#include "Token.h"

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
			node = new ASTNode(ASTNode::A_INTLIT, token.intValue());
			break;

		case Token::T_IDENT:
			// Check that this identifier exists
			identifier = SYMTAB->find(scanner.text());
			if (identifier == SymbolTable::NOT_FOUND)
				FATAL(ERR_PARSE,
					"Unknown variable [%s] on line %d",
						scanner.text().c_str(), line);


			// Make a leaf AST node for it
			node = new ASTNode(ASTNode::A_IDENT, identifier);
			break;

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
		|* Join that sub-tree with ours, convert the token into an ASTnode.
		\*********************************************************************/
		left = new ASTNode(isArith(tokenType, line), left, nullptr, right, 0);

		/*********************************************************************\
		|* Update the details of the current token, if we hit a semicolon,
		|* return just the left node
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
