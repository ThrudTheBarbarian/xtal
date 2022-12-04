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
							
        /*********************************************************************\
        |* Return the size of a given type
        \*********************************************************************/
        static int typeSize(int type, int line=-1);
							
        /*********************************************************************\
        |* Given a primitive type, return the type which is a pointer to it
        \*********************************************************************/
        static int pointerTo(int type, int line=-1);
							
        /*********************************************************************\
        |* Given a primitive pointer, return the type which it points to
        \*********************************************************************/
        static int valueAt(int type, int line=-1);
        
	};

#endif /* Types_h */
