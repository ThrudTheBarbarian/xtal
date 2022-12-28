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

class ASTNode;

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
        |* modify a type within the AST tree
        \*********************************************************************/
		static ASTNode * modify(ASTNode *tree, int rType, int op);
		
        /*********************************************************************\
        |* Return the size of a given type
        \*********************************************************************/
        static int typeSize(int type);
							
        /*********************************************************************\
        |* Given a primitive type, return the type which is a pointer to it
        \*********************************************************************/
        static int pointerTo(int type);
							
        /*********************************************************************\
        |* Given a primitive pointer, return the type which it points to
        \*********************************************************************/
        static int valueAt(int type);
							
        /*********************************************************************\
        |* Return true if something is an integer type
        \*********************************************************************/
        static bool isInt(int type);
							
        /*********************************************************************\
        |* Return true if something is a pointer type
        \*********************************************************************/
        static bool isPointer(int type);
        
	};

#endif /* Types_h */
