//
//  ArgParser.cc
//  dbio
//
//  Created by Thrud on 7/19/20.
//  Copyright © 2020 All rights reserved.
//
#include <filesystem>
namespace fs = std::filesystem;

#include "ArgParser.h"

/*****************************************************************************\
|* Store the initial values for argc, argv
\*****************************************************************************/
ArgParser::ArgParser(int argc, const char **argv)
          :_argc(argc)
          ,_argv(argv)
    {
    _progname = fs::path(argv[0]).filename();
	}
    
/*****************************************************************************\
|* Search for a string argument
\*****************************************************************************/
std::string ArgParser::stringFor(const char *shrt,
                                 const char *lng,
                                 const char *dflt,
                                 const char *group,
                                 const char *help)
	{
	for (int i=1; i<_argc-1; i++)
		if (strcmp(shrt, _argv[i]) == 0 || strcmp(lng, _argv[i]) == 0)
			return std::string(_argv[i+1]);
	return std::string(dflt);
	}
    
/*****************************************************************************\
|* Search for an int argument
\*****************************************************************************/
int ArgParser::intFor(const char *shrt,
					  const char *lng,
					  int dflt,
			 		  const char *group,
					  const char *help)
	{
	for (int i=1; i<_argc-1; i++)
		if (strcmp(shrt, _argv[i]) == 0 || strcmp(lng, _argv[i]) == 0)
			return atoi(_argv[i+1]);
	return dflt;
	}
    
/*****************************************************************************\
|* Search for flag arguments. Allow multiple
\*****************************************************************************/
int ArgParser::flagFor(const char *shrt,
					   const char *lng,
					   int dflt,
			 		   const char *group,
					   const char *help)
	{
	_flags.push_back(shrt);
	if (help && group)
		_addHelp(help, shrt, lng, group);

    int total = 0;
	for (int i=1; i<_argc; i++)
		if (strcmp(shrt, _argv[i]) == 0 || strcmp(lng, _argv[i]) == 0)
			total ++;

	return (total == 0) ? dflt : total;
	}
    
/*****************************************************************************\
|* Search for a float argument
\*****************************************************************************/
float ArgParser::floatFor(const char *shrt,
						  const char *lng,
						  float dflt,
						  const char *group,
						  const char *help)
	{
	for (int i=1; i<_argc-1; i++)
		if (strcmp(shrt, _argv[i]) == 0 || strcmp(lng, _argv[i]) == 0)
			return atof(_argv[i+1]);
	return dflt;
	}
    
/*****************************************************************************\
|* Return a vector of strings for a given flag
\*****************************************************************************/
StringList ArgParser::listOf(const char *shrt,
						     const char *lng,
						     const char *group,
						     const char *help)
	{
	if (help && group)
		_addHelp(help, shrt, lng, group);

	StringList list;
	for (int i=1; i<_argc-1; i++)
		if (strcmp(shrt, _argv[i]) == 0 || strcmp(lng, _argv[i]) == 0)
			{
			list.push_back(_argv[i+1]);
			i ++;
			}

	return list;
	}
    
/*****************************************************************************\
|* Return a vector of strings that are not associated with switches
\*****************************************************************************/
StringList ArgParser::remainingArgs(void)
	{
	StringList list;
	bool skip = false;
	for (int i=1; i<_argc; i++)
		{
		const char *arg = _argv[i];
		if (arg[0] == '-')
			{
			if (std::find(_flags.begin(), _flags.end(), arg) == _flags.end())
				skip = true;
			}
		else if (skip)
			skip = false;
		else
			list.push_back(std::string(arg));
		}
	return list;
	}


/*****************************************************************************\
|* Show the usage
\*****************************************************************************/

/*****************************************************************************\
|* Show the usage
\*****************************************************************************/
void ArgParser::usage(bool fatal)
	{
	std::string msg = "Usage : " + _progname + " [options], "
					  "where options are from:\n\n";

	/*************************************************************************\
	|* Generate a list of all the keys as a vector
	\*************************************************************************/
	StringList allKeys;
	for (Elements<std::string, HelpList> kv : _help)
		allKeys.push_back(kv.key);
		
	/*************************************************************************\
	|* Figure out pad-lengths
	\*************************************************************************/
	size_t slen		= 0;
	size_t llen		= 0;
	size_t hlen		= 0;

	for (std::string section : allKeys)
		{
		auto i = _help.find(section);
		for (HelpItem item : i->second)
			{
			size_t len	= item.longArg().length();
			llen		= (llen > len) ? llen : len;
			
			len			= item.shortArg().length();
			slen		= (slen > len) ? slen : len;
			
			len			= item.helpText().length();
			hlen		= (hlen > len) ? hlen : len;
			}
		}

	/*************************************************************************\
	|* Format the help text
	\*************************************************************************/
	std::sort(allKeys.begin(), allKeys.end(),
		[](const std::string & a, const std::string & b) -> bool
			{
			return a < b;
			});

	for (std::string key : allKeys)
		{
		std::string title = key + ":\n";		// dark grey
		msg += title;

		HelpList list = _help.find(key)->second;
		std::sort(list.begin(), list.end(),
			[](HelpItem& a, HelpItem& b) -> bool
				{
				return a.shortArg() < b.shortArg();
				});
		
		for (HelpItem item : list)
			{
			std::string padS = item.shortArg();
			padS.insert(padS.begin(), slen - padS.size(), ' ');
			
			std::string padL = item.longArg();
			padL.insert(padL.end(), llen - padL.size(), ' ');
			
			msg += "  "+padS+"|"+padL+" : "+item.helpText() + "\n";
			}
		msg += "\n";
		}
	
	std::cerr << msg << std::endl << std::endl;
	
	if (fatal)
		::exit(0);
	}

#pragma mark - Private Methods
    
/*****************************************************************************\
|* Add the help text for this argument
\*****************************************************************************/
void ArgParser::_addHelp(std::string help,
						 std::string shrt,
						 std::string lng,
						 std::string group)
	{
	/*************************************************************************\
	|* Find where we can add the text
	\*************************************************************************/
	auto i = _help.find(group);
	if (i == _help.end())
		{
		HelpList list;
		_help[group] = list;
		i = _help.find(group);
		}

	/*************************************************************************\
	|* Check to see that there are no already-conflicting entries
	\*************************************************************************/
	for (Elements<std::string, HelpList> kv : _help)
		{
		for (HelpItem item : kv.value)
			{
			if (item.longArg() == lng)
				_error(lng);
			if (item.shortArg() == shrt)
				_error(shrt);
			}
		}
		
	/*************************************************************************\
	|* Add in the new entry
	\*************************************************************************/
	HelpItem item(shrt, lng, help);
	(i->second).push_back(item);
	}

/*****************************************************************************\
|* Display an error message - to be converted to coloured text
\*****************************************************************************/
void ArgParser::_error(std::string arg)
	{
	std::string prefix  = "ERROR";
	std::string suffix  = "Argument '"+arg+"' is already in use";
	std::string msg		= suffix;
	std::string line	= prefix + " : " + suffix;
	std::cerr << line << std::endl;
	}
