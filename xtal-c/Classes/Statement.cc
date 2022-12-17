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
	
/****************************************************************************\
|* Process the statements we understand
\****************************************************************************/
ASTNode * Statement::compoundStatement(Token& token, int& line)
	{
	ASTNode *left = nullptr;
	
	// Require a left block-open
	leftBrace(*_scanner, token, line);
	
	forever
		{
		ASTNode *tree = _singleStatement(token, line);
		
        /********************************************************************\
        |* Some statements must be followed by a semicolon
        \********************************************************************/
		if (tree != nullptr)
			if (_needsSemicolon(tree->op()))
				_semicolon(*_scanner, token, line);

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
			rightBrace(*_scanner, token, line);
			return left;
			}
		}
	}

/****************************************************************************\
|* Process a function declaration. Currently void return and arguments
\****************************************************************************/
ASTNode * Statement::_functionDeclaration(Token& token, int& line, int type)
	{
	// the text() accessor gives access to the name of the
	// symbol that has been scanned, and we have the type passed in
	int symIdx = SYMTAB->add(_scanner->text(), type, ST_FUNCTION, 0);
	SYMTAB->setFunctionId(symIdx);

	// Tell the emitter to reserve space for our variable
	//_emitter->genSymbol(symIdx);

	// Parentheses
	leftParen(*_scanner, token, line);
	rightParen(*_scanner, token, line);
	
	// Get the AST for the compound statement
	ASTNode *tree = compoundStatement(token, line);
	
	// If the function type isn't P_VOID, check that
	// the last AST operation in the compound statement
	// was a return statement
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

	
	// Return the AST node representing a function wrapping the body
	return new ASTNode(ASTNode::A_FUNCTION, type, tree, symIdx);
	}


/****************************************************************************\
|* Process a function declaration. Currently void return and arguments
\****************************************************************************/
ASTNode * Statement::returnStatement(Token& token, int& line)
	{
	/*************************************************************************\
    |* Can't return a value from a void function
    \*************************************************************************/
	Symbol sym = SYMTAB->currentFunction();
	if (sym.pType() == PT_VOID)
		FATAL(ERR_PARSE,
			  "Can't return a value from a void function at line %d",
			  line);
	
	/*************************************************************************\
    |* Make sure we have 'return' '('
    \*************************************************************************/
	_match(*_scanner, token, Token::T_RETURN, line, "return");
	leftParen(*_scanner, token, line);
	
	/*************************************************************************\
    |* Parse the following expression
    \*************************************************************************/
	ASTNode *tree 	= Expression::binary(*_emitter, *_scanner, token, line, 0);
	
	/*************************************************************************\
    |* Make sure this is compatible with the function's type
    \*************************************************************************/
    tree = Types::modify(tree, sym.pType(), 0);
	if (tree == nullptr)
		FATAL(ERR_PARSE,
			  "Incompatible function type in call at line %d",
			  line);
		
	/*************************************************************************\
    |* Add on the A_RETURN node
    \*************************************************************************/
	tree = new ASTNode(ASTNode::A_RETURN, PT_NONE, tree, 0);
	
	/*************************************************************************\
    |* Eat the ')'
    \*************************************************************************/
	rightParen(*_scanner, token, line);
	
	return tree;
	}


/****************************************************************************\
|* Global declaration - either function or variable
\****************************************************************************/
ASTNode * Statement::globalDeclaration(Token& token, int&line)
	{
	Register none(Register::NO_REGISTER);
	ASTNode *tree = nullptr;
	
	forever
		{
		// Fetch which type of primitive type we're dealing with
		int type = _parseType(token, line);
		
		// Check we have an identifier
		_identifier(*_scanner, token, line);
		
		if (token.token() == Token::T_LPAREN)
			{
			tree = _functionDeclaration(token, line, type);
			_emitter->emit(tree, none, 0, "");
			}
		else
			_varDeclaration(token, line, type);
		
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
					   int& line,
					   String info)
	{
	if (token.token() == tokenType)
		scanner.scan(token, line);
	else
		FATAL(ERR_PARSE, "%s expected on line %d", info.c_str(), line);
	}

/*****************************************************************************\
|* Static method : Check we're getting a semicolon
\*****************************************************************************/
void Statement::_semicolon(Scanner& scanner, Token& token, int& line)
	{
	_match(scanner, token, Token::T_SEMICOLON, line, ";");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::_identifier(Scanner& scanner,Token& token, int& line)
	{
	_match(scanner, token, Token::T_IDENT, line, "identifier");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::leftBrace(Scanner& scanner, Token& token, int& line)
	{
	_match(scanner, token, Token::T_LBRACE, line, "[");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::rightBrace(Scanner& scanner, Token& token, int& line)
	{
	_match(scanner, token, Token::T_RBRACE, line, "]");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::leftParen(Scanner& scanner, Token& token, int& line)
	{
	_match(scanner, token, Token::T_LPAREN, line, "(");
	}

/*****************************************************************************\
|* Static method : Check we're getting an identifier
\*****************************************************************************/
void Statement::rightParen(Scanner& scanner, Token& token, int& line)
	{
	_match(scanner, token, Token::T_RPAREN, line, ")");
	}


#pragma mark - Private methods : statements

/****************************************************************************\
|* Process a single statement, without the trailing ; so this method can be
|* used in for() statements
\****************************************************************************/
ASTNode * Statement::_singleStatement(Token& token, int& line)
	{
	ASTNode *tree 	= nullptr;
	int type	 	= -1;
	
	switch (token.token())
		{
		case Token::T_PRINT:
			tree = _print(token, line);
			break;

		case Token::T_U8:
		case Token::T_S8:
		case Token::T_U16:
		case Token::T_S16:
		case Token::T_S32:
			// Parse the type
			type = _parseType(token, line);
			
			// Check we have an identifier
			_identifier(*_scanner, token, line);
			
			// Declare the variable
			_varDeclaration(token, line, type);
			tree = nullptr;
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
		
		case Token::T_RETURN:
			return returnStatement(token, line);
			
		default:
			// Catch assignments, for now
			return Expression::binary(*_emitter, *_scanner, token, line, 0);
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
	_match(*_scanner, token, Token::T_PRINT, line, "print");
	
	// Parse the following expression and generate the assembly code
	ASTNode *tree = Expression::binary(*_emitter, *_scanner, token, line, 0);
	
	// Make sure we're type-compatible
	int type = tree->type();
	ASTNode *candidate = Types::modify(tree, tree->type(), 0);
	if (candidate == nullptr)
		{
		type = PT_S32;	// int pointer
		candidate = Types::modify(tree, type, 0);
		}
	if (candidate == nullptr)
		FATAL(ERR_TYPE, "Types incompatible at line %d", line);
	
	// Make a 'print' AST node
	return new ASTNode(ASTNode::A_PRINT, type, tree, 0);
	}


/****************************************************************************\
|* Private Method: process an if statement, including an optional 'else'
\****************************************************************************/
ASTNode * Statement::_if(Token& token, int& line)
	{
	// Looking for an if token followed by a '('
	_match(*_scanner, token, Token::T_IF, line, "if");
	leftParen(*_scanner, token, line);
	
	// Parse the following expression, and the ')' after that. Ensure the
	// tree's operation is a comparison
	ASTNode *condAST = Expression::binary(*_emitter, *_scanner, token, line, 0);
	
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		condAST = new ASTNode(ASTNode::A_TOBOOL, condAST->type(), condAST, 0);

	// Close the parentheses
	rightParen(*_scanner, token, line);
	
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
	return new ASTNode(ASTNode::A_IF, PT_NONE, condAST, trueAST, falseAST, 0);
	}

/****************************************************************************\
|* Private Method: process a while statement
\****************************************************************************/
ASTNode * Statement::_while(Token& token, int& line)
	{
	// Looking for a while token followed by a '('
	_match(*_scanner, token, Token::T_WHILE, line, "while");
	leftParen(*_scanner, token, line);
	
	// Parse the following expression, and the ')' after that. Ensure the
	// tree's operation is a comparison
	ASTNode *condAST = Expression::binary(*_emitter, *_scanner, token, line, 0);
	
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		condAST = new ASTNode(ASTNode::A_TOBOOL, condAST->type(), condAST, 0);
	
	// Close the parentheses
	rightParen(*_scanner, token, line);
	
	// Get the AST for the compound statement
	ASTNode *bodyAST = compoundStatement(token, line);
		
	// Build and return the AST for this entire IF statement
	return new ASTNode(ASTNode::A_WHILE, PT_NONE, condAST, nullptr, bodyAST, 0);
	}

/****************************************************************************\
|* Private Method: process a for statement
\****************************************************************************/
ASTNode * Statement::_for(Token& token, int& line)
	{
	ASTNode *tree = nullptr;
	
	// Looking for a while token followed by a '('
	_match(*_scanner, token, Token::T_FOR, line, "for");
	leftParen(*_scanner, token, line);

	// Get the pre-op statement and the ;
	ASTNode *preOpAST = _singleStatement(token, line);
	_semicolon(*_scanner, token, line);
	
	// Get the condition and the )
	ASTNode *condAST = Expression::binary(*_emitter, *_scanner, token, line, 0);
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		condAST = new ASTNode(ASTNode::A_TOBOOL, condAST->type(), condAST, 0);
	_semicolon(*_scanner, token, line);
	
	// Get the post-op statement and the ;
	ASTNode *postOpAST = _singleStatement(token, line);
	rightParen(*_scanner, token, line);

	// Get the compound statement, which is the body
	ASTNode *bodyAST = compoundStatement(token, line);
	
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

int Statement::_parseType(Token& token, int& line)
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
		
		
		case Token::T_VOID:
			type = PT_VOID;
			break;
		
		default:
			FATAL(ERR_PARSE, "Cannot determing type at line %d\n", line);
		}

	// Scan in one or more further '*' tokens
	// and determine the correct pointer type
	while (1)
		{
		_scanner->scan(token, line);
		if (token.token() != Token::T_STAR)
			break;
		type = Types::pointerTo(type);
		}

	return type;
	}

/****************************************************************************\
|* Private Method: Parse the declaration of a scalar variable or an array
|* with a given size. The identifier has been scanned & we have the type
|* Scanner::text() now has the identifier's name. If all that us ok, add it
|* as a known identifier
\****************************************************************************/
void Statement::_varDeclaration(Token& token, int& line, int type)
	{
	String name = _scanner->text();
	
	// If the next token is a '['
	if (token.token() == Token::T_LBRACE)
		{
		// Skip past the '['
		_scanner->scan(token, line);

		// Check we have an array size
		if (token.token() == Token::T_INTLIT)
			{
			// Add this as a known array and generate its space in assembly.
			// We treat the array as a pointer to its elements' type
			int symIdx = SYMTAB->add(name,
									 Types::pointerTo(type),
									 ST_ARRAY,
									 token.intValue());
			
			// Tell the emitter to reserve space for our variable
			_emitter->genSymbol(symIdx);
			}

		// Ensure we have a following ']'
		_scanner->scan(token, line);
		_match(*_scanner, token, Token::T_RBRACE, line, "]");
		}
	else
		{
		// Add this as a known scalar and generate its space in assembly
		int symIdx = SYMTAB->add(name, type, ST_VARIABLE, 1);
		_emitter->genSymbol(symIdx);
		}

	// Get the trailing semicolon
  	_semicolon(*_scanner, token, line);
	}
	
