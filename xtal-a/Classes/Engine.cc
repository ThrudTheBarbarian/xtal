//
//  Engine.cc
//  as8
//
//  Created by Thrud The Barbarian on 10/29/22.
//

#include "Engine.h"

extern int yyparse();
	
	
/****************************************************************************\
|* Constructor
\****************************************************************************/
Engine::Engine()
	{
	}
	
/*****************************************************************************\
|* Scan an integer, coping with different bases
\*****************************************************************************/
int64_t Engine::parseNumber(void)
	{
	int val		= 0;
	int base 	= 10;
	int c 		= _next();

	if (!::isdigit(c))
		{
		if (c == '$')
			base = 16;
		else if (c == '%')
			base = 2;
		else if (c == '\'')
			{
			if (_peek(1) == '\'')
				{
				c = _next();
				_next();
				return c;
				}
			else
				FATAL(ERR_PARSE, "Cannot understand number/base in %s",
					_src.c_str());
			}
		else
			FATAL(ERR_PARSE, "Cannot understand number/base");
		c = _next();
		}

	/*************************************************************************\
    |* Look for digit characters, since this is scanning for integers and
    |* accumulate the value
    \*************************************************************************/
	while (::isdigit(c) || ((base == 16) && isxdigit(c)))
		{
		if (base == 16)
			{
			if (c > 'F')
				c -= ('f' - 'F');
				
			if (c > '9')
				c -= 7;
			}
		
		int digit = c - '0';
		
		val = val * base + digit;
		c = _next();
		}

	/*************************************************************************\
    |* We hit a non-digit character so put it back and return the value
    \*************************************************************************/
	if (!_atEnd())
		_putBack();
	return val;
	}

	
/*****************************************************************************\
|* Look up the value of a label dynamically
\*****************************************************************************/
int64_t Engine::dynamicLookup(String s)
	{
	if (s.ends_with("_?"))
		{
		s.pop_back();
		s.pop_back();
		s = nextLabel(s);
		}
		
	bool ok = _symbols.contains(s);
	int64_t value	= ok ? _symbols[s].value : 0;
	return value;
	}
	
/*****************************************************************************\
|* Return the number of the next label for a given stem
\*****************************************************************************/
String Engine::nextLabel(String stem)
	{
	int idx = 1;
	bool ok = _labelMap.contains(stem);
	if (ok)
		idx = _labelMap[stem];
	
	return stem+"_"+std::to_string(idx);
	}
	
/*****************************************************************************\
|* Increment the label id for a given stem
\*****************************************************************************/
void Engine::incLabel(String stem)
	{
	int idx = 0;
	bool ok = _labelMap.contains(stem);
	if (ok)
		idx = _labelMap[stem];
	
	_labelMap[stem] = idx + 1;
	}
	
/*****************************************************************************\
|* Evaluate an expression
\*****************************************************************************/
bool Engine::eval(String s)
	{
	bool ok = true;
	_src 	= s+"\n";
	fprintf(stderr, "[%s]\n", s.c_str());
	_at 	= 0;
	//fprintf(stderr, "Eval '%s'\n", s.c_str());
	yyparse();
	return ok;
	}
	
/*****************************************************************************\
|* Get a character from the string
\*****************************************************************************/
int Engine::getc(void)
	{
	return _next();
	}
	
/*****************************************************************************\
|* Get a character from the string
\*****************************************************************************/
bool Engine::eof(void)
	{
	return _atEnd();
	}

/*****************************************************************************\
|* Emulate unget() for the string domain
\*****************************************************************************/
void Engine::unget(void)
	{
	_putBack();
	}

/*****************************************************************************\
|* Insert or update a symbol value
\*****************************************************************************/
void Engine::updateSymbol(String name, int64_t value)
	{
	_symbols[name].type 	= _varType;
	_symbols[name].value	= value;
	_symbols[name].name		= &(String&)_symbols.find(name)->first;
	}

/*****************************************************************************\
|* Dump out the named variables
\*****************************************************************************/
void Engine::dumpVars(void)
	{
	fprintf(stderr, "Symbol table:\n");
	for (Elements<std::string, Symbol> kv : _symbols)
		{
		fprintf(stderr, "  %s : {%s, %lld, 0x%llx}\n",
			kv.key.c_str(),
			kv.value.name->c_str(),
			kv.value.value,
			kv.value.value
			);
		}
	}

/*****************************************************************************\
|* return a symbol's value if it exists
\*****************************************************************************/
bool  Engine::symbolValue(String name, int64_t &value)
	{
	bool ok = _symbols.contains(name);
	value	= ok ? _symbols[name].value : 0;
	return ok;
	}
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Peek ahead and see what's coming
\*****************************************************************************/
int Engine::_peek(int delta)
	{
	if (_atEnd(delta))
		return EOF;
	return _src[_at+delta];
	}

/*****************************************************************************\
|* Get the next character in the input stream
\*****************************************************************************/
int Engine::_next(void)
	{
	if (_atEnd())
		return EOF;
		
	int c = _src[_at];
	_at ++;
	return c;
	}

	
/*****************************************************************************\
|* Are we at the end of the stream
\*****************************************************************************/
bool Engine::_atEnd(int delta)
	{
	return (_at + delta >= _src.size());
	}
	
/*****************************************************************************\
|* Put a character 'back' in the input stream
\*****************************************************************************/
void Engine::_putBack(void)
	{
	if (_at > 0)
		_at --;
	}

