//
//  ContextMgr.h
//  xtal-a
//
//  Created by Simon Gornall on 11/17/22.
//

#ifndef ContextMgr_h
#define ContextMgr_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class ContextMgr
	{
    NON_COPYABLE_NOR_MOVEABLE(ContextMgr)
 
	/*************************************************************************\
    |* Enums etc.
    \*************************************************************************/
	public:
		typedef enum
			{
			FILE	= 0,
			MACRO,
			FUNCTION,
			CLASS,
			METHOD,
			BLOCK,
			UNKNOWN
			} Type;
    
		typedef struct
			{
			String name;				// Human-readable name for this context
			Type type;					// One of the enums for context type
			int64_t line;				// Where we are in the context
			} Context;
			
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    
    private:
        std::vector<Context> _ctxList;					// Context hierarchy
        std::vector<Context*> _blockList;				// Block contexts
        std::vector<Context*> _fileList;				// File contexts
        
        static std::shared_ptr<ContextMgr> _instance;	// Shared instance
 
		/*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit ContextMgr();

		/*********************************************************************\
        |* Return a human-readable version of the type
        \*********************************************************************/
		String _type(Type type);
		
    public:

        /*********************************************************************\
        |* Add a context level, of a given type
        \*********************************************************************/
        Context& push(const String name, Type type, int64_t line = 1);
        Context& push(const String name, String type, int64_t line = 1);
        
        /*********************************************************************\
        |* Increment the context line number
        \*********************************************************************/
		int64_t incLine(int64_t delta = 1);
        
        /*********************************************************************\
        |* Pop a context
        \*********************************************************************/
		void pop(void);
        
        /*********************************************************************\
        |* Reset all the contexts
        \*********************************************************************/
		void reset(void);
        
        /*********************************************************************\
        |* Return the current location as a string
        \*********************************************************************/
		String location(void);
        
        /*********************************************************************\
        |* Return the current context identifier for label naming
        \*********************************************************************/
		String identifier(int idx);

        /**********************************************************************\
        |* This method returns the default global notification center.  You may
        |* alternatively create your own notification center without using the
        |* default notification center
        |*
        |* notification	: the notification you wish to observe.
        \**********************************************************************/
        static std::shared_ptr<ContextMgr> sharedInstance();

	};

#endif /* ContextMgr_h */
