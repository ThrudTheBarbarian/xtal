//
//  Expression.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include "ASTNode.h"
#include "Expression.h"
#include "Scanner.h"
#include "Token.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Expression::Expression()
	{
	}

/*****************************************************************************\
|* Convert a token identifier to an ASTNode identifier
\*****************************************************************************/
int Expression::tokenToAst(int token, int line)
	{
	int astType = ASTNode::A_NONE;
	
	switch (token)
		{
		case Token::T_PLUS:
			astType = ASTNode::A_ADD;
			break;
		case Token::T_MINUS:
			astType = ASTNode::A_SUBTRACT;
			break;
		case Token::T_STAR:
			astType = ASTNode::A_MULTIPLY;
			break;
		case Token::T_SLASH:
			astType = ASTNode::A_DIVIDE;
			break;
		default:
			FATAL(ERR_AST_UNKNOWN_TOKEN,
					"Unknown token [%d] on line %d", token, line);
		}
	return astType;
	}
	

/*****************************************************************************\
|* Primary expression resolution
\*****************************************************************************/
ASTNode * Expression::primary(Scanner &scanner,
								  Token &token,
								  int &line)
	{
	ASTNode *node;
	
	switch (token.token())
		{
		case Token::T_INTLIT:
			node = new ASTNode(ASTNode::A_INTLIT, token.intValue());
			scanner.scan(token, line);
			return node;
		
		default:
			FATAL(ERR_AST_SYNTAX, "Syntax error on line %d", line);
		}
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
    |* If we hit a semicolon, just return the left node
    \*************************************************************************/
    int tokenType = token.token();
	if (tokenType == Token::T_SEMICOLON)
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
		left = new ASTNode(tokenToAst(tokenType, line), left, right, 0);

		/*********************************************************************\
		|* Update the details of the current token, if we hit a semicolon,
		|* return just the left node
		\*********************************************************************/
		tokenType = token.token();
		if (tokenType == Token::T_SEMICOLON)
			return left;
		}
	/*************************************************************************\
    |* Return the tree we have when the precedence is lower
    \*************************************************************************/
	return left;
	}
