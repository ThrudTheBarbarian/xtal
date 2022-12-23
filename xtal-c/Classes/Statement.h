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
        static void _match(Scanner& scanner,
						   Token& token,
						   int tokenType,
						   int &line,
						   String info);
    
        /*********************************************************************\
        |* Match a semicolon
        \*********************************************************************/
        void _semicolon(Scanner& scanner, Token& token, int& line);

        /*********************************************************************\
        |* Match an identifier
        \*********************************************************************/
        void _identifier(Scanner& scanner, Token& token, int& line);



        /********************************************************************\
        |* Handle the print statement
        \********************************************************************/
		ASTNode * _print(Token& token, int& line);
		
        /********************************************************************\
        |* Handle a declaration statement
        \********************************************************************/
		void _varDeclaration(Token& token,
							 int& line,
							 int type,
							 bool isLocal,
							 bool isParam);
		
        /********************************************************************\
        |* Handle parameter declarations
        \********************************************************************/
		int _paramDeclaration(Token& token, int& line);

        /********************************************************************\
        |* Process a function declaration
        \********************************************************************/
        ASTNode * _functionDeclaration(Token& token,
									   int& line,
									   int type);
		
        /********************************************************************\
        |* Handle an if statement
        \********************************************************************/
		ASTNode * _if(Token& token, int& line);
		
        /********************************************************************\
        |* Handle a while statement
        \********************************************************************/
		ASTNode * _while(Token& token, int& line);

        /********************************************************************\
        |* Handle a for statement
        \********************************************************************/
		ASTNode * _for(Token& token, int& line);

        /********************************************************************\
        |* Process a single statement
        \********************************************************************/
        ASTNode * _singleStatement(Token& token, int& line);

        /********************************************************************\
        |* Parse a given type from the token
        \********************************************************************/
        int _parseType(Token& token, int& line);


        /********************************************************************\
        |* Determine if a statement needs a ';' after it
        \********************************************************************/
		bool _needsSemicolon(int op);

    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Statement(Scanner &scanner, Emitter *emitter);

        /********************************************************************\
        |* Process the statements we understand
        \********************************************************************/
        ASTNode * compoundStatement(Token& token, int& line);


        /********************************************************************\
        |* Process a global function or variable declaration
        \********************************************************************/
		ASTNode * globalDeclaration(Token& token, int&line);
     
        /********************************************************************\
        |* Process a return statement
        \********************************************************************/
        ASTNode * returnStatement(Token& token, int& line);
     
     
     
        /*********************************************************************\
        |* Match a left bracket '['
        \*********************************************************************/
        static void leftBrace(Scanner& scanner, Token& token, int& line);

        /*********************************************************************\
        |* Match a right bracket ']'
        \*********************************************************************/
        static void rightBrace(Scanner& scanner, Token& token, int& line);

        /*********************************************************************\
        |* Match a left parentheses '('
        \*********************************************************************/
        static void leftParen(Scanner& scanner, Token& token, int& line);

        /*********************************************************************\
        |* Match a right parentheses ')'
        \*********************************************************************/
        static void rightParen(Scanner& scanner, Token& token, int& line);
	};

#endif /* Statement_h */
