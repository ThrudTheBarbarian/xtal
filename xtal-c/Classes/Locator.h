//
//  Locator.h
//  xtal-c
//
//  Created by Thrud The Barbarian on 12/28/22.
//

#ifndef Locator_h
#define Locator_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

#define LOCATOR Locator::sharedInstance()

#include "NotifyCenter.h"

class Locator
	{
    NON_COPYABLE_NOR_MOVEABLE(Locator)
 
	/************************************************************************\
    |* Properties
    \************************************************************************/

    private:
        static std::shared_ptr<Locator>	    		_instance;
		
		/********************************************************************\
        |* Places we know about
        \********************************************************************/
		std::map<String, int>						_where;
		String										_which;
		
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Locator();

        /********************************************************************\
        |* Callback: We have a new-file event, start counting lines in this
        |* file now
        \********************************************************************/
		void newFile(NotifyData &d);

        /********************************************************************\
        |* Callback: Increment the current file's line number
        \********************************************************************/
		void nextLine(void);
		
        /********************************************************************\
        |* Return the current line number
        \********************************************************************/
		int lineNumber(void);

        /********************************************************************\
        |* Return the current file
        \********************************************************************/
		String file(void);
		
        /********************************************************************\
        |* Return the current location
        \********************************************************************/
		String location(void);
		
        /********************************************************************\
        |* This method returns the shared instance
        \********************************************************************/
        static std::shared_ptr<Locator> sharedInstance();
	};

#endif /* Locator_h */
