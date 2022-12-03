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
			T_PLUS 		= 1,	// Plus op
			T_MINUS,			// Minus op
			T_STAR,				// Multiply op
			T_SLASH,			// Divide op
			
			// Comparisons
			T_EQ,				// Is equal to
			T_NE,				// Is not equal to
			T_LT,				// Is less than
			T_GT,				// Is greater than
			T_LE,				// Is less than or equal to
			T_GE,				// Is greater than or equal to
			
			T_INTLIT,			// Integer literal
			T_SEMICOLON,		// Semicolon
			T_ASSIGN,			// Equals (assign)
			T_IDENT,			// Identifier
			
			T_LBRACE,			// Left block		[
			T_RBRACE,			// Right block		]
			T_LPAREN,			// Left bracket		(
			T_RPAREN,			// Right bracket	)
			
			// Keywords
			T_PRINT,			// Print command
			T_IF,				// Start an IF block
			T_ELSE,				// Start an ELSE block
			T_WHILE,			// Start a WHILE loop
			T_FOR,				// Start a FOR loop

			T_VOID, 			// Void declaration
			T_S32,				// signed 32-bit value
			T_S8,				// signed 8-bit value
			T_U8,				// unsigned 8-bit value
			
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
