//
//  Statement.cc
//  xtal-c
//
//  Created by Thrud The Barbarian on 11/26/22.
//

#include "ASTNode.h"
#include "Emitter.h"
#include "Expression.h"
#include "RegisterFile.h"
#include "Scanner.h"
#include "Statement.h"
#include "SymbolTable.h"
#include "Token.h"
#include "Types.h"

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
ASTNode * Statement::compoundStatement(Token& token)
	{
	ASTNode *left = nullptr;
	
	// Require a left block-open
	leftBrace(*_scanner, token);
	
	forever
		{
		ASTNode *tree = _singleStatement(token);
		
        /********************************************************************\
        |* Some statements must be followed by a semicolon
        \********************************************************************/
		if (tree != nullptr)
			if (_needsSemicolon(tree->op()))
				_semicolon(*_scanner, token);

        /********************************************************************\
        |* For each new tree, either save it in 'left' if left is empty, or
        |* glue the left and new tree together
        \********************************************************************/
		if (tree)
			{
			if (left == nullptr)
				left = tree;
			else
				left = new ASTNode(ASTNode::A_GLUE,
								   PT_NONE,
								   left,
								   nullptr,
								   tree,
								   0);
			}
			
        /********************************************************************\
        |* When we hit a right bracket, skip past it and return the AST
        \********************************************************************/
		if (token.token() == Token::T_RBRACE)
			{
			rightBrace(*_scanner, token);
			return left;
			}
		}
	}


/****************************************************************************\
|* Process a function declaration. Currently void return and arguments
\****************************************************************************/
ASTNode * Statement::returnStatement(Token& token)
	{
	/*************************************************************************\
    |* Can't return a value from a void function
    \*************************************************************************/
	Symbol sym = SYMTAB->currentFunction();
	if (sym.pType() == PT_VOID)
		FATAL(ERR_PARSE, "Can't return a value from a void function");
	
	/*************************************************************************\
    |* Make sure we have 'return' '('
    \*************************************************************************/
	_match(*_scanner, token, Token::T_RETURN, "return");
	leftParen(*_scanner, token);
	
	/*************************************************************************\
    |* Parse the following expression
    \*************************************************************************/
	ASTNode *tree 	= Expression::binary(*_emitter, *_scanner, token, 0);
	
	/*************************************************************************\
    |* Make sure this is compatible with the function's type
    \*************************************************************************/
    tree = Types::modify(tree, sym.pType(), 0);
	if (tree == nullptr)
		FATAL(ERR_PARSE, "Incompatible function type in call");
		
	/*************************************************************************\
    |* Add on the A_RETURN node
    \*************************************************************************/
	tree = new ASTNode(ASTNode::A_RETURN, PT_NONE, tree, 0);
	
	/*************************************************************************\
    |* Eat the ')'
    \*************************************************************************/
	rightParen(*_scanner, token);
	
	return tree;
	}


/****************************************************************************\
|* Global declaration - either function or variable
\****************************************************************************/
ASTNode * Statement::globalDeclaration(Token& token)
	{
	Register none(Register::NO_REGISTER);
	ASTNode *tree = nullptr;
	
	forever
		{
		// Fetch which type of primitive type we're dealing with
		int type = _parseType(token);
		
		// Check we have an identifier
		_identifier(*_scanner, token);
		
		if (token.token() == Token::T_LPAREN)
			{
			tree = _functionDeclaration(token, type);
			
			// Check for a function prototype
			if (tree == nullptr)
				continue;
			_emitter->emit(tree, none, 0, "");
			}
		else
			{
			// Parse a global variable and skip past the semicolon
			_varDeclaration(token, type, C_GLOBAL);
			_semicolon(*_scanner, token);
			}
			
		if (token.token() == Token::T_NONE)
			break;
		}
	
	return tree;
	}


#pragma mark - static methods : matching

/*****************************************************************************\
|* Static method : Ensure the current token is 't', and fetch the next token
|* else throw an error
\*****************************************************************************/
void Statement::_match(Scanner& scanner,
					   Token& token,
					   int tokenType,
					   String info)
	{
	if (token.token() == tokenType)
		scanner.scan(token);
	else
		FATAL(ERR_PARSE, "%s expected", info.c_str());
	}

/*****************************************************************************\
|* Static method : Check we're getting a semicolon
\*****************************************************************************/
void Statement::_semicolon(Scanner& scanner, Token& token)
	{
	_match(scanner, token, Token::T_SEMICOLON, ";");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::_identifier(Scanner& scanner,Token& token)
	{
	_match(scanner, token, Token::T_IDENT, "identifier");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::leftBrace(Scanner& scanner, Token& token)
	{
	_match(scanner, token, Token::T_LBRACE, "[");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::rightBrace(Scanner& scanner, Token& token)
	{
	_match(scanner, token, Token::T_RBRACE, "]");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::leftParen(Scanner& scanner, Token& token)
	{
	_match(scanner, token, Token::T_LPAREN, "(");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::rightParen(Scanner& scanner, Token& token)
	{
	_match(scanner, token, Token::T_RPAREN, ")");
	}


#pragma mark - Private methods : statements

/****************************************************************************\
|* Process a single statement, without the trailing ; so this method can be
|* used in for() statements
\****************************************************************************/
ASTNode * Statement::_singleStatement(Token& token)
	{
	ASTNode *tree 	= nullptr;
	int type	 	= -1;
	
	switch (token.token())
		{
		case Token::T_PRINT:
			tree = _print(token);
			break;

		case Token::T_U8:
		case Token::T_S8:
		case Token::T_U16:
		case Token::T_S16:
		case Token::T_S32:
		case Token::T_U32:
			// Parse the type
			type = _parseType(token);
			
			// Check we have an identifier
			_identifier(*_scanner, token);
			
			// Declare the variable then skip over the semicolon
			_varDeclaration(token, type, C_LOCAL);
			_semicolon(*_scanner, token);
			tree = nullptr;
			break;

		case Token::T_IF:
			tree = _if(token);
			break;

		case Token::T_WHILE:
			tree = _while(token);
			break;

		case Token::T_FOR:
			tree = _for(token);
			break;
		
		case Token::T_RETURN:
			return returnStatement(token);
			
		default:
			// Catch assignments, for now
			return Expression::binary(*_emitter, *_scanner, token, 0);
		}
		
	return tree;
	}

/****************************************************************************\
|* Private Method: process a print statement
\****************************************************************************/
ASTNode * Statement::_print(Token& token)
	{
	Register none(Register::NO_REGISTER);
	
	// Match a 'print' as the first token
	_match(*_scanner, token, Token::T_PRINT, "print");
	
	// Parse the following expression and generate the assembly code
	ASTNode *tree = Expression::binary(*_emitter, *_scanner, token, 0);
	
	// Make sure we're type-compatible
	int type = tree->type();
	ASTNode *candidate = Types::modify(tree, tree->type(), 0);
	if (candidate == nullptr)
		{
		type = PT_S32;	// try an int
		candidate = Types::modify(tree, type, 0);
		}
	if (candidate == nullptr)
		FATAL(ERR_TYPE, "Types incompatible");
	
	// Make a 'print' AST node
	return new ASTNode(ASTNode::A_PRINT, type, tree, 0);
	}


/****************************************************************************\
|* Private Method: process an if statement, including an optional 'else'
\****************************************************************************/
ASTNode * Statement::_if(Token& token)
	{
	// Looking for an if token followed by a '('
	_match(*_scanner, token, Token::T_IF, "if");
	leftParen(*_scanner, token);
	
	// Parse the following expression, and the ')' after that. Ensure the
	// tree's operation is a comparison
	ASTNode *condAST = Expression::binary(*_emitter, *_scanner, token, 0);
	
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		condAST = new ASTNode(ASTNode::A_TOBOOL, condAST->type(), condAST, 0);

	// Close the parentheses
	rightParen(*_scanner, token);
	
	// Get the AST for the compound statement
	ASTNode *trueAST = compoundStatement(token);
	
	// If we have an 'else', skip over the 'else' and get the AST for the
	// compound statement
	ASTNode *falseAST = nullptr;
	if (token.token() == Token::T_ELSE)
		{
		_scanner->scan(token);
		falseAST = compoundStatement(token);
		}
	
	// Build and return the AST for this entire IF statement
	return new ASTNode(ASTNode::A_IF, PT_NONE, condAST, trueAST, falseAST, 0);
	}

/****************************************************************************\
|* Private Method: process a while statement
\****************************************************************************/
ASTNode * Statement::_while(Token& token)
	{
	// Looking for a while token followed by a '('
	_match(*_scanner, token, Token::T_WHILE, "while");
	leftParen(*_scanner, token);
	
	// Parse the following expression, and the ')' after that. Ensure the
	// tree's operation is a comparison
	ASTNode *condAST = Expression::binary(*_emitter, *_scanner, token, 0);
	
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		condAST = new ASTNode(ASTNode::A_TOBOOL, condAST->type(), condAST, 0);
	
	// Close the parentheses
	rightParen(*_scanner, token);
	
	// Get the AST for the compound statement
	ASTNode *bodyAST = compoundStatement(token);
		
	// Build and return the AST for this entire IF statement
	return new ASTNode(ASTNode::A_WHILE, PT_NONE, condAST, nullptr, bodyAST, 0);
	}

/****************************************************************************\
|* Private Method: process a for statement
\****************************************************************************/
ASTNode * Statement::_for(Token& token)
	{
	ASTNode *tree = nullptr;
	
	// Looking for a while token followed by a '('
	_match(*_scanner, token, Token::T_FOR, "for");
	leftParen(*_scanner, token);

	// Get the pre-op statement and the ;
	ASTNode *preOpAST = _singleStatement(token);
	_semicolon(*_scanner, token);
	
	// Get the condition and the )
	ASTNode *condAST = Expression::binary(*_emitter, *_scanner, token, 0);
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		condAST = new ASTNode(ASTNode::A_TOBOOL, condAST->type(), condAST, 0);
	_semicolon(*_scanner, token);
	
	// Get the post-op statement and the ;
	ASTNode *postOpAST = _singleStatement(token);
	rightParen(*_scanner, token);

	// Get the compound statement, which is the body
	ASTNode *bodyAST = compoundStatement(token);
	
	// FIXME: For now all 4 sub-trees have to be non-null.
	tree = new ASTNode(ASTNode::A_GLUE,
								PT_NONE,
								bodyAST,
								nullptr,
								postOpAST,
								0);
	
	// Make a WHILE loop with the condition and this new body
	tree = new ASTNode(ASTNode::A_WHILE,
								PT_NONE,
								condAST,
								nullptr,
								tree,
								0);
	
	// And glue the pre-op to the A_WHILE
	return new ASTNode(ASTNode::A_GLUE,
								PT_NONE,
								preOpAST,
								nullptr,
								tree,
								0);
	}

#pragma mark - Private methods : declarations

int Statement::_parseType(Token& token)
	{
	int type = PT_NONE;
	
	switch (token.token())
		{
		case Token::T_U8:
			type = PT_U8;
			break;
		
		case Token::T_S8:
			type = PT_S8;
			break;

		case Token::T_U16:
			type = PT_U16;
			break;
		
		case Token::T_S16:
			type = PT_S16;
			break;
		
		case Token::T_S32:
			type = PT_S32;
			break;
		
		case Token::T_U32:
			type = PT_U32;
			break;
		
		
		case Token::T_VOID:
			type = PT_VOID;
			break;
		
		default:
			FATAL(ERR_PARSE, "Cannot determing type");
		}

	// Scan in one or more further '*' tokens
	// and determine the correct pointer type
	while (1)
		{
		_scanner->scan(token);
		if (token.token() != Token::T_STAR)
			break;
		type = Types::pointerTo(type);
		}

	return type;
	}

/****************************************************************************\
|* Private Method: param_declaration: <null>
|*           | variable_declaration
|*           | variable_declaration ',' param_declaration
|*
|* Parse the parameters in parentheses after the function name.
|* Add them as symbols to the symbol table and return the number
|* of parameters
|*
|* If idx is not -1, there is an existing function prototype, and the function
|* has this symbol slot number.
\****************************************************************************/
int Statement::_paramDeclaration(Token& token, int idx)
	{
	Symbol sym;
	int count 			= 0;
	int paramId 		= idx + 1;
	int origNumParams	= 0;
	
	if (idx != SymbolTable::NOT_FOUND)
		{
		sym 			= SYMTAB->at(idx);
		if (sym.pType() == PT_NONE)
			FATAL(ERR_TYPE, "Unknown function identifier for id %d", idx);
		origNumParams	= sym.numParams();
		}
		
	// Loop until the final right parentheses
	while (token.token() != Token::T_RPAREN)
		{
		// Get the type and identifier and add it to the symbol table
		int type = _parseType(token);
    
		// Match an identifier
		_identifier(*_scanner, token);
		
		// If we have an existing prototype, check each param-type matches
		if (paramId)
			{
			sym = SYMTAB->at(paramId);
			if (sym.pType() == PT_NONE)
				FATAL(ERR_TYPE, "Unknown param for id %d", paramId);
			if (type != sym.pType())
				FATAL(ERR_TYPE, "Type doesn't match at index %d", count +1);
			paramId ++;
			}
		else
			_varDeclaration(token, type, C_PARAM);
    	count ++;

		// Must have a ',' or ')' at this point
		switch (token.token())
			{
			case Token::T_COMMA:
				_scanner->scan(token);
				break;
				
			case Token::T_RPAREN:
				break;
      
			default:
				FATAL(ERR_TOKEN, "Unexpected token in parameters");
			}
		}
	
	// Check the number of parameters matches the prototype, if we have one
	if ((idx != SymbolTable::NOT_FOUND) && (count != origNumParams))
		{
		sym = SYMTAB->at(idx);
		FATAL(ERR_FUNCTION, "Parameter count mismatch for function %s",
							sym.name().c_str());
		}
		
	// Return the count of parameters
	return(count);
	}
	
/****************************************************************************\
|* Private Method: Parse the declaration of a scalar variable or an array
|* with a given size. The identifier has been scanned & we have the type
|* Scanner::text() now has the identifier's name. If all that us ok, add it
|* as a known identifier
\****************************************************************************/
void Statement::_varDeclaration(Token& token, int type, Storage sClass)
	{
	String name = _scanner->text();
	int symIdx	= -1;
	
	// If the next token is a '['
	if (token.token() == Token::T_LBRACE)
		{
		// Skip past the '['
		_scanner->scan(token);

		// Check we have an array size
		if (token.token() == Token::T_INTLIT)
			{
			// Add this as a known array and generate its space in assembly.
			// We treat the array as a pointer to its elements' type
			if (sClass == C_LOCAL)
				{
				/*
				symIdx = SYMTAB->addLocal(name,
										  Types::pointerTo(type),
										  ST_ARRAY,
										  token.intValue());
				*/
				FATAL(ERR_PARSE, "Local arrays are not implemented");
				}
			else
				symIdx = SYMTAB->addGlobal(name,
										  Types::pointerTo(type),
										  ST_ARRAY,
										  token.intValue(),
										  sClass);
			// Tell the emitter to reserve space for our variable
			_emitter->genSymbol(symIdx);
			}

		// Ensure we have a following ']'
		_scanner->scan(token);
		_match(*_scanner, token, Token::T_RBRACE, "]");
		}
	else
		{
		// Add this as a known scalar and generate its space in assembly
		if (sClass == C_LOCAL)
			{
			symIdx = SYMTAB->addLocal(name, type, ST_VARIABLE, 1, sClass);
			if (symIdx < 0)
				FATAL(ERR_PARSE, "Duplicate local variable %s", name.c_str());
			}
		else
			symIdx = SYMTAB->addGlobal(name, type, ST_VARIABLE, 1, sClass);

		_emitter->genSymbol(symIdx);
		}
	}
	

/****************************************************************************\
|* Process a function declaration. Currently void return and arguments
\****************************************************************************/
ASTNode * Statement::_functionDeclaration(Token& token, int type)
	{
	int idx = SYMTAB->find(_scanner->text());

	/************************************************************************\
    |* If we have a symbol, and it's a function, then set the index
    \************************************************************************/
	if (idx != SymbolTable::NOT_FOUND)
		{
		Symbol s = SYMTAB->at(idx);
		if (s.pType() == PT_NONE)
			FATAL(ERR_TYPE, "Unknown function identifier for id %d", idx);
		if (s.sType() != ST_FUNCTION)
			idx = -1;
		}

	/************************************************************************\
    |* If this is a new function, then add it to the symbol table as a global
    \************************************************************************/
	int symIdx = SYMTAB->addGlobal(_scanner->text(),
								   type,
								   ST_FUNCTION,
								   0,
								   C_GLOBAL);
	
	/************************************************************************\
    |* Reset the local frame offset within the stack pointer
    \************************************************************************/
	_emitter->genResetLocals();

	/************************************************************************\
    |* Open the parentheses
    \************************************************************************/
	leftParen(*_scanner, token);
	
	/************************************************************************\
    |* Parse the parameters, and set the expected number if this is a new
    |* function declaration
    \************************************************************************/
	int numParams = _paramDeclaration(token, idx);
	if (idx == -1)
		SYMTAB->at(symIdx).setNumParams(numParams);

	/************************************************************************\
    |* Close the parentheses
    \************************************************************************/
	rightParen(*_scanner, token);

	/************************************************************************\
    |* Check to see if it's just a prototype (ie: next is a ; not a [)
    \************************************************************************/
	if (token.token() == Token::T_SEMICOLON)
		{
		_scanner->scan(token);
		return nullptr;
		}

	/************************************************************************\
    |* So not just a prototype, copy the global params to the local space
    \************************************************************************/
	if (idx == SymbolTable::NOT_FOUND)
		idx = symIdx;
	SYMTAB->copyFuncParams(idx);

	/************************************************************************\
    |* Set the current-function-id to point to this one
    \************************************************************************/
	SYMTAB->setFunctionId(idx);

	/************************************************************************\
    |* Get the AST for the compound statement
    \************************************************************************/
	ASTNode *tree = compoundStatement(token);
	
	/************************************************************************\
    |* If the function type isn't P_VOID, check that the last AST operation
    |* in the compound statement was a return statement
    \************************************************************************/
	if (type != PT_VOID)
		{
		if (tree == nullptr)
			FATAL(ERR_FUNCTION, "No statements in function with non-void type");

		ASTNode *lastStmt = (tree->op() == ASTNode::A_GLUE)
						   ? tree->right()
						   : tree;
		if ((lastStmt == nullptr) || (lastStmt->op() != ASTNode::A_RETURN))
			FATAL(ERR_PARSE, "No return for function with non-void type");
		}

	/************************************************************************\
    |* Return the AST node representing a function wrapping the body
    \************************************************************************/
	return new ASTNode(ASTNode::A_FUNCTION, type, tree, idx);
	}


/****************************************************************************\
|* Determine if a statement needs a ';' after it
\****************************************************************************/
bool Statement::_needsSemicolon(int op)
	{
	switch (op)
		{
		case ASTNode::A_PRINT:
		case ASTNode::A_ASSIGN:
		case ASTNode::A_RETURN:
		case ASTNode::A_FUNCCALL:
			return true;
		default:
			return false;
		}
	}
