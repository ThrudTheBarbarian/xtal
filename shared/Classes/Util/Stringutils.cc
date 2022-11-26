//
//  stringutils.cc
//  xtal
//
//  Created by Thrud on 10/13/22.
//  Copyright © 2020 All rights reserved.
//

#include "Stringutils.h"

const std::string WHITESPACE = " \n\r\t\f\v";

/*****************************************************************************\
|* Trim whitespace from left
\*****************************************************************************/
std::string ltrim(const std::string &s)
	{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
	}
 
/*****************************************************************************\
|* Trim whitespace from right
\*****************************************************************************/
std::string rtrim(const std::string &s)
	{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
	}
 
/*****************************************************************************\
|* Trim whitespace
\*****************************************************************************/
std::string trim(const std::string &s)
	{
    return rtrim(ltrim(s));
	}

/*****************************************************************************\
|* Return a lowercase copy of the input
\*****************************************************************************/
std::string lcase(const std::string &s)
	{
	std::string data = s;
	std::transform(data.begin(),
				   data.end(),
				   data.begin(),
				   [](unsigned char c){ return std::tolower(c); });
	return data;
	}

/*****************************************************************************\
|* Return an uppercase copy of the input
\*****************************************************************************/
std::string ucase(const std::string &s)
	{
	std::string data = s;
	std::transform(data.begin(),
				   data.end(),
				   data.begin(),
				   [](unsigned char c){ return std::toupper(c); });
	return data;
	}

/*****************************************************************************\
|* Split a string by a character, called by below
\*****************************************************************************/
std::vector<std::string> split(const std::string &text, char sep)
	{
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos)
		{
		tokens.push_back(trim(text.substr(start, end - start)));
		start = end + 1;
		}
	tokens.push_back(trim(text.substr(start)));
	return tokens;
	}

std::vector<std::string> split(const std::string& text, const std::string& by)
	{
    std::vector<std::string> tokens;
    std::size_t start = text.find_first_not_of(by), end = 0;

    while((end = text.find_first_of(by, start)) != std::string::npos)
		{
        tokens.push_back(text.substr(start, end - start));
        start = text.find_first_not_of(by, end);
		}
    if(start != std::string::npos)
        tokens.push_back(text.substr(start));

    return tokens;
	}
	
/*****************************************************************************\
|* Join a vector by a string
\*****************************************************************************/
std::string join(std::vector<std::string> &v, std::string delimiter)
	{
    std::string out;
    if (auto i = v.begin(), e = v.end(); i != e)
		{
        out += *i++;
        for (; i != e; ++i)
			out.append(delimiter).append(*i);
		}
    return out;
	}

/*****************************************************************************\
|* See if a string ends with another, case optional
\*****************************************************************************/
bool endsWith (std::string const &haystack,
			   std::string const &needle,
			   bool caseSensitive)
	{
    std::string h = (caseSensitive) ? haystack : lcase(haystack);
    std::string n = (caseSensitive) ? needle   : lcase(needle);
    
    if (h.length() >= n.length())
        return (0 == h.compare (h.length() - n.length(), n.length(), n));
	return false;
    }

/*****************************************************************************\
|* Turn am int into a hex string, optionally with a prefix
\*****************************************************************************/
std::string toHexString(int value, std::string prefix)
	{
	std::ostringstream ss;
	ss << prefix << std::hex << value;
	return ss.str();
	}
	
/*****************************************************************************\
|* replace any occurrence of a substring with another string and return it
\*****************************************************************************/
std::string replace(const std::string& src,
					const std::string& searchFor,
					const std::string& replaceWith,
					bool replaceAll)
	{
	if (searchFor.empty())
		return src;
	
	std::string result;
	std::string::size_type startPos = 0;
	std::string::size_type pos;
	
	do
		{
		pos = src.find(searchFor, startPos);
		if (pos == std::string::npos)
			break;
		
		result.append(src, startPos, pos - startPos);
		result.append(replaceWith);
		startPos = pos + searchFor.size();
		}
	while (replaceAll);
	
	result.append(src, startPos, src.length() - startPos);
	return result;
	}

/*****************************************************************************\
|* Return a random string of the specified length
\*****************************************************************************/
std::string randomString( size_t length )
	{
    auto randchar = []() -> char
		{
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
		};
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
	}
