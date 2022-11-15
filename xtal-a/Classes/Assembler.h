//
//  Assembler.h
//  as8
//
//  Created by Thrud The Barbarian on 10/27/22.
//

#ifndef Assembler_h
#define Assembler_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

#include "Macro.h"
#include "OutputBlock.h"

class ArgParser;

class Assembler
	{
    NON_COPYABLE_NOR_MOVEABLE(Assembler)
 
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GET(ArgParser*, ap);				// Permanent reference to the arguments
    GET(int, line);						// The current line number
    GET(StringList, includeDirs);		// List of places to pull includes from
    GET(StringList, symbols);			// List of provided symbols
    GET(BlockList, blocks);				// List of output blocks
    GET(String, output);				// Output filename
    GET(String, listFile);				// Output listing filename
    GET(MacroMap, macros);				// map of named macros
    
    private:
        /*********************************************************************\
        |* General run() method
        \*********************************************************************/
        int _run(std::string content);
        
        /*********************************************************************\
        |* Actual error reporting
        \*********************************************************************/
        static void _report(int line, String where, String msg);
        
        /*********************************************************************\
        |* Find an included file on the filesystem
        \*********************************************************************/
        String _find(String path);
        
        /*********************************************************************\
        |* Handle .include or .macro/.endmacro directives
        \*********************************************************************/
        String _preparse(String path);
        
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit Assembler();
		~Assembler();
		
        /*********************************************************************\
        |* Run the main application
        \*********************************************************************/
		int main(int argc, const char *argv[]);

        /*********************************************************************\
        |* Error handling
        \*********************************************************************/
        static void error(int line, String msg);
	};

#endif /* Assembler_h */