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

#pragma mark - Private methods

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
