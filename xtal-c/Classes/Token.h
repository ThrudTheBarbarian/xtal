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
			T_PLUS,
			T_MINUS,
			T_STAR,
			T_SLASH,
			T_INTLIT,
			T_MAXVAL
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
