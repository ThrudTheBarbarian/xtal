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

        /********************************************************************\
        |* Handle the print statement
        \********************************************************************/
		void _print(Token& token, int& line);
		
        /********************************************************************\
        |* Handle a declaration statement
        \********************************************************************/
		void _declaration(Token& token, int& line);
		
        /********************************************************************\
        |* Handle an assign statement
        \********************************************************************/
		void _assignment(Token& token, int& line);
		
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Statement(Scanner &scanner, Emitter *emitter);

        /********************************************************************\
        |* Process a statement
        \********************************************************************/
        void process(Token& token, int& line);
        
	};

#endif /* Statement_h */
