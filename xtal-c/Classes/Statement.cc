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
ASTNode * Statement::functionDeclaration(Token& token, int& line)
	{
	// Looking for the type, the identifier, and the '(' ')'. For now don't
	// do anything with them
	int type = _parseType(token, line);

	// Check we have an identifier
	_identifier(*_scanner, token, line);

	// Add it to the global symbol table
	int nameSlot = SYMTAB->add(_scanner->text(), type, ST_FUNCTION);
	SYMTAB->setFunctionId(nameSlot);
	
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
		ASTNode *lastStmt = (tree->op() == ASTNode::A_GLUE)
						   ? tree->right()
						   : tree;
		if ((lastStmt == nullptr) || (lastStmt->op() != ASTNode::A_RETURN))
			FATAL(ERR_PARSE, "No return for function with non-void type");
		}

	
	// Return the AST node representing a function wrapping the body
	return new ASTNode(ASTNode::A_FUNCTION, PT_VOID, tree, nameSlot);
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
	ASTNode *tree 	= Expression::binary(*_scanner, token, line, 0);
	
	/*************************************************************************\
    |* Make sure this is compatible with the function's type
    \*************************************************************************/
	int returnType 	= tree->type();
	int funcType	= sym.pType();
	if (! Types::areCompatible(line, returnType, funcType, true))
		FATAL(ERR_PARSE,
			  "Incompatible function type in call at line %d",
			  line);
	
	/*************************************************************************\
    |* Widen the left if required
    \*************************************************************************/
	if (returnType == ASTNode::A_WIDEN)
		tree = new ASTNode(returnType, sym.pType(), tree, 0);
	
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
	ASTNode *tree = nullptr;
	
	switch (token.token())
		{
		case Token::T_PRINT:
			tree = _print(token, line);
			break;

		case Token::T_U8:
		case Token::T_S8:
		case Token::T_S32:
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
		
		case Token::T_RETURN:
			return returnStatement(token, line);
			
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
	_match(*_scanner, token, Token::T_PRINT, line, "print");
	
	// Parse the following expression and generate the assembly code
	ASTNode *tree = Expression::binary(*_scanner, token, line, 0);
	
	int leftType 	= PT_S32;
	int rightType	= tree->type();
	if (!Types::areCompatible(line, leftType, rightType))
		FATAL(ERR_TYPE, "Types incompatible at line %d", line);
	
	// Widen the tree if required
	if (rightType)
		tree = new ASTNode(rightType, PT_S32, tree, 0);
	
	// Make a 'print' AST node
	return new ASTNode(ASTNode::A_PRINT, PT_S32, tree, 0);
	}

/****************************************************************************\
|* Private Method: process an assignment statement
\****************************************************************************/
ASTNode * Statement::_assignment(Token& token, int& line)
	{
	Register none(Register::NO_REGISTER);

   	// Ensure we have an identifier
	_identifier(*_scanner, token, line);

	// This could be a variable or a function call. If the next tokens is
	// a '(', its a function call
	if (token.token() == Token::T_LPAREN)
		return Expression::funcCall(*_scanner, token, line);

	// Not a function call, so check it's been defined then make a
	// leaf node for it
	int idx;
	if ((idx = SYMTAB->find(_scanner->text())) == SymbolTable::NOT_FOUND)
		FATAL(ERR_PARSE, "Undeclared variable '%s' on line %d",
			_scanner->text().c_str(), line);

	Symbol sym = SYMTAB->table()[idx];
	ASTNode *right = new ASTNode(ASTNode::A_LVIDENT, sym.pType(), idx);

	// Ensure we have an equals sign
	_match(*_scanner, token, Token::T_ASSIGN, line, "=");

	// Parse the following expression
	ASTNode *left = Expression::binary(*_scanner, token, line, 0);

	// Ensure the types are compatible
	int leftType 	= left->type();
	int rightType	= right->type();
	if (!Types::areCompatible(line, leftType, rightType, true))
		FATAL(ERR_TYPE, "Types incompatible at line %d", line);

	// Widen the left if required
	if (leftType)
		left = new ASTNode(leftType, right->type(), left, 0);

	// Make an assignment AST tree
	// FIXME: Ought this always be PT_S32 ?
	ASTNode *tree = new ASTNode(ASTNode::A_ASSIGN,
								PT_S32,
								left,
								nullptr,
								right,
								0);

	return tree;
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
	ASTNode *condAST = Expression::binary(*_scanner, token, line, 0);
	
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		FATAL(ERR_AST_SYNTAX, "Bad comparison operator at %d", line);
	
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
	ASTNode *condAST = Expression::binary(*_scanner, token, line, 0);
	
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		FATAL(ERR_AST_SYNTAX, "Bad comparison operator at %d", line);
	
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
	ASTNode *condAST = Expression::binary(*_scanner, token, line, 0);
	if ((condAST->op() < ASTNode::A_EQ) || (condAST->op() > ASTNode::A_GE))
		FATAL(ERR_AST_SYNTAX, "Bad comparison operator at %d", line);
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
		case Token::T_S32:
			type = PT_S32;
			break;
		
		case Token::T_U8:
			type = PT_U8;
			break;
		
		case Token::T_S8:
			type = PT_S8;
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
|* Private Method: process a variable declaration. Ensure we have a declarator
|* token followed by an identifier and a semicolon. Scanner::text() now has
|* the identifier's name. If all that us ok, add it as a known identifier
\****************************************************************************/
void Statement::_varDeclaration(Token& token, int& line)
	{
	// Fetch which type of primitive type we're dealing with
	int pType = _parseType(token, line);
	
	// Check we have an identifier
	_identifier(*_scanner, token, line);

	// Add it to the global symbol table
	int symIdx = SYMTAB->add(_scanner->text(), pType, ST_VARIABLE);

	// Tell the emitter to reserve space for our variable
	_emitter->genSymbol(symIdx);

	// Match the following semi-colon and stop if we're out of tokens
	_semicolon(*_scanner, token, line);
	}


