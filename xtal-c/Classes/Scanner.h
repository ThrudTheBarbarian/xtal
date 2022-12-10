//
//  Scanner.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#ifndef Scanner_h
#define Scanner_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Token;

class Scanner
	{
    NON_COPYABLE_NOR_MOVEABLE(Scanner)
 
	public:
		enum
			{
			SCAN_COMPLETE = 0,
			SCAN_MORE
			};
 
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GET(String, src);				// The source code for the compiler
    GET(int, at);					// Where we're at in the source code
    GET(String, text);				// Last identifier scanned
    
    private:    
    
        /*********************************************************************\
        |* Get the next character in the input stream
        \*********************************************************************/
        int _next(int &line);
        
		/*********************************************************************\
        |* Are we at the end of 'file'
        \*********************************************************************/
        bool _atEnd(void);
       
        /*********************************************************************\
        |* "put back" a character in the input stream
        \*********************************************************************/
        void _putBack(void);
       
        /*********************************************************************\
        |* Skip whitespace
        \*********************************************************************/
        int _skipWhitespace(int &line);
       
        /*********************************************************************\
        |* Scan an integer, starting with character 'c'
        \*********************************************************************/
        int _scanInteger(int c, int &line);
       
        /*********************************************************************\
        |* Scan a character literal
        \*********************************************************************/
        int _scanCharacter(int &line);
       
        /*********************************************************************\
        |* Scan a string literal
        \*********************************************************************/
        int _scanString(int &line);
       
        /*********************************************************************\
        |* Scan an identifier/keyword into '_text', starting with character 'c'
        \*********************************************************************/
        int _scanIdentifier(int c, int &line);
        
        /*********************************************************************\
        |* Check to see if the value in _text is a keyword, return 0 if false
        |* else return the token identifier
        \*********************************************************************/
        int _keyword(void);
      
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit Scanner(String src);

        /*********************************************************************\
        |* Scan for tokens
        \*********************************************************************/
        int scan(Token &token, int& line);
         
        /*********************************************************************\
        |* Reject a token as no longer needed, store it for later
        \*********************************************************************/
        void reject(Token& t);
	};

#endif /* Scanner_h */
