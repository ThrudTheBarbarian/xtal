//
//  Statement.cc
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
//

#include "ASTNode.h"
#include "Emitter.h"
#include "Expression.h"
#include "RegisterFile.h"
#include "Scanner.h"
#include "Statement.h"
#include "SymbolTable.h"
#include "Token.h"

#define IS_OP(x) (tree->op() == ASTNode::x)

/****************************************************************************\
|* Constructor
\****************************************************************************/
Statement::Statement(Scanner &scanner, Emitter *emitter)
		  :_scanner(&scanner)
		  ,_emitter(emitter)
	{
	}


/****************************************************************************\
|* Process the statements we understand
\****************************************************************************/
ASTNode * Statement::compoundStatement(Token& token, int& line)
	{
	ASTNode *left = nullptr;
	
	// Require a left block-open
	_lbrace(token, line);
	
	forever
		{
		ASTNode *tree = _singleStatement(token, line);
		
        /********************************************************************\
        |* Some statements must be followed by a semicolon
        \********************************************************************/
		if (tree != nullptr && (IS_OP(A_PRINT) || IS_OP(A_ASSIGN)))
			_semicolon(token, line);

        /********************************************************************\
        |* For each new tree, either save it in 'left' if left is empty, or
        |* glue the left and new tree together
        \********************************************************************/
		if (tree)
			{
			if (left == nullptr)
				left = tree;
			else
				left = new ASTNode(ASTNode::A_GLUE, left, nullptr, tree, 0);
			}
			
        /********************************************************************\
        |* When we hit a right bracket, skip past it and return the AST
        \********************************************************************/
		if (token.token() == Token::T_RBRACE)
			{
			_rbrace(token, line);
			return left;
			}
		}
	}

/****************************************************************************\
|* Process a function declaration. Currently void return and arguments
\****************************************************************************/
ASTNode * Statement::functionDeclaration(Token& token, int& line)
	{
	// Looking for 'void', the identifier, and the '(' ')'. For now don't
	// do anything with them
	_match(token, Token::T_VOID, line, "void");

	// Check we have an identifier
	_identifier(token, line);

	// Add it to the global symbol table
	int nameSlot = SYMTAB->add(_scanner->text());

	// Parentheses
	_lparen(token, line);
	_rparen(token, line);
	
	// Get the AST for the compound statement
	ASTNode *tree = compoundStatement(token, line);
	
	// Return the AST node representing a function wrapping the body
	return new ASTNode(ASTNode::A_FUNCTION, tree, nameSlot);
	}

#pragma mark - Private methods : matching

/*****************************************************************************\
|* Private method : Ensure the current token is 't', and fetch the next token
|* else throw an error
\*****************************************************************************/
void Statement::_match(Token& token, int tokenType, int& line, String info)
	{
	if (token.token() == tokenType)
		_scanner->scan(token, line);
	else
		FATAL(ERR_PARSE, "%s expected on line %d", info.c_str(), line);
	}

/*****************************************************************************\
|* Private method : Check we're getting a semicolon
\*****************************************************************************/
void Statement::_semicolon(Token& token, int& line)
	{
	_match(token, Token::T_SEMICOLON, line, ";");
	}

/*****************************************************************************\
|* Private method : Check we're getting an identifier
\*****************************************************************************/
void Statement::_identifier(Token& token, int& line)
	{
	_match(token, Token::T_IDENT, line, "identifier");
	}

/*****************************************************************************\
|* Private method : Check we're getting an identifier
\*****************************************************************************/
void Statement::_lbrace(Token& token, int& line)
	{
	_match(token, Token::T_LBRACE, line, "[");
	}

/*****************************************************************************\
|* Private method : Check we're getting an identifier
\*****************************************************************************/
void Statement::_rbrace(Token& token, int& line)
	{
	_match(token, Token::T_RBRACE, line, "]");
	}

/*****************************************************************************\
|* Private method : Check we're getting an identifier
\*****************************************************************************/
void Statement::_lparen(Token& token, int& line)
	{
	_match(token, Token::T_LPAREN, line, "(");
	}

/*****************************************************************************\
|* Private method : Check we're getting an identifier
\*****************************************************************************/
void Statement::_rparen(Token& token, int& line)
	{
	_match(token, Token::T_RPAREN, line, ")");
	}


#pragma mark - Private methods : statements

/****************************************************************************\
|* Process a single statement, without the trailing ; so this method can be
|* used in for() statements
\****************************************************************************/
ASTNode * Statement::_singleStatement(Token& token, int& line)
	{
	ASTNode *tree = nullptr;
	
	switch (token.token())
		{
		case Token::T_PRINT:
			tree = _print(token, line);
			break;

		case Token::T_INT:
			_varDeclaration(token, line);
			tree = nullptr;
			break;

		case Token::T_IDENT:
			tree = _assignment(token, line);
			break;

		case Token::T_IF:
			tree = _if(token, line);
			break;

		case Token::T_WHILE:
			tree = _while(token, line);
			break;

		case Token::T_FOR:
			tree = _for(token, line);
			break;
		
		default:
			FATAL(ERR_PARSE, "Syntax error, token %d", token.token());
		}
		
	return tree;
	}

/****************************************************************************\
|* Private Method: process a print statement
\****************************************************************************/
ASTNode * Statement::_print(Token& token, int& line)
	{
	Register none(Register::NO_REGISTER);
	
	// Match a 'print' as the first token
	_match(token, Token::T_PRINT, line, "print");
	
	// Parse the following expression and generate the assembly code
	ASTNode *node = Expression::binary(*_scanner, token, line, 0);
	ASTNode *tree = new ASTNode(ASTNode::A_PRINT, node, 0);

	return tree;
	}

/****************************************************************************\
|* Private Method: process an assignment statement
\****************************************************************************/
ASTNode * Statement::_assignment(Token& token, int& line)
	{
	Register none(Register::NO_REGISTER);

   	// Ensure we have an identifier
	_identifier(token, line);

	// Check it's been defined then make a leaf node for it
	int idx;
	if ((idx = SYMTAB->find(_scanner->text())) == SymbolTable::NOT_FOUND)
		FATAL(ERR_PARSE, "Undeclared variable '%s' on line %d",
			_scanner->text().c_str(), line);

	ASTNode *right = new ASTNode(ASTNode::A_LVIDENT, idx);

	// Ensure we have an equals sign
	_match(token, Token::T_ASSIGN, line, "=");

	// Parse the following expression
	ASTNode *left = Expression::binary(*_scanner, token, line, 0);

	// Make an assignment AST tree
	ASTNode *tree = new ASTNode(ASTNode::A_ASSIGN, left, nullptr, right, 0);

	return tree;
	}


/****************************************************************************\
|* Private Method: process an if statement, including an optional 'else'
\****************************************************************************/
ASTNode * Statement::_if(Token& token, int& line)
	{
	// Looking for an if token followed by a '('
	_match(token, Token::T_IF, line, "if");
	_lparen(token, line);
	
	// Parse the following expression, and the ')' after that. Ensure the
	// tree's operation is a comparison
	ASTNode *condAST = Expression::binary(*_scanner, token, line, 0);
	
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		FATAL(ERR_AST_SYNTAX, "Bad comparison operator at %d", line);
	
	// Close the parentheses
	_rparen(token, line);
	
	// Get the AST for the compound statement
	ASTNode *trueAST = compoundStatement(token, line);
	
	// If we have an 'else', skip over the 'else' and get the AST for the
	// compound statement
	ASTNode *falseAST = nullptr;
	if (token.token() == Token::T_ELSE)
		{
		_scanner->scan(token, line);
		falseAST = compoundStatement(token, line);
		}
	
	// Build and return the AST for this entire IF statement
	return new ASTNode(ASTNode::A_IF, condAST, trueAST, falseAST, 0);
	}

/****************************************************************************\
|* Private Method: process a while statement
\****************************************************************************/
ASTNode * Statement::_while(Token& token, int& line)
	{
	// Looking for a while token followed by a '('
	_match(token, Token::T_WHILE, line, "while");
	_lparen(token, line);
	
	// Parse the following expression, and the ')' after that. Ensure the
	// tree's operation is a comparison
	ASTNode *condAST = Expression::binary(*_scanner, token, line, 0);
	
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		FATAL(ERR_AST_SYNTAX, "Bad comparison operator at %d", line);
	
	// Close the parentheses
	_rparen(token, line);
	
	// Get the AST for the compound statement
	ASTNode *bodyAST = compoundStatement(token, line);
		
	// Build and return the AST for this entire IF statement
	return new ASTNode(ASTNode::A_WHILE, condAST, nullptr, bodyAST, 0);
	}

/****************************************************************************\
|* Private Method: process a for statement
\****************************************************************************/
ASTNode * Statement::_for(Token& token, int& line)
	{
	ASTNode *tree = nullptr;
	
	// Looking for a while token followed by a '('
	_match(token, Token::T_FOR, line, "for");
	_lparen(token, line);

	// Get the pre-op statement and the ;
	ASTNode *preOpAST = _singleStatement(token, line);
	_semicolon(token, line);
	
	// Get the condition and the )
	ASTNode *condAST = Expression::binary(*_scanner, token, line, 0);
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		FATAL(ERR_AST_SYNTAX, "Bad comparison operator at %d", line);
	_semicolon(token, line);
	
	// Get the post-op statement and the ;
	ASTNode *postOpAST = _singleStatement(token, line);
	_rparen(token, line);

	// Get the compound statement, which is the body
	ASTNode *bodyAST = compoundStatement(token, line);
	
	// FIXME: For now all 4 sub-trees have to be non-null.
	tree = new ASTNode(ASTNode::A_GLUE, bodyAST, nullptr, postOpAST, 0);
	
	// Make a WHILE loop with the condition and this new body
	tree = new ASTNode(ASTNode::A_WHILE, condAST, nullptr, tree, 0);
	
	// And glue the pre-op to the A_WHILE
	return new ASTNode(ASTNode::A_GLUE, preOpAST, nullptr, tree, 0);
	}

#pragma mark - Private methods : declarations

/****************************************************************************\
|* Private Method: process a variable declaration. Ensure we have a declarator
|* token followed by an identifier and a semicolon. Scanner::text() now has
|* the identifier's name. If all that us ok, add it as a known identifier
\****************************************************************************/
void Statement::_varDeclaration(Token& token, int& line)
	{
	// Looking for an integer token
	_match(token, Token::T_INT, line, "s32");

	// Check we have an identifier
	_identifier(token, line);

	// Add it to the global symbol table
	SYMTAB->add(_scanner->text());

	// Tell the emitter to reserve space for our variable
	_emitter->genSymbol(_scanner->text());

	// Match the following semi-colon and stop if we're out of tokens
	_semicolon(token, line);
	}


