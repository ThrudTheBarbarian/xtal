//
//  Scanner.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include "Scanner.h"
#include "StringUtils.h"
#include "Symbol.h"
#include "Token.h"

static Token _rejected;				// Token we scanned and no longer need

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Scanner::Scanner(String src)
		:_src(src)
		,_at(0)
	{
	_rejected.setValid(false);
	}

/*****************************************************************************\
|* Scan for tokens
\*****************************************************************************/
int Scanner::scan(Token& token, int& line)
	{
	/*************************************************************************\
    |* First see if we have a previously-rejected token, and if so return it
    \*************************************************************************/
	if (_rejected.valid())
		{
		token = _rejected;
		_rejected.setValid(false);
		return SCAN_MORE;
		}
	
	/*************************************************************************\
    |* Carry on with the normal parsing
    \*************************************************************************/
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

		case ';':
			token.setToken(Token::T_SEMICOLON);
			break;
				
		case '[':
			token.setToken(Token::T_LBRACE);
			break;
				
		case ']':
			token.setToken(Token::T_RBRACE);
			break;
				
		case '(':
			token.setToken(Token::T_LPAREN);
			break;
				
		case ')':
			token.setToken(Token::T_RPAREN);
			break;
				

		case '=':
			if ((c = _next(line)) == '=')
				token.setToken(Token::T_EQ);
			else
				{
				_putBack();
				token.setToken(Token::T_ASSIGN);
				}
			break;
		
		case '!':
			if ((c = _next(line)) == '=')
				token.setToken(Token::T_NE);
			else
				FATAL(ERR_LEX_BAD_CHAR,
					"Unrecognised character '%c' on line %d", c, line);
			break;
		
		case '<':
			if ((c = _next(line)) == '=')
				token.setToken(Token::T_LE);
			else
				{
				_putBack();
				token.setToken(Token::T_LT);
				}
			break;
		
		case '>':
			if ((c = _next(line)) == '=')
				token.setToken(Token::T_GE);
			else
				{
				_putBack();
				token.setToken(Token::T_GT);
				}
			break;
			
		default:
			if (::isdigit(c))
				{
				token.setIntValue(_scanInteger(c, line));
				token.setToken(Token::T_INTLIT);
				break;
				}
			else if (::isalpha(c) || ('_' == c))
				{
				// Read in a keyword or identifier
				_scanIdentifier(c, line);
				
				// Check to see if it's a known keyword, returns non-zero if so
				int tokenType = _keyword();
				if (tokenType != Token::T_NONE)
					{
					token.setToken(tokenType);
					break;
					}
				
				// Not a recognised keyword, so must be an identifier
				token.setToken(Token::T_IDENT);
				break;
				}
				
			FATAL(ERR_LEX_BAD_CHAR,
					"Unrecognised character '%c' on line %d", c, line);
			break;
		}
	return SCAN_MORE;
	}


/*****************************************************************************\
|* Reject a token
\*****************************************************************************/
void Scanner::reject(Token &t)
	{
	if (_rejected.valid())
		FATAL(ERR_TOKEN, "Can't reject a token twice!");
		
	_rejected = t;
	_rejected.setValid(true);
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
	
/*****************************************************************************\
|* Scan an identifier into _text, starting with character 'c'
\*****************************************************************************/
int Scanner::_scanIdentifier(int c, int& line)
	{
	_text = "";
	
	/*************************************************************************\
    |* Look for digits, underscores and alpha chars
    \*************************************************************************/
	while (isalpha(c) || isdigit(c) || '_' == c)
		{
		if (_text.length() >= Symbol::TEXTLEN)
			FATAL(ERR_PARSE, "Symbol too long");
		_text += c;
		c = _next(line);
		}

	/*************************************************************************\
    |* We hit a non-digit character so put it back and return the value
    \*************************************************************************/
	_putBack();
	return (int) _text.length();
	}
	
/*****************************************************************************\
|* Check to see if the value in _text is a keyword, return 0 if false else
|* return the token identifier
\*****************************************************************************/
int Scanner::_keyword(void)
	{
	String lc = lcase(_text);
	
	switch (lc[0])
		{
		case 'e':
			if (lc == "else")
				return Token::T_ELSE;
			break;
		
		case 'f':
			if (lc == "for")
				return Token::T_FOR;
			break;
		
		case 'i':
			if (lc == "if")
				return Token::T_IF;
			break;
		
		case 'p':
			if (lc == "print")
				return Token::T_PRINT;
			break;
		
		case 'r':
			if (lc == "return")
				return Token::T_RETURN;
			break;

		case 's':
			if (lc == "s32")
				return Token::T_S32;
			else if (lc == "s8")
				return Token::T_S8;
			break;

		case 'u':
			if (lc == "u8")
				return Token::T_U8;
			break;
		
		case 'v':
			if (lc == "void")
				return Token::T_VOID;
			break;
		
		case 'w':
			if (lc == "while")
				return Token::T_WHILE;
			break;
		}
		
	return Token::T_NONE;
	}
