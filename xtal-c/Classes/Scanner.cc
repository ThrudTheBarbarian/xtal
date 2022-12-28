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

#include "NotifyCenter.h"

#define MAX_STRING_LENGTH 32768

static Token _rejected;				// Token we scanned and no longer need

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Scanner::Scanner(String src)
		:_src(src)
		,_at(0)
	{
	_rejected.setValid(false);
	_nc = NotifyCenter::defaultNotifyCenter();
	}

/*****************************************************************************\
|* Scan for tokens
\*****************************************************************************/
int Scanner::scan(Token& token)
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
	int c = _skipWhitespace();

	/*************************************************************************\
    |* First check to see that there's no directive here
    \*************************************************************************/
	if (c == '#')
		c = _scanDirective();
		
	/*************************************************************************\
    |* Process any token
    \*************************************************************************/
	switch (c)
		{
		case EOF:
			token.setToken(Token::T_NONE);
			return SCAN_COMPLETE;
		
		case '+':
			if ((c = _next()) == '+')
				token.setToken(Token::T_INC);
			else
				{
				_putBack();
				token.setToken(Token::T_PLUS);
				}
			break;
		
		case '-':
			if ((c = _next()) == '-')
				token.setToken(Token::T_DEC);
			else
				{
				_putBack();
				token.setToken(Token::T_MINUS);
				}
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

		case ',':
 			token.setToken(Token::T_COMMA);
			break;

		case '~':
 			token.setToken(Token::T_INVERT);
			break;

		case '^':
 			token.setToken(Token::T_XOR);
			break;

		case '=':
			if ((c = _next()) == '=')
				token.setToken(Token::T_EQ);
			else
				{
				_putBack();
				token.setToken(Token::T_ASSIGN);
				}
			break;
		
		case '!':
			if ((c = _next()) == '=')
				token.setToken(Token::T_NE);
			else
				{
				_putBack();
				token.setToken(Token::T_LOGNOT);
				}
			break;
		
		case '<':
			if ((c = _next()) == '=')
				token.setToken(Token::T_LE);
			else if (c == '<')
				token.setToken(Token::T_LSHIFT);
			else
				{
				_putBack();
				token.setToken(Token::T_LT);
				}
			break;
		
		case '>':
			if ((c = _next()) == '=')
				token.setToken(Token::T_GE);
			else if (c == '>')
				token.setToken(Token::T_RSHIFT);
			else
				{
				_putBack();
				token.setToken(Token::T_GT);
				}
			break;

		case '&':
			if ((c = _next()) == '&')
				token.setToken(Token::T_LOGAND);
			else
				{
				_putBack();
				token.setToken(Token::T_AMPER);
				}
			break;
		
		case '|':
			if ((c = _next()) == '|')
				token.setToken(Token::T_LOGOR);
			else
				{
				_putBack();
				token.setToken(Token::T_OR);
				}
			break;

		case '\'':
			// If it's a quote, scan in the literal character value and
			// the trailing quote
			token.setIntValue(_scanCharacter());
			token.setToken(Token::T_INTLIT);
			if (_next() != '\'')
				FATAL(ERR_LEX_BAD_CHAR, "Expected '\\' at end of char literal");
			break;
			
		case '"':
			// Scan in a literal string
			_scanString();
			token.setToken(Token::T_STRLIT);
			break;

		default:
			if (::isdigit(c))
				{
				token.setIntValue(_scanInteger(c));
				token.setToken(Token::T_INTLIT);
				break;
				}
			else if (::isalpha(c) || ('_' == c))
				{
				// Read in a keyword or identifier
				_scanIdentifier(c);
				
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
					"Unrecognised character '%c'", c);
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
int Scanner::_next(void)
	{
	if (_atEnd())
		return EOF;
		
	int c = _src[_at++];
	if (c == '\n')
		_nc->notify(NC_LINE_INC);
		
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
int Scanner::_skipWhitespace(void)
	{
	int c = _next();
	
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
				c = _next();
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
int Scanner::_scanInteger(int c)
	{
	int val = 0;

	/*************************************************************************\
    |* Look for digit characters, since this is scanning for integers and
    |* accumulate the value
    \*************************************************************************/
	while (::isdigit(c))
		{
		val = val * 10 + (c-'0');
		c = _next();
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
int Scanner::_scanIdentifier(int c)
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
		c = _next();
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
			else if (lc == "s16")
				return Token::T_S16;
			else if (lc == "s8")
				return Token::T_S8;
			break;

		case 'u':
			if (lc == "u8")
				return Token::T_U8;
			else if (lc == "u16")
				return Token::T_U16;
			else if (lc == "u32")
				return Token::T_U32;
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

	
/*****************************************************************************\
|* Scan a character literal
\*****************************************************************************/
int Scanner::_scanCharacter(void)
	{
	int c = _next();
	if (c == '\\')
		{
		switch (c = _next())
			{
			case 'a':  return '\a';
			case 'b':  return '\b';
			case 'f':  return '\f';
			case 'n':  return '\n';
			case 'r':  return '\r';
			case 't':  return '\t';
			case 'v':  return '\v';
			case '\\': return '\\';
			case '"':  return '"' ;
			case '\'': return '\'';
      
			default:
				FATAL(ERR_PARSE, "Unknown escape sequence \%c", c);
			}
		}
	return (c); 	// Just an ordinary old character!
	}
	
	
/*****************************************************************************\
|* Scan a character literal
\*****************************************************************************/
int Scanner::_scanString(void)
	{
	_text = "";
	
	for (int i=0; i<MAX_STRING_LENGTH; i++)
		{
		int c = _scanCharacter();
		if (c == '"')
			return (int)(_text.length());
		_text += c;
		}
	FATAL(ERR_PARSE, "String too long");
	}

/*****************************************************************************\
|* Scan a directive, or just random text until the end of a line
\*****************************************************************************/
int Scanner::_scanDirective(void)
	{
	String text = "";
	int c = _next();
	while (c != EOF)
		{
		if (c == '\n')
			break;
		text += c;
		c = _next();
		}
	
	if (startsWith(text, ":line", false))
		_nc->notify(NC_FILE_START, text.substr(5));
	
	return _skipWhitespace();
	}




