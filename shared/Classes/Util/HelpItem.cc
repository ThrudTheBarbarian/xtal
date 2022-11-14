//
//  HelpItem.cc
//  xtal2
//
//  Created by Thrud The Barbarian on 10/19/22.
//

#include "HelpItem.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
HelpItem::HelpItem(std::string shortArg,
				   std::string longArg,
				   std::string helpText)
		 :_shortArg(shortArg)
		 ,_longArg(longArg)
		 ,_helpText(helpText)
	{}
