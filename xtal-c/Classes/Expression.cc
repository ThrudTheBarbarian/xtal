//
//  Expression.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include "ASTNode.h"
#include "Emitter.h"
#include "Expression.h"
#include "Scanner.h"
#include "Statement.h"
#include "SymbolTable.h"
#include "Token.h"
#include "Types.h"

#define IS_OP(x)	(tree->op() != ASTNode::x)

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Expression::Expression()
	{
	}


/*****************************************************************************\
|* Primary expression resolution
\*****************************************************************************/
ASTNode * Expression::primary(Emitter& emitter,
							  Scanner &scanner,
							  Token &token)
	{
	ASTNode *node;
	int identifier;
	
	switch (token.token())
		{
		case Token::T_INTLIT:
			{
			int v = token.intValue();
			
			// the value < 0, enforce a signed type
			if (v < 0)
				{
				if (v > -129)
					node = new ASTNode(ASTNode::A_INTLIT, PT_S8, v);
				else if (v > -32769)
					node = new ASTNode(ASTNode::A_INTLIT, PT_S16, v);
				else
					node = new ASTNode(ASTNode::A_INTLIT, PT_S32, v);
				}
			else
				{
				if (v < 256)
					node = new ASTNode(ASTNode::A_INTLIT, PT_U8, v);
				else if (v < 65536)
					node = new ASTNode(ASTNode::A_INTLIT, PT_U16, v);
				else
					node = new ASTNode(ASTNode::A_INTLIT, PT_U32, v);
				}
			break;
			}
			
		case Token::T_STRLIT:
			// For a STRLIT token, generate the assembly for it.
			// Then make a leaf AST node for it. id is the string's label.
			identifier =  emitter.genGlobalString(scanner.text());
			node = new ASTNode(ASTNode::A_STRLIT, PT_U8PTR, identifier);
			break;

		case Token::T_IDENT:
			return _postfix(emitter, scanner, token);

		case Token::T_LPAREN:
			// Beginning of a parenthesised expression, skip the '('.
			// Scan in the expression and the right parenthesis
			scanner.scan(token);\
			node = binary(emitter, scanner, token, 0);
			Statement::rightParen(scanner, token);
			return node;

		default:
			FATAL(ERR_AST_SYNTAX, "Syntax error");
		}
		
	scanner.scan(token);
	return node;
	}

/*****************************************************************************\
|* Parse a postfix expression and return an AST node representing it. The
|* identifier is already in scanner.text()
\*****************************************************************************/
ASTNode * Expression::_postfix(Emitter& emitter,
							   Scanner &scanner,
							   Token &token)
	{
	ASTNode *node = nullptr;

	// Scan in the next token to see if we have a postfix expression
	scanner.scan(token);

	// Function call
	if (token.token() == Token::T_LPAREN)
		return (_funcCall(emitter, scanner, token));

	// An array reference
	if (token.token() == Token::T_LBRACE)
		return (_arrayAccess(emitter, scanner, token));

	// A variable. Check that the variable exists.
	int identifier 	= SYMTAB->find(scanner.text());
	if (identifier == SymbolTable::NOT_FOUND)
		FATAL(ERR_PARSE, "Unknown variable [%s]", scanner.text().c_str());
	
	// And that the symbol is actually a variable
	Symbol s = SYMTAB->at(identifier);
	if (s.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown identifier for id %d", identifier);
	if (s.sType() != ST_VARIABLE)
		FATAL(ERR_PARSE, "Symbol %s is not a variable", scanner.text().c_str());

	switch (token.token())
		{
		// Post-increment: skip over the token
		case Token::T_INC:
			scanner.scan(token);
			node = new ASTNode(ASTNode::A_POSTINC, s.pType(), identifier);
			break;

		// Post-decrement: skip over the token
		case Token::T_DEC:
 			scanner.scan(token);
			node = new ASTNode(ASTNode::A_POSTDEC, s.pType(), identifier);
			break;

		// Just a variable reference
		default:
			node = new ASTNode(ASTNode::A_IDENT, s.pType(), identifier);
			break;
		}
		
	return node;
	}

/*****************************************************************************\
|* Helper to determine if it's an arithmetic operation
\*****************************************************************************/
int Expression::_binaryAstOp(int tokentype)
	{
	if (tokentype > Token::T_NONE && tokentype < Token::T_INTLIT)
		return(tokentype);
	FATAL(ERR_AST_SYNTAX, "Syntax error, token %d", tokentype);
	}

/*****************************************************************************\
|* Binary expression resolution. Return an AST tree whose root is a binary
|* operator.
\*****************************************************************************/
ASTNode * Expression::binary(Emitter& emitter,
							 Scanner &scanner,
							 Token &token,
							 int previousPrecedence)
	{
	/*************************************************************************\
    |* Get the next token and parse it recursively as a prefix expression
    \*************************************************************************/
	ASTNode *left = _prefix(emitter, scanner, token);
	
	if (left == nullptr)
		FATAL(ERR_RUNTIME, "Internal compiler error");
		
	/*************************************************************************\
    |* If we hit a semicolon, comma, ] or ), just return the left node
    \*************************************************************************/
    int tokenType = token.token();
	if ((tokenType == Token::T_SEMICOLON) ||
	    (tokenType == Token::T_RPAREN)    ||
	    (tokenType == Token::T_RBRACE)	  ||
	    (tokenType == Token::T_COMMA))
		{
		left->setIsRValue(true);
		return left;
		}
		
	/*************************************************************************\
    |* Handle precedence: while the precedence of this token > that of the
    |* previous precedence, ...
    \*************************************************************************/
	while ((Token::precedence(tokenType) > previousPrecedence) ||
		   (_rightAssoc(tokenType) &&
				(Token::precedence(tokenType) == previousPrecedence)))
		{
		/*********************************************************************\
		|* Convert the token into a node-type and scan the next token
		\*********************************************************************/
		scanner.scan(token);
		
		/*********************************************************************\
		|* Recursively get the right-hand tree based on precedence
		\*********************************************************************/
		ASTNode *right = binary(emitter,
								scanner,
								token,
								Token::precedence(tokenType));

 		/*********************************************************************\
		|* Determine the operation to perform on the subtrees
		\*********************************************************************/
		int ASTop 		= _binaryAstOp(tokenType);

 		/*********************************************************************\
		|* If we're assigning, then make the tree an r-value
		\*********************************************************************/
		if (ASTop == ASTNode::A_ASSIGN)
			{
			/*****************************************************************\
			|* Set the right to be an r-value
			\*****************************************************************/
			right->setIsRValue(true);
			
			/*****************************************************************\
			|* Ensure the right type matches the left
			\*****************************************************************/
			right = Types::modify(right, left->type(), 0);
			if ((right == nullptr) || (left == nullptr))
				FATAL(ERR_RUNTIME, "Internal compiler error");
			
			if (left == nullptr)
				FATAL(ERR_PARSE, "Incompatible expression in assignment");
			
			/*****************************************************************\
			|* Make an assignment AST tree, but switch left and right around
			|* so the right expression will be generated before the left
			\*****************************************************************/
			ASTNode *lTemp = left;
			left = right;
			right = lTemp;
				
			if (left == nullptr)
				FATAL(ERR_RUNTIME, "Internal compiler error");
			}
		else
			{
			/*****************************************************************\
			|* We aren't doing assignment, so make both L and R into r-values
			\*****************************************************************/
			left->setIsRValue(true);
			right->setIsRValue(true);
			
			/*****************************************************************\
			|* Check to make sure that the types are compatible by trying to
			|* modify each tree to the others type
			\*****************************************************************/
			ASTNode *lTemp	= Types::modify(left, right->type(), ASTop);
			ASTNode *rTemp	= Types::modify(right, left->type(), ASTop);
			if ((lTemp == nullptr) && (rTemp == nullptr))
				FATAL(ERR_TYPE, "Incompatible types");
			
			/*****************************************************************\
			|* Update any trees that were widened or scaled
			\*****************************************************************/
			if (lTemp != nullptr)
				left = lTemp;
			if (left == nullptr)
				FATAL(ERR_RUNTIME, "Internal compiler error");
			if (rTemp != nullptr)
				right = rTemp;
			}
			
		/*********************************************************************\
		|* Join that sub-tree with ours, convert the token into an ASTnode.
		\*********************************************************************/
		left = new ASTNode(_binaryAstOp(tokenType),
						   left->type(),
						   left,
						   nullptr,
						   right,
						   0);

		/*********************************************************************\
		|* Update the details of the current token, if we hit a semicolon,
		|* comma, ] or ), return just the left node
		\*********************************************************************/
		tokenType = token.token();
		if ((tokenType == Token::T_SEMICOLON) ||
			(tokenType == Token::T_RPAREN)    ||
			(tokenType == Token::T_RBRACE)	  ||
			(tokenType == Token::T_COMMA))
			{
			left->setIsRValue(true);
			return left;
			}
		}
		
	/*************************************************************************\
    |* Return the tree we have when the precedence is lower
    \*************************************************************************/
	left->setIsRValue(true);
	return left;
	}

/*****************************************************************************\
|* function call resolution. Return an AST tree whose root is a binary
|* operator.
\*****************************************************************************/
ASTNode * Expression::_funcCall(Emitter& emitter,
								Scanner &scanner,
								Token &token)
	{
	/*************************************************************************\
    |* Find the function
    |* FIXME: Check the structural type as well
    \*************************************************************************/
	String funcName = scanner.text();
	int identifier 	= SYMTAB->find(scanner.text());
	if (identifier == SymbolTable::NOT_FOUND)
		FATAL(ERR_PARSE, "Unknown function [%s]", funcName.c_str());
	
	/*************************************************************************\
    |* Eat the left parenthesis
    \*************************************************************************/
    Statement::leftParen(scanner, token);
    
	/*************************************************************************\
    |* Parse the following expressions
    \*************************************************************************/
	ASTNode *tree = _expressionList(emitter, scanner, token);
		
	/*************************************************************************\
    |* Build the function-call AST node, store the function's return type as
    |* this node's type. Also record the function's symbol-id
    \*************************************************************************/
	Symbol s	= SYMTAB->at(identifier);
	if (s.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown function identifier for id %d", identifier);
	
	tree 		= new ASTNode(ASTNode::A_FUNCCALL,
							  s.pType(),
							  tree,
							  identifier);
							  
	/*************************************************************************\
    |* Eat the right parenthesis
    \*************************************************************************/
    Statement::rightParen(scanner, token);
    
	return tree;
	}


/*****************************************************************************\
|* Parse the index into an array and return an AST tree for it
\*****************************************************************************/
ASTNode * Expression::_arrayAccess(Emitter& emitter,
								   Scanner &scanner,
								   Token &token)
	{
	/*************************************************************************\
    |* Check that the identifier has been defined as an array then make a leaf
    |* node for it that points at the base
    \*************************************************************************/
	int identifier 	= SYMTAB->find(scanner.text());
	Symbol s		= SYMTAB->at(identifier);
	if (s.pType() == PT_NONE)
		FATAL(ERR_TYPE, "Unknown array identifier for id %d", identifier);
	
	if ((identifier == SymbolTable::NOT_FOUND) || (s.sType() != ST_ARRAY))
		FATAL(ERR_ARRAY, "Undeclared array '%s'", scanner.text().c_str());
		
	ASTNode *left = new ASTNode(ASTNode::A_ADDR, s.pType(), identifier);
	
	/*************************************************************************\
    |* Eat the left [
    \*************************************************************************/
	scanner.scan(token);
	
	/*************************************************************************\
    |* Parse the following expression
    \*************************************************************************/
	ASTNode *right = binary(emitter, scanner, token, 0);
	
	/*************************************************************************\
    |* Make sure we close the ]
    \*************************************************************************/
    Statement::rightBrace(scanner, token);

	/*************************************************************************\
    |* Make sure this is of integer type
    \*************************************************************************/
	if (!Types::isInt(right->type()))
		FATAL(ERR_ARRAY, "Array index is not of integer type");

	/*************************************************************************\
    |* Scale the index by the size of the element's type
    \*************************************************************************/
	right = Types::modify(right, left->type(), ASTNode::A_ADD);

	/*************************************************************************\
    |* Return an AST tree where the array's base has the offset added to it,
    |* and dereference the element. Still an lvalue at this point
    \*************************************************************************/
    left = new ASTNode(ASTNode::A_ADD,
					   s.pType(),
					   left,
					   nullptr,
					   right,
					   0);
					   
    left = new ASTNode(ASTNode::A_DEREF,
					   Types::valueAt(left->type()),
					   left,
					   0);
  
	return left;
	}
	
/*****************************************************************************\
|* Primary expression resolution
\*****************************************************************************/
bool Expression::_rightAssoc(int tokenType)
	{
	bool isRight = false;
	
	switch (tokenType)
		{
		case Token::T_ASSIGN:
			isRight = true;
			break;
		
		default:
			isRight = false;
			break;
		}
	return isRight;
	}
	
/*****************************************************************************\
|* Parse a prefix expression and return a sub-tree representing it. Used for
|* pointers and de-refs
\*****************************************************************************/
ASTNode * Expression::_prefix(Emitter& emitter,
							  Scanner &scanner,
							  Token &token)
	{
  	ASTNode *tree;
  
	switch (token.token())
		{
		case Token::T_AMPER:
			// Get the next token and parse it recursively as a prefix
			scanner.scan(token);
			tree = _prefix(emitter, scanner, token);

			// Ensure that it's an identifier
			if (tree->op() != ASTNode::A_IDENT)
				FATAL(ERR_PARSE, "& operator must be followed by an identifier");

			// Now change the operator to A_ADDR and the type to
			// a pointer to the original type
			tree->setOp(ASTNode::A_ADDR);
			tree->setType(Types::pointerTo(tree->type()));
			break;
    
		case Token::T_STAR:
			// Get the next token and parse it recursively as a prefix
			scanner.scan(token);
			tree = _prefix(emitter, scanner, token);

			// For now, ensure it's either another deref or an identifier
			if (!(IS_OP(A_IDENT) || IS_OP(A_DEREF)))
				FATAL(ERR_PARSE, "* operator must be followed "
								 "by an identifier or *");

			// Prepend an A_DEREF operation to the tree
			tree = new ASTNode(ASTNode::A_DEREF,
							   Types::valueAt(tree->type()),
							   tree,
							   0);
			break;

		case Token::T_MINUS:
			// Get the next token and parse it recursively as a prefix
			scanner.scan(token);
			tree = _prefix(emitter, scanner, token);


			// Modify the child node if it is an A_INTLIT node
			tree->setIsRValue(true);
			if (tree->op() == ASTNode::A_INTLIT)
				{
				int64_t val = tree->value().intValue;
				ASTNode::Value v;
				v.intValue = - val;
				tree->setValue(v);
									
				switch (tree->type())
					{
					case PT_U8:
						if (val > 127)
							tree = Types::modify(tree, PT_S16, 0);
						break;
					
					case PT_U16:
					case PT_U32:
						if (val > 32767)
							tree = Types::modify(tree, PT_S32, 0);
					}
					
				if (v.intValue < 0)
					switch (tree->type())
						{
						case PT_U8:
							tree->setType(PT_S8);
							break;
						case PT_U16:
							tree->setType(PT_S16);
							break;
						case PT_U32:
							tree->setType(PT_S32);
							break;
						}
				}
			else
				{
				// Prepend a A_NEGATE operation to the tree and
				// make the child an rvalue.
				// Don't do: tree = Types::modify(tree, PT_S32, 0);
				tree = new ASTNode(ASTNode::A_NEGATE, tree->type(), tree, 0);
				}
			break;
    
		case Token::T_INVERT:
			// Get the next token and parse it recursively as a prefix
			scanner.scan(token);
			tree = _prefix(emitter, scanner, token);

			// Prepend a A_INVERT operation to the tree and
			// make the child an rvalue.
			tree->setIsRValue(true);
			tree = new ASTNode(ASTNode::A_INVERT, tree->type(), tree, 0);
			break;
    
		case Token::T_LOGNOT:
			// Get the next token and parse it recursively as a prefix
			scanner.scan(token);
			tree = _prefix(emitter, scanner, token);

			// Prepend a A_LOGNOT operation to the tree and
			// make the child an rvalue.
			tree->setIsRValue(true);
			tree = new ASTNode(ASTNode::A_LOGNOT, tree->type(), tree, 0);
			break;
    
		case Token::T_INC:
			// Get the next token and parse it recursively as a prefix
			scanner.scan(token);
			tree = _prefix(emitter, scanner, token);

			// For now, ensure it's an identifier
			if (tree->op() != ASTNode::A_IDENT)
				FATAL(ERR_PARSE, "++ operator must be followed "
								 "by an identifier");

			// Prepend a A_LOGNOT operation to the tree and
			// make the child an rvalue.
			tree->setIsRValue(true);
			tree = new ASTNode(ASTNode::A_PREINC, tree->type(), tree, 0);
			break;
    
		case Token::T_DEC:
			// Get the next token and parse it recursively as a prefix
			scanner.scan(token);
			tree = _prefix(emitter, scanner, token);

			// For now, ensure it's an identifier
			if (tree->op() != ASTNode::A_IDENT)
				FATAL(ERR_PARSE, "-- operator must be followed "
								 "by an identifier");

			// Prepend a A_LOGNOT operation to the tree and
			// make the child an rvalue.
			tree->setIsRValue(true);
			tree = new ASTNode(ASTNode::A_PREDEC, tree->type(), tree, 0);
			break;

		default:
			tree = primary(emitter, scanner, token);
		}
	return (tree);
	}

/*****************************************************************************\
|* Handle an expression list
|*
|* expression_list: <null>
|*        | expression ',' expression_list
|*         ;
|*
|* Parse a list of zero or more comma-separated expressions and
|* return an AST composed of A_GLUE nodes with the left-hand child
|* being the sub-tree of previous expressions (or NULL) and the right-hand
|* child being the next expression. Each A_GLUE node will have size field
|* set to the number of expressions in the tree at this point. If no
|* expressions are parsed, NULL is returned
\*****************************************************************************/
ASTNode * Expression::_expressionList(Emitter& emitter,
									  Scanner &scanner,
									  Token &token)
	{
	ASTNode *tree 	= nullptr;

	// Loop until the final right parentheses
	std::vector<ASTNode *> nodes;
	while (token.token() != Token::T_RPAREN)
		{
		// Parse the next expression and increment the expression count
    	ASTNode *child = binary(emitter, scanner, token, 0);
		nodes.push_back(child);

		// Must have a ',' or ')' at this point
		switch (token.token())
			{
			case Token::T_COMMA:
				scanner.scan(token);
				break;
				
			case Token::T_RPAREN:
				break;
			
			default:
				FATAL(ERR_AST_SYNTAX,
					  "Unexpected token %d in expression", token.token());
			}
		}

	for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
		{
		tree = new ASTNode(ASTNode::A_GLUE,
						   PT_NONE,
						   tree,
						   nullptr,
						   *it,
						   0);
		}

	// Return the tree of expressions
	return (tree);
	}
	
