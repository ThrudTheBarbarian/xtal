//
//  Driver.h
//  xtal
//
//  Created by Simon Gornall on 11/25/22.
//

#ifndef Driver_h
#define Driver_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class ArgParser;

class Driver
	{
    NON_COPYABLE_NOR_MOVEABLE(Driver)
 
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GET(ArgParser*, ap);				// Permanent reference to the arguments
	GET(String, baseDir);				// Where everything is installed
	GET(String, tmpDir);				// Temporary working directory
    GET(StringList, includeDirs);		// List of places to pull includes from
    GET(StringList, symbols);			// List of provided symbols
    GET(String, output);				// Output filename
    GET(String, hexOutput);				// Hex-dump Output filename
    GET(String, listFile);				// Output listing filename
	GET(String, asmFile);				// Intermediate assembly file
	GET(bool, dumpTree);				// Dump out the AS tree
	
    private:
		/*********************************************************************\
        |* Generate the commandline for the compiler command
        \*********************************************************************/
		String _cCmd(void);
       
		/*********************************************************************\
        |* Generate the commandline for the assembler command
        \*********************************************************************/
		String _aCmd(void);
       
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Driver();

        /*********************************************************************\
        |* Entry point from main()
        \*********************************************************************/
		int main(int argc, const char *argv[]);
	};

#endif /* Driver_h */
