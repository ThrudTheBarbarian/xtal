//
//  Locator.cc
//  xtal-c
//
//  Created by Thrud The Barbarian on 12/28/22.
//

#include "sharedDefines.h"
#include "stringUtils.h"

#include "Locator.h"

/****************************************************************************\
|* The shared object for retrieving the current location
\****************************************************************************/
std::shared_ptr<Locator> Locator::_instance = NULL;


/****************************************************************************\
|* Constructor
\****************************************************************************/
Locator::Locator()
	{
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &z){newFile(z);}, NC_FILE_START);
	nc->addObserver(std::bind(&Locator::nextLine, this), NC_LINE_INC);
	
	_which = "unknown";
	_where[_which] = 0;
	}
	
/****************************************************************************\
|* Return the default notification center
\****************************************************************************/
std::shared_ptr<Locator> Locator::sharedInstance()
    {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (!_instance)
        _instance = std::shared_ptr<Locator>(new Locator);


    return _instance;
    }

/****************************************************************************\
|* Callback method: We have a new file to handle lines in
\****************************************************************************/
void Locator::newFile(NotifyData &d)
	{
	StringList info = split(d.stringValue(), " ");
	if (info.size() == 2)
		{
		int line;
		sscanf(info[0].c_str(), "%d", &line);
		String file = trim(info[1]);
		_where[file] = line;
		_which = file;
		}
	else
		FATAL(ERR_LOCATION, "Internal error for #:line '%s'",
							d.stringValue().c_str());
	}

/****************************************************************************\
|* Callback method: Increment the current file line number
\****************************************************************************/
void Locator::nextLine(void)
	{
	_where[_which] ++;
	}
	

/****************************************************************************\
|* Return the current line
\****************************************************************************/
int Locator::lineNumber(void)
	{
	return _where[_which];
	}

/****************************************************************************\
|* Return the current file
\****************************************************************************/
String Locator::file(void)
	{
	return _which;
	}

/****************************************************************************\
|* Return the current location
\****************************************************************************/
String Locator::location(void)
	{
	return _which + ":" + std::to_string(_where[_which]);
	}

