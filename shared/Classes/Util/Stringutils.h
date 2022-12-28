//
//  stringutils.h
//  xtal
//
//  Created by Thrud on 10/13/22.
//  Copyright © 2020 All rights reserved.
//

#ifndef stringutils_h
#define stringutils_h

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>

/*****************************************************************************\
|* Trim whitespace from left
\*****************************************************************************/
std::string ltrim(const std::string &s);

/*****************************************************************************\
|* Trim whitespace from right
\*****************************************************************************/
std::string rtrim(const std::string &s);

/*****************************************************************************\
|* Trim whitespace
\*****************************************************************************/
std::string trim(const std::string &s);

/*****************************************************************************\
|* Return a lowercase copy of the input
\*****************************************************************************/
std::string lcase(const std::string &s);

/*****************************************************************************\
|* Return an uppercase copy of the input
\*****************************************************************************/
std::string ucase(const std::string &s);

/*****************************************************************************\
|* Split a string by a character, called by below
\*****************************************************************************/
std::vector<std::string> split(const std::string& text, char sep);
std::vector<std::string> split(const std::string& text, const std::string& by);

/*****************************************************************************\
|* Join a vector by a string
\*****************************************************************************/
std::string join(std::vector<std::string> &v, std::string delimiter = ",");

/*****************************************************************************\
|* See if a string ends with another, case optional
\*****************************************************************************/
bool endsWith (std::string const &haystack,
			   std::string const &needle,
			   bool caseSensitive=true);

/*****************************************************************************\
|* See if a string starts with another, case optional
\*****************************************************************************/
bool startsWith (std::string const &haystack,
			     std::string const &needle,
			     bool caseSensitive=true);

/*****************************************************************************\
|* Turn am int into a hex string, optionally with a prefix
\*****************************************************************************/
std::string toHexString(int value, std::string prefix = "");
		
/*****************************************************************************\
|* replace any occurrence of a substring with another string and return it
\*****************************************************************************/
std::string replace(const std::string& src,
					const std::string& searchFor,
					const std::string& replaceWith,
					bool replaceAll=true);
	   

/*****************************************************************************\
|* Return a random string of the specified length
\*****************************************************************************/
std::string randomString(size_t length);

#endif /* stringutils_h */
