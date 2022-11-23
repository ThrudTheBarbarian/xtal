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
			String name;			// Human-readable name for this context
			Type type;				// One of the enums for context type
			int64_t line;			// Where we are in the context
			int contextId;			// Incrementing context id
			} Context;
			
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    
    private:
        std::vector<Context> 	_ctxList;				// Context hierarchy
        std::vector<int> 		_blockList;				// index of block ctxs
        std::vector<int> 		_fileList;				// index of file ctxs
        int 					_nextContextId;			// Incrementing int
        
        static std::shared_ptr<ContextMgr> _instance;	// Shared instance
 
		/*********************************************************************\
        |* Constructors and Destructor
        \*********************************************************************/
        explicit ContextMgr();

		/*********************************************************************\
        |* A map of label:address pairs, built up by context-id
        \*********************************************************************/
		std::map<int,std::map<String,int>> _labels;
		
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
		String identifier(void);

        /*********************************************************************\
        |* Add a label to the current or global (if starts with @) context
        \*********************************************************************/
		void addLabel(String label, int location);

        /*********************************************************************\
        |* Return a label's value or -1
        \*********************************************************************/
		bool labelValue(String label, int& value);

        /*********************************************************************\
        |* Return all the context's label values
        \*********************************************************************/
		String labelValues(void);
	
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
