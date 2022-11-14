//
//  ArgParser.h
//
//  Created by Thrud on 7/19/20.
//  Copyright © 2020 All rights reserved.
//

#ifndef ArgParser_h
#define ArgParser_h

#include <cstdio>
#include <string>
#include <vector>
#include <map>

#include "properties.h"
#include "macros.h"
#include "HelpItem.h"

typedef std::vector<HelpItem> HelpList;

class ArgParser
	{
    NON_COPYABLE_NOR_MOVEABLE(ArgParser)
 
	/**************************************************************************\
    |* Properties
    \**************************************************************************/
    GET(int, argc);                     			// Copy of argc
    GET(const char **, argv);           			// Copy of argv
    GET(std::string, progname);						// Program name
    
    private:
        std::map<std::string, HelpList>	_help;		// Help text
		StringList _flags;							// To find remaining args
		
        /**********************************************************************\
        |* Add the help to the list
        \**********************************************************************/
		void _addHelp(std::string help, 			// Text to add
					  std::string shrt,				// Short argument string
					  std::string lng,				// Long argument string
					  std::string group="");		// Group to place help in
	
        /**********************************************************************\
        |* Display an error
        \**********************************************************************/
        void _error(std::string arg);
        
    public:
        /**********************************************************************\
        |* Constructors and Destructor
        \**********************************************************************/
        ArgParser(int argc, const char **argv);

        /**********************************************************************\
        |* Show the usage message
        \**********************************************************************/
		void usage(bool fatal);
		
        /**********************************************************************\
        |* Access the fields
        \**********************************************************************/
        std::string stringFor(const char *shrt,
                              const char *lng,
                              const char *dflt,
						      const char *group=nullptr,
						      const char *help=nullptr);
                              
        int         intFor(const char *shrt,
						   const char *lng,
						   int dflt,
						   const char *group=nullptr,
						   const char *help=nullptr);
						   
        float       floatFor(const char *shrt,
							 const char *lng,
							 float dflt,
						     const char *group=nullptr,
						     const char *help=nullptr);
						     
        int         flagFor(const char *shrt,
							const char *lng,
							int dflt,
						    const char *group=nullptr,
						    const char *help=nullptr);
		
		StringList	listOf(const char *shrt,
						   const char *lng,
						   const char *group=nullptr,
						   const char *help=nullptr);
		
		StringList	remainingArgs(void);
        
	};
	
#endif // ! ArgParser_h
