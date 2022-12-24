//
//  Expression.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#ifndef Expression_h
#define Expression_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Emitter;
class Scanner;
class Token;

class Expression
	{
    NON_COPYABLE_NOR_MOVEABLE(Expression)
 
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    
    private:
        /*********************************************************************\
        |* Return true is a token is right-associative
        \*********************************************************************/
		static bool _rightAssoc(int tokenType);
		
        /*********************************************************************\
        |* Return true if this is a binary AST op
        \*********************************************************************/
		static int _binaryAstOp(int tokenType, int line);
		
        /*********************************************************************\
        |* Function call recognition
        \*********************************************************************/
        static ASTNode * _funcCall(Emitter& emitter,
								   Scanner &scanner,
								   Token &token,
								   int &line);
		
        /*********************************************************************\
        |* Function call recognition
        \*********************************************************************/
        static ASTNode * _arrayAccess(Emitter& emitter,
									  Scanner &scanner,
									  Token &token,
									  int &line);
								
        /*********************************************************************\
        |* Prefix resolution - pointers and de-refs
        \*********************************************************************/
        static ASTNode * _prefix(Emitter& emitter,
								 Scanner &scanner,
								 Token &token,
								 int &line);
								
        /*********************************************************************\
        |* Prefix resolution - pointers and de-refs
        \*********************************************************************/
        static ASTNode * _postfix(Emitter& emitter,
								  Scanner &scanner,
								  Token &token,
								  int &line);
								
        /*********************************************************************\
        |* Handle an expression list
        \*********************************************************************/
        static ASTNode * _expressionList(Emitter& emitter,
								  Scanner &scanner,
								  Token &token,
								  int &line);
		
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit Expression();

        /*********************************************************************\
        |* Primary expression resolution
        \*********************************************************************/
        static ASTNode * primary(Emitter& emitter,
								 Scanner &scanner,
								 Token &token,
								 int &line);

        /*********************************************************************\
        |* Binary expression resolution using Pratt precedence
        \*********************************************************************/
        static ASTNode * binary(Emitter& emitter,
								Scanner &scanner,
								Token &token,
								int &line,
								int previousPrecedence);

		
	};

#endif /* Expression_h */
