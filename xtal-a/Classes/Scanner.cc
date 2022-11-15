//
//  Scanner.cc
//  as8
//
//  Created by Thrud The Barbarian on 10/28/22.
//
#include  <regex>

#include "Assembler.h"
#include "Engine.h"
#include "Scanner.h"
#include "Token.h"

#include "Stringutils.h"

#define IS_REG(x) ((x == REG_MAIN) || (x == REG_FN) || (x == REG_SCRATCH))


static int _ttSizes[] = {1, 	// REG_MAIN			r...
						 2, 	// REG_FN			fn...
						 1, 	// REG_SCRATCH		s...
						 0, 	// ABSOLUTE			...
						 1};	// IMMEDIATE		#...
				

/****************************************************************************\
|* Constructor
\****************************************************************************/
Scanner::Scanner(String src)
		  :_src(src)
		  ,_at(0)
		  ,_engine(Engine::getInstance())
	{
	reset();
	}
	
/*****************************************************************************\
|* Reset the scanner
\*****************************************************************************/
void Scanner::reset(int to)
	{
	_current		= 0;
	_at 			= to;
	_pageIndex[0] 	= 0;
	_pageIndex[1] 	= 0;
	_pageIndex[2] 	= 0;
	_pageIndex[3] 	= 0;
	_listing		= "";
	}
	
/*****************************************************************************\
|* Scan for tokens
\*****************************************************************************/
int Scanner::scan(TokenList &tokens, int& line, int pass)
	{
	int state = SCAN_MORE;
	
	String s = _nextLine(line, state);
	s		 = trim(s.substr(0, s.find(";")));
	
	/*************************************************************************\
    |* If the string is empty, just return
    \*************************************************************************/
	if (s.length() == 0)
		return state;
		
	/*************************************************************************\
    |* Check for labels, and if one is there, find, process and remove
    \*************************************************************************/
    Token t = _hasLabel(s);
    if (t.type() != T_NONE)
		_emit(tokens, line, t, A_NONE);
	
	/*************************************************************************\
    |* Look for a lone = sign
    \*************************************************************************/
	bool foundEquate 	= true;
	size_t where 		= s.find('=');
	if (where == std::string::npos)
		foundEquate = false;
	if (s[where-1] == '!')
		foundEquate = false;
	if (s[where+1] == '=')
		foundEquate = false;
		
	/*************************************************************************\
    |* If there's a lone '=' sign in the line, it's an equate, so..
    \*************************************************************************/
	if (foundEquate)
		{
		// Split into two terms either side of the =
		StringList terms = split(s, '=');
		if (terms.size() != 2)
			FATAL(ERR_EQUATE, "Cannot parse '%s' at line %d\n", s.c_str(), line);
		String identifier = terms[0];
		String expression = terms[1];
		_engine.eval(s);
		}
			
	/*************************************************************************\
    |* Otherwise get the next word and determine what to do based on that
    \*************************************************************************/
	else if (s.length() > 0)
		{
		int wordState 				= SCAN_MORE;
		String word 				= _firstWord(s, wordState);
		String lword 				= lcase(word);
		String args					= trim(s.substr(word.length()));
		Token::TokenInfo info 	= Token::parsePrefix(lword);
		
		switch (info.type)
			{
			case T_DIRECTIVE:
				{
				switch (info.which)
					{
					case P_IF:
						_if(tokens, line, args);
						break;
					
					case P_ELSE:
						_else(tokens, line);
						break;
					
					case P_ENDIF:
						_endif(tokens, line);
						break;
					
					case P_BYTE:
						_reserveBytes(tokens, line, 1, args);
						break;
					
					case P_WORD:
						_reserveBytes(tokens, line, 2, args);
						break;
					
					case P_ADDR:
						_embedAddress(tokens, line, args);
						break;
					
					case P_ORG:
						_setOrigin(tokens, line, args);
						break;
				
					case P_INCLUDE:
						break;
						
					default:
						FATAL(ERR_PARSE, "Unknown assembly directive at %d", line);
					}
				break;
				}
			case T_6502:
				if (shouldEvaluate())
					_handle6502(info, args, tokens, line, pass);
				break;
			
			case T_META:
				{
				int extent  = (word.ends_with(".1")) ? 1
							: (word.ends_with(".2")) ? 2
							: 4;
							
				_handleMeta(info, extent, args, tokens, line, pass);
				break;
				}
				
			default:
				if (shouldEvaluate())
					{
					if (_macros.find(word) != _macros.end())
						{
						_macro(tokens, line, _macros[word], args);
						}
					else
						FATAL(ERR_PARSE, "Unknown assembly token %s at %d",
										word.c_str(), line);
					}
			}
		}
	return state;
	}


/*****************************************************************************\
|* Add symbols to the engine
\*****************************************************************************/
void Scanner::insertSymbols(StringList symbols)
	{
	for (String symbol : symbols)
		{
		StringList kv = split(trim(symbol), "=");
		if (kv.size() == 2)
			{
			_engine.eval(kv[1]);
			_engine.updateSymbol(kv[0],_engine.result());
			}
		}
	}

/*****************************************************************************\
|* Add macros to the scanner
\*****************************************************************************/
void Scanner::insertMacros(MacroMap macros)
	{
	_macros = macros;
	}
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Handle macros
\*****************************************************************************/
int Scanner::_macro(TokenList &tokens, int &line, Macro macro, String argstr)
	{
	int ok = 0;
	
	StringList args = split(argstr, ',');
	if (args.size() == macro.args())
		{
		String content = "";
		for (String s : macro.lines())
			{
			for (int i=0; i<macro.args(); i++)
				{
				char buf[1024];
				snprintf(buf, 1024, "%%%d", i+1);
				s = replace(s, buf ,args[i]);
				}
			content += s + "\n";
			}
		_src.insert(_at, content);
		}
	else
		FATAL(ERR_MACRO, "Incorrect number of arguments to %s at line %d",
				macro.name().c_str(), line);
	
	return ok;
	}
	
/*****************************************************************************\
|* Handle any meta-call opcodes
\*****************************************************************************/
int Scanner::_handleMeta(Token::TokenInfo info,
						   int extent,
						   String args,
						   TokenList &tokens,
						   int& line,
						   int pass)
	{
	int ok = 0;
	
	switch (info.which)
		{
		case P_MOVE:
			ok = _handleMove(info, extent, args, tokens, line, pass);
			break;
		
		default:
			FATAL(ERR_META, "Don't understand meta-call at line %d", line);
		}
		
	return ok;
	}

	
/*****************************************************************************\
|* Determine a meta target-type and value
\*****************************************************************************/
Scanner::TargetType Scanner::_determineTarget(String s, int64_t &val, int line)
	{
	TargetType type = (s.starts_with("r")) 		? REG_MAIN
					: (s.starts_with("f"))		? REG_FN
					: (s.starts_with("s"))		? REG_SCRATCH
					: (s.starts_with("#"))  	? IMMEDIATE
												: ABSOLUTE;
	
	_engine.eval(s.substr(_ttSizes[type]));
	val = _engine.result();
	
	if (val > 0)
		{
		if ((type == REG_MAIN) && (val >= 1024))
			FATAL(ERR_META, "Main reg %lld OUB at line %d", val, line);
		if ((type == REG_FN) && (val >= 4))
			FATAL(ERR_META, "FN reg %lld OUB at line %d", val, line);
		if ((type == REG_SCRATCH) && (val >= 4))
			FATAL(ERR_META, "Scratch reg %lld OUB at line %d", val, line);
		}
	
	return type;
	}
	
/*****************************************************************************\
|* Handle the meta 'move' opcode. This is overloaded several ways, and can be
|* of the form:
|*
|*	move.4	r0		$aaaa
|*	move.2	$aaaa	$bbbb
|*	move.1	$aa		r1
|*	move.4	#$342	s0
|*
|* where:
|*		.x defines the width of the move,
|*		arguments can be registers, (r*, fn*, s*), immediate values (#*) or
|*			*absolute addresses
|*
\*****************************************************************************/
int Scanner::_handleMove(Token::TokenInfo info,
						   int extent,
						   String argString,
						   TokenList &tokens,
						   int& line,
						   int pass)
	{
	int ok 			= 0;
	StringList args = split(argString, " \t");
	if (args.size() != 2)
		FATAL(ERR_META, "Illegal meta-construct at line %d", line);
		
	/*************************************************************************\
	|* Determine src target-type and value
	\*************************************************************************/
	int64_t v1, v2;
	String arg1 	= trim(args[0]);
	TargetType t1	= _determineTarget(arg1, v1, line);
		
	/*************************************************************************\
	|* Determine dst target-type and value
	\*************************************************************************/
	String arg2 	= trim(args[1]);
	TargetType t2	= _determineTarget(arg2, v2, line);
	if (t2 == IMMEDIATE)
		FATAL(ERR_META, "Cannot move to immediate value on line %d", line);
	
	/*************************************************************************\
	|* If the first argument is a main register, make sure the bank is set up
	\*************************************************************************/
	Token::TokenInfo opInfo;
	int bank1	= (v1 % 16) / 4;	// 0 -> $80,	1 -> $81
	int bank2 	= (v2 % 16) / 4;	// 2 -> $82, 	3 -> $83
	int page1	= (int) v1 / 16;	// r0 -> 0,		r17 -> 1
	int page2	= (int) v2 / 16;	// r1 -> 0, 	r18	-> 1
	
	if (t1 == REG_MAIN)
		{
		if ((bank1 == bank2) && (page1 != page2))
			FATAL(ERR_META, "Overlapping banks for src and dst, line %d", line);
	
        /*********************************************************************\
        |* If we need to change banking 1, do so
        \*********************************************************************/
		if (page1 != _pageIndex[bank1])
			{
			String arg	= "#" + std::to_string(page1);
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, line, pass);
			
			arg			= toHexString(PAGEIDX0+bank1, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, line, pass);
			
			_pageIndex[bank1] = page1;
			}
		}
		
	/*************************************************************************\
	|* If the second argument is a main register, make sure the bank is set up
	\*************************************************************************/
	if (t2 == REG_MAIN)
		{
		if ((bank1 == bank2) && (page1 != page2))
			FATAL(ERR_META, "Overlapping banks for src and dst, line %d", line);

        /*********************************************************************\
        |* If we need to change banking 2, do so
        \*********************************************************************/
		if (page2 != _pageIndex[bank2])
			{
			String arg	= "#" + std::to_string(page2);
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, line, pass);
			
			arg			= toHexString(PAGEIDX0+bank2, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, line, pass);
			
			_pageIndex[bank2] = page2;
			}
		}

	/*************************************************************************\
	|* Handle register -> register move
	\*************************************************************************/
	if (IS_REG(t1) && IS_REG(t2))
		{
        /*********************************************************************\
        |* Copy the correct number of bytes across. Bytes are right-justified
        |* in the 4-byte range
        \*********************************************************************/
		int base1	= (t1 == REG_MAIN)	? REGMAIN_BASE
					: (t1 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
		int base2	= (t2 == REG_MAIN)	? REGMAIN_BASE
					: (t2 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
					
		int addr1	= base1 + ((v1 % 16) * 4) + 3;
		int addr2	= base2 + ((v2 % 16) * 4) + 3;
		for (int i=0; i<extent; i++)
			{
			String arg	= toHexString(addr1-i, "$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, line, pass);

			arg			= toHexString(addr2-i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, line, pass);
			}
		}

	/*************************************************************************\
	|* Handle register -> memory move
	\*************************************************************************/
	else if (IS_REG(t1) && (t2 == ABSOLUTE))
		{
		int base1	= (t1 == REG_MAIN)	? REGMAIN_BASE
					: (t1 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
	
		_engine.eval(arg2);
		int addr1	= base1 + ((v1 % 16) * 4) + 3;
		int addr2 	= (int) _engine.result();
		
		for (int i=0; i<extent; i++)
			{
			String arg	= toHexString(addr1-i, "$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, line, pass);

			arg			= toHexString(addr2+extent-1-i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, line, pass);
			}
		}

	/*************************************************************************\
	|* Handle immediate -> register move
	\*************************************************************************/
	else if ((t1 == IMMEDIATE) && (IS_REG(t2)))
		{
		_engine.eval(arg1.substr(1));		// Trim off the preceding #
		int64_t value	= _engine.result();
		
		int base2		= (t2 == REG_MAIN)	? REGMAIN_BASE
						: (t2 == REG_FN)	? REGFN_BASE
						:					  REGSCRATCH_BASE;
	
		int addr2	= base2 + ((v2 % 16) * 4) + 3;
		
		for (int i=0; i<extent; i++)
			{
			int v		= value & 0xFF;
			value		= value >> 8;
			String arg	= toHexString(v, "#$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, line, pass);

			arg			= toHexString(addr2-i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, line, pass);
			}
		}

	/*************************************************************************\
	|* Handle memory -> register move
	\*************************************************************************/
	else if ((t1 == ABSOLUTE) && (IS_REG(t2)))
		{
		int base2	= (t2 == REG_MAIN)	? REGMAIN_BASE
					: (t2 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
	
		_engine.eval(arg1);
		int addr1 	= (int) _engine.result();
		int addr2	= base2 + ((v2 % 16) * 4) + 3;
		
		for (int i=0; i<extent; i++)
			{
			String arg	= toHexString(addr1+extent-1-i, "$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, line, pass);

			arg			= toHexString(addr2-i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, line, pass);
			}
		}

	/*************************************************************************\
	|* Handle memory -> memory move
	\*************************************************************************/
	else if ((t1 == ABSOLUTE) && (t2 == ABSOLUTE))
		{
		_engine.eval(arg1);
		int addr1 	= (int) _engine.result();
		
		_engine.eval(arg2);
		int addr2	= (int) _engine.result();
		
		for (int i=0; i<extent; i++)
			{
			String arg	= toHexString(addr1+extent-1-i, "$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, line, pass);

			arg			= toHexString(addr2+extent-1-i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, line, pass);
			}
		}
	
	else
		FATAL(ERR_META, "Illegal meta operation, line %d", line);
		
	return ok;
	}
	
/*****************************************************************************\
|* Handle any 6502-native opcode
\*****************************************************************************/
int Scanner::_handle6502(Token::TokenInfo info,
						   String args,
						   TokenList &tokens,
						   int& line,
						   int pass)
	{
	Engine& e 		= Engine::getInstance();
	bool isBranch	= 	(info.which >= P_BCC)
					&&	(info.which <= P_BVS);
	
	uint8_t bytes[2];		// Bytes for operands
	
	
	/************************************************************************\
    |* Remove whitespace
    \************************************************************************/
	args.erase(remove_if(args.begin(), args.end(), ::isspace), args.end());

	/************************************************************************\
    |* Figure out the addressing mode
    \************************************************************************/
	AddressingMode amode = A_NONE;
	if (args.length() == 0)
		amode = A_IMPLIED;
	else if (args[0] == '#')
		{
		amode = A_IMMEDIATE;
		e.eval(args.substr(1));
		bytes[0] = e.result() & 0xFF;
		}
	else if ((::tolower(args[0] == 'a')) && args.length() == 1)
		amode = A_ACCUMULATOR;
	else if (args.find(',') == std::string::npos)
		{
		// We have no comma, so it's absolute, relative, or zero-page
		// Need to evaluate the expression to see how large it is
		if (isBranch)
			{
			amode = A_RELATIVE;
			e.eval("( "+args+" ) - "+ std::to_string(_current));
			int offset = (int) e.result();
			if (offset != 0)
				offset -= 2;
			bytes[0] = (uint32_t)(offset) & 0xFF;
			}
		else
			{
			e.eval(args);
			if (e.result() > 0xFF)
				{
				amode = A_ABSOLUTE;
				bytes[0] = (e.result())		 & 0xFF;
				bytes[1] = (e.result() >> 8) & 0xFF;
				}
			else
				{
				amode = A_ZEROPAGE;
				bytes[0] = e.result() & 0xFF;
				}
			}
		}
	else if (args[0] == '(')
		{
		// Handle indirect addressing
		if (endsWith(args, ",x)", false))
			{
			e.eval(args.substr(1, args.length()-4));
			bytes[0] 	= e.result() & 0xFF;
			amode 		= A_XINDEX_INDIRECT;
			}
		else if (endsWith(args, "),y", false))
			{
			e.eval(args.substr(1, args.length()-4));
			bytes[0] 	= e.result() & 0xFF;
			amode 		= A_INDIRECT_YINDEX;
			}
		else
			{
			e.eval(args.substr(1, args.length()-2));
			bytes[0] 	= (e.result())	    & 0xFF;
			bytes[1] 	= (e.result() >> 8) & 0xFF;
			amode 		= A_INDIRECT;
			}
		}
	else if (endsWith(args, ",x", false))
		{
		String arg = args.substr(0, args.length()-2);
		e.eval(arg);
		if (e.result() <= 0xFF)
			{
			bytes[0] 	= e.result() & 0xFF;
			amode 		= A_ZEROPAGE_XINDEX;
			}
		else
			{
			bytes[0] 	= (e.result())		& 0xFF;
			bytes[1] 	= (e.result() >> 8) & 0xFF;
			amode 		= A_ABSOLUTE_XINDEX;
			}
		}
	else if (endsWith(args, ",y", false))
		{
		String arg = args.substr(0, args.length()-2);
		e.eval(arg);
		if (e.result() <= 0xFF)
			{
			bytes[0] 	= e.result() & 0xFF;
			amode = A_ZEROPAGE_YINDEX;
			}
		else
			{
			bytes[0] 	= (e.result())		& 0xFF;
			bytes[1] 	= (e.result() >> 8) & 0xFF;
			amode = A_ABSOLUTE_YINDEX;
			}
		}
	else
		{
		FATAL(ERR_PARSE, "Unknown addressing mode '%s'", args.c_str());
		}
	
	/************************************************************************\
    |* Check that the addressing mode is legal for this opcode
    \************************************************************************/
    int opcode = Token::opcode(info.which, amode);
    if ((pass == 2) && (opcode < 0))
		FATAL(ERR_PARSE, "Illegal addressing mode '%s' for %s on line %d",
				args.c_str(), info.name.c_str(), line);
	
	/************************************************************************\
    |* Create a token to represent the opcode and add it to the stream
    \************************************************************************/
	Token t(T_6502, info.which);
	t.setOpcode(opcode);
	int numbytes = 1;
	switch (amode)
		{
		case A_IMPLIED:
		case A_ACCUMULATOR:
			break;
			
		case A_ABSOLUTE:
		case A_ABSOLUTE_XINDEX:
		case A_ABSOLUTE_YINDEX:
		case A_INDIRECT:
			t.setOp1(bytes[0]);
			t.setOp2(bytes[1]);
			numbytes +=2;
			break;
	
		case A_RELATIVE:
			{
			//  Find the name of the label in this string
			const char *str 	= args.c_str();
			const char * bar 	= strstr(str, "_?");
			const char * stt	= bar;
			if (stt != NULL)
				{
				while ((stt > str) && (!::isspace(*stt)))
					stt --;
				
				String lName 	= String(stt,bar-stt);
				
				String labelId 	= e.nextLabel(lName);
				String oldName	= lName + "_?";
				str 			= replace(args, oldName,  labelId).c_str();
				}

			t.setArg1(str);
			t.setOp1(bytes[0]);
			numbytes += 1;
			break;
			}
		default:
			t.setOp1(bytes[0]);
			numbytes += 1;
		}
	t.setBytes(numbytes);
	
	/************************************************************************\
    |* Bump the current address
    \************************************************************************/
	if (_emit(tokens, line, t, amode))
		_current += numbytes;
	
	return 0;
	}
	
/*****************************************************************************\
|* Set the assembly origin by inserting a token into the stream
\*****************************************************************************/
int Scanner::_setOrigin(TokenList &tokens, int& line, String args)
	{
	Token t(T_DIRECTIVE, P_ORG);
	_engine.eval(args);
	_current = _engine.result() & 0xFFFF;
	t.setAddr(_current);
	_emit(tokens, line, t, A_NONE);
	return 0;
	}

/*****************************************************************************\
|* Process if blocks
\*****************************************************************************/
int Scanner::_if(TokenList &tokens, int& line, String condition)
	{
	// Evaluate the condition
	_engine.eval(condition);
	if (_engine.result())
		_ifState.push_back(true);
	else
		_ifState.push_back(false);
		
	return 0;
	}
	
/*****************************************************************************\
|* Process else blocks. This just switches the most-recent state of the
|* current if-block. That will prevent output if it was previously enabled,
|* and allow output if it was previously prevented
\*****************************************************************************/
int Scanner::_else(TokenList &tokens, int& line)
	{
	if (_ifState.size() > 0)
		{
		bool state = _ifState.back();
		_ifState.pop_back();
		_ifState.push_back(!state);
		}
	else
		FATAL(ERR_IF, "Found an ELSE without IF at line %d", line);
		
	return 0;
	}
	
/*****************************************************************************\
|* Process endif blocks
\*****************************************************************************/
int Scanner::_endif(TokenList &tokens, int& line)
	{
	if (_ifState.size() > 0)
		_ifState.pop_back();
	else
		FATAL(ERR_IF, "Found an ENDIF without IF at line %d", line);
		
	return 0;
	}

/*****************************************************************************\
|* Reserve space for the data in the arguments
\*****************************************************************************/
int Scanner::_reserveBytes(TokenList &tokens, int& line, int size, String args)
	{
	Token t(T_DIRECTIVE, P_BYTE);

	/*************************************************************************\
    |* Split into comma-separated words, respecting "," and ','
    \*************************************************************************/
	String word = "";
	int at		= 0;
	int len		= (int) args.length();

	int  inQuote = 0;
	while (at <= len)
		{
		if (!inQuote)
			{
			if (args[at] == '"')
				inQuote = '"';
			else if (args[at] == '\'')
				inQuote = '\'';
			else if ((args[at] == ',') || (at == len))
				{
				if (word.length() > 0)
					{
					int val = 0;
					
					if (word.find_first_not_of("0123456789") == std::string::npos)
						{
						val = std::stoi(word);
						}
					else
						{
						_engine.eval(word);
						val	= (int) _engine.result();
						}
					if (size == 2)
						{
						t.data().push_back(val & 0xFF);
						t.data().push_back((val >> 8) & 0xFF);
						}
					else
						{
						t.data().push_back(val & 0xFF);
						}
					}
				}
			else
				{
				word += args[at];
				}
			}
		else if (inQuote)
			{
			if ((args[at] == '"') && (inQuote == '"'))
				{
				inQuote = 0;
				}
			else if ((args[at] == '\'') && (inQuote == '\''))
				{
				inQuote = 0;
				}
			else
				t.data().push_back(args[at]);
			}
		at ++;
		}
	
	_emit(tokens, line, t, A_NONE);
	return 0;
	}

/*****************************************************************************\
|* Embed an address into the byte stream
\*****************************************************************************/
int Scanner::_embedAddress(TokenList &tokens, int& line, String args)
	{
	// FIXME - needs implementation
	return 0;
	}
	
/*****************************************************************************\
|* Does this line start with a label
\*****************************************************************************/
Token Scanner::_hasLabel(String& s)
	{
	Engine& e = Engine::getInstance();
	
	Token t;
	String label = "";
	
	int idx = 0;
	while ((idx < s.length()) && (!::isspace(s[idx])))
		{
		label += s[idx];
		idx ++;
		}
	
	/*************************************************************************\
    |* See if this is a global or local label
    \*************************************************************************/
	bool global = (label.back() == ':');
	bool local	= (label.back() == '.');
	label.pop_back();
	
	bool isLabel = global | local;
	if (isLabel)
		{
        /*********************************************************************\
        |* Is this a dynamically-named label ?
        \*********************************************************************/
		if (label.back() == '?')
			{
			label.pop_back();
			label.pop_back();
			String name = label;
			
			e.incLabel(name);
			label = e.nextLabel(label);
			}
			
		if (global)
			{
			t.setType(T_LABEL);
			t.setWhich(P_LABEL);
			_majorLabel = label;
			t.setArg1(label);
			s = trim(s.substr(idx));
			e.updateSymbol(label, _current);
			}
		else if (local)
			{
			t.setType(T_LLABEL);
			t.setWhich(P_LLABEL);
			String labelName = _majorLabel+"_"+label;
			e.updateSymbol(labelName, _current);
			t.setArg1(labelName);
			t.setArg2(label);
			s = trim(s.substr(idx));
			}
		}
		
	return t;
	}
	
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
|* Get the next character in the input stream
\*****************************************************************************/
String Scanner::_firstWord(String src, int &state)
	{
	String 		s = "";
	state	 	= SCAN_MORE;
	int at		= 0;
	
	int c = src[at++];
	
	while (at < src.length() && (::isspace(c)))
		c = src[at++];
	
	while (at <= src.length())
		{
		if (::isspace(c))
			return s;
		
		s += (char)c;
		c = src[at++];
		}
		
	state = SCAN_COMPLETE;
	return s;
	}

/*****************************************************************************\
|* Get the next line in the input stream
\*****************************************************************************/
String Scanner::_nextLine(int &line, int &state)
	{
	String s = "";
	state	 = SCAN_MORE;
	
	while (!_atEnd())
		{
		int c = _src[_at++];
		if (c == '\n')
			{
			line ++;
			return s;
			}
		s += (char)c;
		}
		
	state = SCAN_COMPLETE;
	return s;
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
|* Emit a token
\*****************************************************************************/
bool Scanner::_emit(TokenList &tokens,
					  int& line,
					  Token token,
					  AddressingMode amode)
	{
	bool emit = shouldEvaluate();
	if (emit)
		{
		tokens.push_back(token);
		token.setAddrMode(amode);
		_listing += token.toString()+"\n";
		}
	
	return emit;
	}
	
	
/*****************************************************************************\
|* Detect if we should evaluate a statement
\*****************************************************************************/
bool Scanner::shouldEvaluate(void)
	{
	bool evaluate = true;
	
	if (_ifState.size())
		for (bool flag : _ifState)
			{
			evaluate = evaluate & flag;
			if (!evaluate)
				break;
			}
	return evaluate;
	}