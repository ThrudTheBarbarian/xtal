//
//  Token.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include <map>

#include "Token.h"

typedef struct
	{
	int type;				// Type of token
	String name;			// Human-readable name
	int precedence;			// Pratt-parsing precedence value, higher values
							// mean higher precedence
	} TokenInfo;

static TokenInfo _tokens[] =
	{
		{Token::T_PLUS, 	"TOKEN_PLUS       ", 10},
		{Token::T_MINUS, 	"TOKEN_MINUS      ", 10},
		{Token::T_STAR, 	"TOKEN_STAR       ", 20},
		{Token::T_SLASH, 	"TOKEN_SLASH      ", 20},
		{Token::T_INTLIT, "TOKEN_INT_LITERAL", 0},
		{Token::T_NONE, 	"[None]           ", 0}		// Must be last
	};

static std::map<int, TokenInfo> _info;

/*****************************************************************************\
|* Helper function to generate the map if it's not already populated
\*****************************************************************************/
void _populateMap(void)
	{
	if (_info.size() == 0)
		{
		int i = 0;
		forever
			{
			_info[_tokens[i].type] = _tokens[i];
			if (_tokens[i].type == Token::T_NONE)
				break;
			i++;
			}
		}
	}

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Token::Token(int token, int intValue)
	  :_token(token)
	  ,_intValue(intValue)
	{}

/*****************************************************************************\
|* Return a string representation of the token
\*****************************************************************************/
String Token::toString(void)
	{
	/*************************************************************************\
	|* Populate the map if it isn't already holding data
	\*************************************************************************/
	_populateMap();
	
	/*************************************************************************\
	|* Return the human-readable string
	\*************************************************************************/
	auto it = _info.find(_token);
	if (it == _info.end())
		{
		WARN("Cannot find token %d in the token info map", _token);
		it = _info.find(Token::T_NONE);
		}
	return it->second.name;
	}


/*****************************************************************************\
|* Expression precedence (for table driven, Pratt parsing)
\*****************************************************************************/
int Token::precedence(void)
	{
	/*************************************************************************\
	|* Populate the map if it isn't already holding data
	\*************************************************************************/
	_populateMap();
	
	auto it = _info.find(_token);
	if (it == _info.end())
		FATAL(ERR_AST_PRIORITY, "Cannot find token priority %d in AST", _token);
	
	return it->second.precedence;
	}
	

/*****************************************************************************\
|* Expression precedence (for table driven, Pratt parsing)
\*****************************************************************************/
int Token::precedence(int type)
	{
	/*************************************************************************\
	|* Populate the map if it isn't already holding data
	\*************************************************************************/
	_populateMap();
	
	auto it = _info.find(type);
	if (it == _info.end())
		FATAL(ERR_AST_PRIORITY, "Cannot find token priority %d in AST", type);
	
	return it->second.precedence;
	}
	
