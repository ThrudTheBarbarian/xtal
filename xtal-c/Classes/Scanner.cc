//
//  Scanner.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include "Scanner.h"
#include "Token.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Scanner::Scanner(String src)
		:_src(src)
		,_at(0)
	{
	}

/*****************************************************************************\
|* Scan for tokens
\*****************************************************************************/
int Scanner::scan(Token& token, int& line)
	{
	int c = _skipWhitespace(line);
	
	switch (c)
		{
		case EOF:
			token.setToken(Token::T_NONE);
			return SCAN_COMPLETE;
		
		case '+':
			token.setToken(Token::T_PLUS);
			break;
		
		case '-':
			token.setToken(Token::T_MINUS);
			break;
		
		case '/':
			token.setToken(Token::T_SLASH);
			break;
		
		case '*':
			token.setToken(Token::T_STAR);
			break;
		
		default:
			if (::isdigit(c))
				{
				token.setIntValue(_scanInteger(c, line));
				token.setToken(Token::T_INTLIT);
				break;
				}
			
			FATAL(ERR_LEX_BAD_CHAR,
					"Unrecognised character '%c' on line %d", c, line);
			break;
		}
	return SCAN_MORE;
	}
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Are we at the end of the stream
\*****************************************************************************/
bool Scanner::_atEnd(void)
	{
	return (_at >= _src.size());
	}
	
/*****************************************************************************\
|* Get the next character in the input stream
\*****************************************************************************/
int Scanner::_next(int &line)
	{
	if (_atEnd())
		return EOF;
		
	int c = _src[_at++];
	if (c == '\n')
		line ++;
	return c;
	}
	
/*****************************************************************************\
|* Put a character 'back' in the input stream
\*****************************************************************************/
void Scanner::_putBack(void)
	{
	if (_at > 0)
		_at --;
	}
	
/*****************************************************************************\
|* Skip the whitespace
\*****************************************************************************/
int Scanner::_skipWhitespace(int &line)
	{
	int c = _next(line);
	
	bool readAgain = true;
	while (readAgain)
		{
		readAgain = false;
		
		switch (c)
			{
			case '\n':
			case ' ':
			case '\t':
			case '\r':
			case '\f':
				readAgain = true;
				c = _next(line);
				break;
			
			default:
				break;
			}
		}
	return c;
	}
	
/*****************************************************************************\
|* Scan an integer into a token, starting with character 'c'
\*****************************************************************************/
int Scanner::_scanInteger(int c, int& line)
	{
	int val = 0;

	/*************************************************************************\
    |* Look for digit characters, since this is scanning for integers and
    |* accumulate the value
    \*************************************************************************/
	while (::isdigit(c))
		{
		val = val * 10 + (c-'0');
		c = _next(line);
		}

	/*************************************************************************\
    |* We hit a non-digit character so put it back and return the value
    \*************************************************************************/
	_putBack();
	return val;
	}
