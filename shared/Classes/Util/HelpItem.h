//
//  HelpItem.h
//  xtal2
//
//  Created by Thrud The Barbarian on 10/19/22.
//

#ifndef HelpItem_h
#define HelpItem_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class HelpItem
	{ 
	/***************************************************************************\
    |* Properties
    \***************************************************************************/
    GET(std::string, shortArg);				// Short argument string
    GET(std::string, longArg);				// Long argument string
    GET(std::string, helpText);				// Help text itself
    private:
        
    public:
        /***********************************************************************\
        |* Constructors and Destructor
        \***********************************************************************/
        explicit HelpItem(std::string shortArg,
						  std::string longArg,
						  std::string helpText);

        /***********************************************************************\
        |* 
        \***********************************************************************/
	};

#endif /* HelpItem_h */
