//
//  Compiler.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#ifndef Compiler_h
#define Compiler_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class ArgParser;
class Emitter;
class Scanner;
class Statement;
class Token;

class Compiler
	{
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GET(ArgParser*, ap);			// Permanent reference to the arguments
    GET(bool, hadError);			// Whether we've seen a syntax error
    GET(int, line);					// The current line number
    GET(Emitter *, emitter);		// How we produce assembly
    GET(bool, dumpAST);				// Whether to dump the AST tree
    
    private:
    
        /*********************************************************************\
        |* General run() method
        \*********************************************************************/
        int _run(std::string content);
    
        /*********************************************************************\
        |* Handle statements
        \*********************************************************************/
        void _statements(Scanner& scanner, Token &token, int& line);
        
        /*********************************************************************\
        |* Actual error reporting
        \*********************************************************************/
        static void _report(int line, std::string where, std::string msg);
        
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit Compiler(void);
		~Compiler(void);
		
        /*********************************************************************\
        |* Run the main application
        \*********************************************************************/
		int main(int argc, const char *argv[]);

        /*********************************************************************\
        |* Error handling
        \*********************************************************************/
        static void error(int line, std::string msg);
        
	};

#endif /* Compiler_h */
