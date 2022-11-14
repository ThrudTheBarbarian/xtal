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

class Scanner;
class Token;

class Expression
	{
    NON_COPYABLE_NOR_MOVEABLE(Expression)
 
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    
    private:
        
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit Expression();

        /*********************************************************************\
        |* Convert a Token identifier into an ASTNode identifier
        \*********************************************************************/
        static int tokenToAst(int token, int line);
        
        /*********************************************************************\
        |* Primary expression resolution
        \*********************************************************************/
        static ASTNode * primary(Scanner &scanner,
								   Token &token,
								   int &line);

        /*********************************************************************\
        |* Binary expression resolution using Pratt precedence
        \*********************************************************************/
        static ASTNode * binary(Scanner &scanner,
								  Token &token,
								  int &line,
								  int previousPrecedence);
		
	};

#endif /* Expression_h */
