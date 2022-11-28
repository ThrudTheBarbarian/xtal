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
void Statement::process(Token& token, int& line)
	{
	forever
		{
		switch (token.token())
			{
			case Token::T_PRINT:
				_print(token, line);
				break;
    
			case Token::T_INT:
				_declaration(token, line);
				break;
    
			case Token::T_IDENT:
				_assignment(token, line);
				break;
    
			case Token::T_NONE:
				return;
    
			default:
				FATAL(ERR_PARSE, "Syntax error, token %d", token.token());
			}
		}
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


#pragma mark - Private methods : statement types

/****************************************************************************\
|* Private Method: process a print statement
\****************************************************************************/
void Statement::_print(Token& token, int& line)
	{
	// Match a 'print' as the first token
	_match(token, Token::T_PRINT, line, "print");
	
	// Parse the following expression and generate the assembly code
	ASTNode *node = Expression::binary(*_scanner, token, line, 0);
	Register r = _emitter->emit(node, Register(Register::NO_REGISTER));
	_emitter->printReg(r);
	RegisterFile::clear();
	
	// Match the following semi-colon and stop if we're out of tokens
	_semicolon(token, line);
	}

/****************************************************************************\
|* Private Method: process a print statement
\****************************************************************************/
void Statement::_assignment(Token& token, int& line)
	{
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
	ASTNode *tree = new ASTNode(ASTNode::A_ASSIGN, left, right, 0);

	// Generate the assembly code for the assignment
	Register r = _emitter->emit(tree, Register(Register::NO_REGISTER));
	RegisterFile::clear();

	// Match the following semi-colon and stop if we're out of tokens
	_semicolon(token, line);
	}

#pragma mark - Private methods : declarations

/****************************************************************************\
|* Private Method: process a print statement. Ensure we have a declarator
|* token followed by an identifier and a semicolon. Scanner::text() now has
|* the identifier's name. If all that us ok, add it as a known identifier
\****************************************************************************/
void Statement::_declaration(Token& token, int& line)
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
