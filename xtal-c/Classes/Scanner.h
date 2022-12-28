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
class NotifyCenter;

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
		std::shared_ptr<NotifyCenter>	_nc;
		
		/*********************************************************************\
        |* Get the next character in the input stream
        \*********************************************************************/
        int _next(void);
        
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
        int _skipWhitespace(void);
       
        /*********************************************************************\
        |* Scan an integer, starting with character 'c'
        \*********************************************************************/
        int _scanInteger(int c);
       
        /*********************************************************************\
        |* Scan a character literal
        \*********************************************************************/
        int _scanCharacter(void);
       
        /*********************************************************************\
        |* Scan a string literal
        \*********************************************************************/
        int _scanString(void);
       
        /*********************************************************************\
        |* Scan an identifier/keyword into '_text', starting with character 'c'
        \*********************************************************************/
        int _scanIdentifier(int c);
       
        /*********************************************************************\
        |* Scan a directive, or just random text until the end of a line
        \*********************************************************************/
        int _scanDirective(void);
        
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
        int scan(Token& t);
         
        /*********************************************************************\
        |* Reject a token as no longer needed, store it for later
        \*********************************************************************/
        void reject(Token& t);
	};

#endif /* Scanner_h */
