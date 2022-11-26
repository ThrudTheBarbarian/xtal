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

class Compiler
	{
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GET(ArgParser*, ap);			// Permanent reference to the arguments
    GET(bool, hadError);			// Whether we've seen a syntax error
    GET(int, line);					// The current line number
    GET(Emitter *, emitter);		// How we produce assembly
    
    private:
    
        /*********************************************************************\
        |* General run() method
        \*********************************************************************/
        int _run(std::string content, FILE *fp);
        
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
