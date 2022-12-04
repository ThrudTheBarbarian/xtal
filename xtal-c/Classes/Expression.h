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
        |* Primary expression resolution
        \*********************************************************************/
        static ASTNode * primary(Scanner &scanner, Token &token, int &line);

        /*********************************************************************\
        |* Binary expression resolution using Pratt precedence
        \*********************************************************************/
        static ASTNode * binary(Scanner &scanner,
								Token &token,
								int &line,
								int previousPrecedence);
								
        /*********************************************************************\
        |* Prefix resolution - pointers and de-refs
        \*********************************************************************/
        static ASTNode * prefix(Scanner &scanner, Token &token, int &line);

		
        /*********************************************************************\
        |* Function call recognition
        \*********************************************************************/
        static ASTNode * funcCall(Scanner &scanner, Token &token, int &line);
		
	};

#endif /* Expression_h */
