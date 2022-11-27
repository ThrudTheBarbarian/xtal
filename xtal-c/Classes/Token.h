//
//  Token.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#ifndef Token_h
#define Token_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Token
	{
	/*************************************************************************\
    |* Types of token. Make sure this syncs up with the code in toString()
    \*************************************************************************/
	public:
		enum
			{
			T_NONE		= -1,
			T_PLUS,				// Plus op
			T_MINUS,			// Minus op
			T_STAR,				// Multiply op
			T_SLASH,			// Divide op
			T_INTLIT,			// Integer literal
			T_SEMICOLON,		// Semicolon
			T_EQUALS,			// Equals (assign)
			T_IDENT,			// Identifier
			
			// Keywords
			T_PRINT,			// Print command
			T_INT,				// Integer declaration

			T_MAXVAL,			// Max token value
			};
		
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GETSET(int, token, Token);				// Token type
    GETSET(int, intValue, IntValue);		// Integer value
    
    private:
        
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
		explicit Token(int token=T_NONE, int intValue=0);
		
        /*********************************************************************\
        |* Return a string representation of the token
        \*********************************************************************/
        String toString(void);

        /*********************************************************************\
        |* Expression precedence (for table driven, Pratt parsing)
        \*********************************************************************/
		int precedence(void);
		static int precedence(int tokenType);
		
	};

#endif /* Token_h */
