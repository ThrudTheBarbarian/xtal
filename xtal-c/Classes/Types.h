//
//  Types.h
//  xtal-c
//
//  Created by Simon Gornall on 11/30/22.
//

#ifndef Types_h
#define Types_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"
#include "sharedDefines.h"

class Types
	{
    NON_COPYABLE_NOR_MOVEABLE(Types)
 
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    
    private:
        /*********************************************************************\
        |* Return the size of a given type
        \*********************************************************************/
        static int _typeSize(int type, int line);
        
    public:
        /*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit Types();

        /*********************************************************************\
        |* Check whether two types are compatible
        \*********************************************************************/
        static bool areCompatible(int line,
								  int& left,
								  int& right,
								  bool onlyRight = false);
							
	};

#endif /* Types_h */
