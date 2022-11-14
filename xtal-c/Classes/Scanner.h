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
        
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit Scanner(String src);

        /*********************************************************************\
        |* Scan for tokens
        \*********************************************************************/
        int scan(Token &token, int& line);
	};

#endif /* Scanner_h */
