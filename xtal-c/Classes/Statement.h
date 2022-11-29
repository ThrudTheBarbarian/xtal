//
//  Statement.h
//  xtal-c
//
//  Created by Simon Gornall on 11/26/22.
//

#ifndef Statement_h
#define Statement_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class ASTNode;
class Scanner;
class Emitter;
class Token;

class Statement
	{
    NON_COPYABLE_NOR_MOVEABLE(Statement)
 
	/************************************************************************\
    |* Properties
    \************************************************************************/
	
    private:
        Scanner *_scanner;				// Ptr to the compiler's scanner
        Emitter *_emitter;				// Ptr to the compiler's emitter

        /*********************************************************************\
        |* Ensure the current token is 't', and fetch the next token else
        |* throw an error
        \*********************************************************************/
        void _match(Token& token, int tokenType, int &line, String info);
    
        /*********************************************************************\
        |* Match a semicolon
        \*********************************************************************/
        void _semicolon(Token& token, int& line);

        /*********************************************************************\
        |* Match an identifier
        \*********************************************************************/
        void _identifier(Token& token, int& line);

        /*********************************************************************\
        |* Match a left bracket '['
        \*********************************************************************/
        void _lbrace(Token& token, int& line);

        /*********************************************************************\
        |* Match a right bracket ']'
        \*********************************************************************/
        void _rbrace(Token& token, int& line);

        /*********************************************************************\
        |* Match a left parentheses '('
        \*********************************************************************/
        void _lparen(Token& token, int& line);

        /*********************************************************************\
        |* Match a right parentheses ')'
        \*********************************************************************/
        void _rparen(Token& token, int& line);


        /********************************************************************\
        |* Handle the print statement
        \********************************************************************/
		ASTNode * _print(Token& token, int& line);
		
        /********************************************************************\
        |* Handle a declaration statement
        \********************************************************************/
		void _declaration(Token& token, int& line);
		
        /********************************************************************\
        |* Handle an assign statement
        \********************************************************************/
		ASTNode * _assignment(Token& token, int& line);
		
        /********************************************************************\
        |* Handle an if statement
        \********************************************************************/
		ASTNode * _if(Token& token, int& line);
		
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Statement(Scanner &scanner, Emitter *emitter);

        /********************************************************************\
        |* Process a statement
        \********************************************************************/
        ASTNode * compoundStatement(Token& token, int& line);
        
	};

#endif /* Statement_h */
