//
//  Scanner.cc
//  as8
//
//  Created by Thrud The Barbarian on 10/28/22.
//
#include <cstdlib>
#include <regex>
#include <iomanip>
#include <sstream>

#include "Assembler.h"
#include "ContextMgr.h"
#include "Engine.h"
#include "Scanner.h"
#include "Token.h"

#include "Stringutils.h"

#define IS_REG(x) 				((x == REG_MAIN) || \
								 (x == REG_FN)   || \
								 (x == REG_SCRATCH))
#define MAX_REGISTERS			(4*4*256)
#define REG_BASE				(0xC0)

#define OVERFLOW_LOCATION		0x89
static int _ttSizes[] = {1, 	// REG_MAIN			r...
						 2, 	// REG_FN			fn...
						 1, 	// REG_SCRATCH		s...
						 0, 	// ABSOLUTE			...
						 1};	// IMMEDIATE		#...
						 
typedef struct
	{
	int idx;
	uint8_t byte;
	} IndexedByte;

/****************************************************************************\
|* Constructor
\****************************************************************************/
Scanner::Scanner(String src)
		  :_src(src)
		  ,_at(0)
		  ,_engine(Engine::getInstance())
		  ,_showListing(false)
	{
	reset();
	}
	
/*****************************************************************************\
|* Reset the scanner
\*****************************************************************************/
void Scanner::reset(int to)
	{
	_labelId		= 0;
	_current		= 0;
	_at 			= to;
	_pageIndex[0] 	= 0;
	_pageIndex[1] 	= 0;
	_pageIndex[2] 	= 0;
	_pageIndex[3] 	= 0;
	_listing		= "";
	
	_regSize.clear();
	_regSigned.clear();
	
	ContextMgr::sharedInstance()->reset();
	
	for (Elements<String, Function> kv : _functions)
		{
		kv.value.setEmitted(false);
		kv.value.setUsed(false);
		}
	}
	
/*****************************************************************************\
|* Scan for tokens
\*****************************************************************************/
int Scanner::scan(TokenList &tokens, int pass)
	{
	int state 		= SCAN_MORE;
	
	String s		= _nextLine(state);
	_currentLine	= s;
	s		 		= trim(s.substr(0, s.find(";")));
	String lc		= lcase(s);
	
	/*************************************************************************\
    |* If the string is empty, just return
    \*************************************************************************/
	if (s.length() == 0)
		return state;
	
	/*************************************************************************\
    |* Insert a pseudo-token for the line-content
    \*************************************************************************/
	if (_showListing && (pass == 2))
		{
		fprintf(stderr, "; [%s]\n", trim(s).c_str());
		}
		
	/*************************************************************************\
    |* Check for labels, and if one is there, find, process and remove
    \*************************************************************************/
    Token t = _hasLabel(s);
    if (t.type() != T_NONE)
		_emit(tokens, t, A_NONE);
	
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
			FATAL(ERR_EQUATE, "Cannot parse '%s'\n%s",
				s.c_str(), CTXMGR->location().c_str());
		String identifier = terms[0];
		String expression = terms[1];
		_engine.eval(s);
		}
			
	/*************************************************************************\
    |* Check to see if we're pushing a context onto the stack
    \*************************************************************************/
	else if (lc.starts_with(".push context"))
		{
		CTXMGR->incLine(-1);	// This isn't a real line so correct the sum
		_pushContext(s);
		}
		
	/*************************************************************************\
    |* Or popping one
    \*************************************************************************/
	else if (lc.starts_with(".pop context"))
		{
		CTXMGR->pop();
		CTXMGR->incLine(-1);	// This isn't a real line so correct the sum
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
		Token::TokenInfo info 		= Token::parsePrefix(lword);
		
		switch (info.type)
			{
			case T_DIRECTIVE:
				{
				switch (info.which)
					{
					case P_IF:
						_if(tokens, args);
						break;
					
					case P_ELSE:
						_else(tokens);
						break;
					
					case P_ENDIF:
						_endif(tokens);
						break;
					
					case P_BYTE:
						_reserveBytes(tokens, 1, args);
						break;
					
					case P_WORD:
						_reserveBytes(tokens, 2, args);
						break;
					
					case P_ADDR:
						_embedAddress(tokens, args);
						break;
					
					case P_ORG:
						_setOrigin(tokens, args);
						break;
				
					case P_INCLUDE:
						break;
						
					default:
						FATAL(ERR_PARSE, "Unknown assembly directive\n%s",
							 CTXMGR->location().c_str());
					}
				break;
				}
			case T_6502:
				if (shouldEvaluate())
					_handle6502(info, args, tokens, pass);
				break;
			
			case T_META:
				{
				int extent  = (word.ends_with(".1")) ? 1
							: (word.ends_with(".2")) ? 2
							: 4;
							
				_handleMeta(word, info, extent, args, tokens, pass);
				break;
				}
				
			default:
				if (shouldEvaluate())
					{
					if (_macros.find(word) != _macros.end())
						{
						_macro(tokens, _macros[word], args);
						}
					else
						FATAL(ERR_PARSE, "Unknown assembly token '%s'\n%s",
								word.c_str(),
								CTXMGR->location().c_str());
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
			String value = trim(kv[1]);
			if (value.starts_with("0x"))
				value = "$" + value.substr(2);
				
			_engine.eval(value);
			_engine.updateSymbol(kv[0],_engine.result());
			}
		}
	}

/*****************************************************************************\
|* Append any used functions to the end of the source
\*****************************************************************************/
bool Scanner::appendUsedFunctions(void)
	{
	bool emitted = false;
	
	for (Elements<String,Function> kv : _functions)
		{
		if (kv.value.used() && (!kv.value.emitted()))
			{
			kv.value.setEmitted(true);
			emitted = true;
			
			_src.append("; Function definition for " + kv.key + "\n");
			_src.append(join(kv.value.lines(), "\n"));
			_src.append("\n\n");
			
			//fprintf(stderr, "Function : %s\n------\n%s\n------\n\n\n",
			//	kv.key.c_str(),
			//	join(kv.value.lines(), "\n").c_str());
			}
		}
		
	return emitted;
	}
	
#pragma mark - Private Methods

/*****************************************************************************\
|* Handle macros
\*****************************************************************************/
int Scanner::_macro(TokenList &tokens, Macro macro, String argstr)
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
		FATAL(ERR_MACRO, "Incorrect number of arguments to %s\n%s",
				macro.name().c_str(),
				CTXMGR->location().c_str());
	
	return ok;
	}
	
/*****************************************************************************\
|* Handle any meta-call opcodes
\*****************************************************************************/
int Scanner::_handleMeta(String word,
						 Token::TokenInfo info,
						 int extent,
						 String args,
						 TokenList &tokens,
						 int pass)
	{
	int ok = 0;
	
	switch (info.which)
		{
		case P_MOVE:
			ok = _handleMove(info, extent, args, tokens, pass);
			break;
		
		case P_MUL:
			ok = _handleMath(word, info, extent, args, tokens, pass, "mul");
			break;
		
		case P_ADD:
			ok = _handleMath(word, info, extent, args, tokens, pass, "add");
			break;
		
		case P_SUB:
			ok = _handleMath(word, info, extent, args, tokens, pass, "sub");
			break;
		
		case P_DIV:
			ok = _handleMath(word, info, extent, args, tokens, pass, "div");
			break;
		
		case P_CALL:
			ok = _handleCall(info, extent, args, tokens, pass, true);
			break;
		
		case P_EXEC:
			ok = _handleCall(info, extent, args, tokens, pass, false);
			break;
		
		default:
			FATAL(ERR_META, "Don't understand meta-call \n%s",
				CTXMGR->location().c_str());
		}
		
	return ok;
	}

	
/*****************************************************************************\
|* Determine a meta target-type and value
\*****************************************************************************/
Scanner::TargetType Scanner::_determineTarget(String s, int64_t &val)
	{
	TargetType type = (s.starts_with("r")) 		? REG_MAIN
					: (s.starts_with("f"))		? REG_FN
					: (s.starts_with("s"))		? REG_SCRATCH
					: (s.starts_with("#"))  	? IMMEDIATE
												: ABSOLUTE;
	String expr = s.substr(_ttSizes[type]);
	val = 0;
	if (expr.length() > 0)
		{
		_engine.eval(expr);
		val = _engine.result();
		}
		
	if (val > 0)
		{
		if ((type == REG_MAIN) && (val >= 1024))
			FATAL(ERR_META, "Main reg %lld OUB\n%s",
				val, CTXMGR->location().c_str());
		if ((type == REG_FN) && (val >= 4))
			FATAL(ERR_META, "FN reg %lld OUB\n%s",
				val, CTXMGR->location().c_str());
		if ((type == REG_SCRATCH) && (val >= 4))
			FATAL(ERR_META, "Scratch reg %lld OUB\n%s",
				val, CTXMGR->location().c_str());
		}
	
	return type;
	}

/*****************************************************************************\
|* Helper function to sort the indexed bytes
\*****************************************************************************/
static int _compareIndexedByte(const void *v1, const void *v2)
	{
	const IndexedByte *b1 = (const IndexedByte *)v1;
	const IndexedByte *b2 = (const IndexedByte *)v2;
	
	if (b1->byte == b2->byte)
		return 0;
	if (b1->byte > b2->byte)
		return 1;
	return -1;
	}

/*****************************************************************************\
|* Helper method to surface the correct registers
\*****************************************************************************/
void Scanner::_surfaceRegs(TargetType t1,
						   TargetType t2,
						   int64_t v1,
						   int64_t v2,
						   TokenList &tokens,
						   int pass)
	{
	Token::TokenInfo opInfo;
	int bank1	= (v1 % 16) / 4;	// 0 -> $80,	1 -> $81
	int bank2 	= (v2 % 16) / 4;	// 2 -> $82, 	3 -> $83
	int page1	= (int) v1 / 16;	// r0 -> 0,		r17 -> 1
	int page2	= (int) v2 / 16;	// r1 -> 0, 	r18	-> 1
	
	if (t1 == REG_MAIN)
		{
		if ((t2 == REG_MAIN) && (bank1 == bank2) && (page1 != page2))
			FATAL(ERR_META, "Overlapping banks for src and dst\n%s",
				CTXMGR->location().c_str());
	
        /*********************************************************************\
        |* If we need to change banking 1, do so
        \*********************************************************************/
		if (page1 != _pageIndex[bank1])
			{
			String arg	= "#" + std::to_string(page1);
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);
			
			arg			= toHexString(PAGEIDX0+bank1, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			
			_pageIndex[bank1] = page1;
			}
		}

	/*************************************************************************\
	|* If the second argument is a main register, make sure the bank is set up
	\*************************************************************************/
	if (t2 == REG_MAIN)
		{
		if ((t1 == REG_MAIN) && (bank1 == bank2) && (page1 != page2))
			FATAL(ERR_META, "Overlapping banks for src and dst\n%s",
				CTXMGR->location().c_str());

        /*********************************************************************\
        |* If we need to change banking 2, do so
        \*********************************************************************/
		if (page2 != _pageIndex[bank2])
			{
			String arg	= "#" + std::to_string(page2);
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);
			
			arg			= toHexString(PAGEIDX0+bank2, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			
			_pageIndex[bank2] = page2;
			}
		}
	}

/*****************************************************************************\
|* Helper method to surface the correct registers
\*****************************************************************************/
void Scanner::_surfaceRegs3(TargetType t1,
						    TargetType t2,
						    TargetType t3,
						    int64_t v1,
						    int64_t v2,
						    int64_t v3,
						    TokenList &tokens,
						    int pass)
	{
	Token::TokenInfo opInfo;
	int bank1	= (v1 % 16) / 4;	// 0 -> $80,	1 -> $81
	int bank2 	= (v2 % 16) / 4;	// 2 -> $82, 	3 -> $83
	int bank3 	= (v3 % 16) / 4;	// 2 -> $82, 	3 -> $83
	int page1	= (int) v1 / 16;	// r0 -> 0,		r17 -> 1
	int page2	= (int) v2 / 16;	// r1 -> 0, 	r18	-> 1
	int page3	= (int) v3 / 16;	// r1 -> 0, 	r18	-> 1
	
	
	if (t1 == REG_MAIN)
		{
		bool bank2Err = (bank1 == bank2) && (page1 != page2);
		bool bank3Err = (bank1 == bank3) && (page1 != page3);
		if (((t2 == REG_MAIN) && bank2Err) || ((t3 == REG_MAIN) && bank3Err))
			FATAL(ERR_META, "Overlapping banks for src and dst\n%s",
				CTXMGR->location().c_str());
	
        /*********************************************************************\
        |* If we need to change banking 1, do so
        \*********************************************************************/
		if (page1 != _pageIndex[bank1])
			{
			String arg	= "#" + std::to_string(page1);
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);
			
			arg			= toHexString(PAGEIDX0+bank1, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			
			_pageIndex[bank1] = page1;
			}
		}

	/*************************************************************************\
	|* If the second argument is a main register, make sure the bank is set up
	\*************************************************************************/
	if (t2 == REG_MAIN)
		{
		bool bank1Err = (bank2 == bank1) && (page2 != page1);
		bool bank3Err = (bank2 == bank3) && (page2 != page3);
		if (((t1 == REG_MAIN) && bank1Err) || ((t3 == REG_MAIN) && bank3Err))
			FATAL(ERR_META, "Overlapping banks for src and dst\n%s",
				CTXMGR->location().c_str());

        /*********************************************************************\
        |* If we need to change banking 2, do so
        \*********************************************************************/
		if (page2 != _pageIndex[bank2])
			{
			String arg	= "#" + std::to_string(page2);
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);
			
			arg			= toHexString(PAGEIDX0+bank2, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			
			_pageIndex[bank2] = page2;
			}
		}

	/*************************************************************************\
	|* If the third argument is a main register, make sure the bank is set up
	\*************************************************************************/
	if (t3 == REG_MAIN)
		{
		bool bank1Err = (bank3 == bank1) && (page3 != page1);
		bool bank2Err = (bank3 == bank2) && (page3 != page2);
		if (((t1 == REG_MAIN) && bank1Err) || ((t2 == REG_MAIN) && bank2Err))
			FATAL(ERR_META, "Overlapping banks for src and dst\n%s",
				CTXMGR->location().c_str());

        /*********************************************************************\
        |* If we need to change banking 3, do so
        \*********************************************************************/
		if (page3 != _pageIndex[bank3])
			{
			String arg	= "#" + std::to_string(page3);
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);
			
			arg			= toHexString(PAGEIDX0+bank3, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			
			_pageIndex[bank3] = page3;
			}
		}
	}
	
/*****************************************************************************\
|* Insert a 3-register operation into the source code, so it will be acted on
|* next
\*****************************************************************************/
void Scanner::_insertOp(TargetType t1,
					    TargetType t2,
					    TargetType t3,
					    int64_t v1,
					    int64_t v2,
					    int64_t v3,
					    String stem,
					    int overflowPos
					    )
	{
	int base1		= (t1 == REG_MAIN)	? REGMAIN_BASE
					: (t1 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
	int base2		= (t2 == REG_MAIN)	? REGMAIN_BASE
					: (t2 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
	int base3		= (t3 == REG_MAIN)	? REGMAIN_BASE
					: (t3 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;

	int addr1		= base1 + ((v1 % 16) * 4);
	int addr2		= base2 + ((v2 % 16) * 4);
	int addr3		= base3 + ((v3 % 16) * 4);

	String macro	= stem;
	if (overflowPos == 1)
		macro	   += toHexString(OVERFLOW_LOCATION, ", $");
	macro		   += toHexString(addr1, " $");
	
	if (overflowPos == 2)
		macro	   += toHexString(OVERFLOW_LOCATION, ", $");
	macro		   += toHexString(addr2, ", $");

	if (overflowPos == 3)
		macro	   += toHexString(OVERFLOW_LOCATION, ", $");
	macro		   += toHexString(addr3, ", $");
	
	if (overflowPos == 4)
		macro 	   += toHexString(OVERFLOW_LOCATION, ", $");
	
	macro += "\n";
	
	_src.insert(_at, macro);
	}
	
/*****************************************************************************\
|* Handle the meta 'mul' opcode. This is either signed or unsigned, and is
|* always register to register, eg:
|*
|*	muls.4	r0		r1
|*	mulu.2	r2		r3
|*
|* where:
|*		.x defines the width of the move,
|*		arguments must be registers, (r*, fn*, s*)
|*
\*****************************************************************************/
int Scanner::_handleMath(String word,
						 Token::TokenInfo info,
						 int extent,
						 String argString,
						 TokenList &tokens,
						 int pass,
						 String op)
	{
	int ok 			= 0;
	StringList args = split(argString, " \t");
	if (args.size() != 3)
		FATAL(ERR_META, "Illegal meta syntax for '%s'\n%s",
			  op.c_str(), CTXMGR->location().c_str());
	
	/*************************************************************************\
	|* Determine src1 target-type and value
	\*************************************************************************/
	int64_t v1, v2, v3;
	String arg1 	= trim(args[0]);
	TargetType t1	= _determineTarget(arg1, v1);
	if (!IS_REG(t1))
		FATAL(ERR_META, "Argument 1 to %s must be register\n%s",
			  op.c_str(), CTXMGR->location().c_str());
		
	/*************************************************************************\
	|* Determine src2 target-type and value
	\*************************************************************************/
	String arg2 	= trim(args[1]);
	TargetType t2	= _determineTarget(arg2, v2);
	if (!IS_REG(t2))
		FATAL(ERR_META, "Argument 2 to %s must be register\n%s",
			  op.c_str(), CTXMGR->location().c_str());
		
	/*************************************************************************\
	|* Determine dst target-type and value
	\*************************************************************************/
	String arg3 	= trim(args[2]);
	TargetType t3	= _determineTarget(arg3, v3);
	if (!IS_REG(t3))
		FATAL(ERR_META, "Argument 3 to %s must be register\n%s",
			  op.c_str(), CTXMGR->location().c_str());
	
	/*************************************************************************\
	|* Infer any new register sizes
	\*************************************************************************/
	_regSize[arg1] = extent;
	_regSize[arg2] = extent;
	_regSize[arg3] = extent;

	/*************************************************************************\
	|* Make sure any registers that need banks remapped, have that happen
	\*************************************************************************/
	_surfaceRegs3(t1, t2, t3, v1, v2, v3, tokens, pass);

	/*************************************************************************\
	|* Use the appropriate macro to run the code inline
	\*************************************************************************/
	String stem 	= "_" + op;
	stem		   += std::to_string(extent*8);
	
	String lword	= lcase(word);
	String notsign	= lcase(op) + "u";
	stem 		   += (lword.starts_with(notsign)) ? "u" : "";
	
	_insertOp(t1, t2, t3, v1, v2, v3, stem, -1);

	return ok;
	}

/*****************************************************************************\
|* Handle the meta 'call' and 'exec' opcodes.
|*
|*	For call, the assembler will:
|*		- check that the function being called actually exists as a function
|*		- check what the function clobbers, and push to the stack if not f or s
|*		- if there are arguments to the call, as a convenience these will be
|*		  moved to f0..f3. Currently more than 4 args is unsupported
|*		- jsr to the global label @function-name
|*		- on return, pull any previously-pushed regs. Note that return
|*		  values from functions should be in f0..f3
|*		- carry on as you were.
|*
|*	For exec, the same happens, but no registers are saved/restored. This can
|*	prevent needless save/restore when a function calls another function and
|*	doesn't care about the register-state
|*
|* where:
|*		.x defines the width of the move,
|*		arguments can be registers, (r*, fn*, s*), immediate values (#*) or
|*			*absolute addresses
|*
\*****************************************************************************/
int Scanner::_handleCall(Token::TokenInfo info,
						   int extent,
						   String argString,
						   TokenList &tokens,
						   int pass,
						   bool isCall)
	{
	char buf[1024];
	
	int ok = 0;
	StringList assembly;
	
	StringList args = split(argString, " \t");
	if (args.size() < 1)
		FATAL(ERR_FUNCTION, "Syntax error during 'call' or 'exec'\n%s",
				CTXMGR->location().c_str());
	
	if (args.size() > 5)
		FATAL(ERR_FUNCTION, "Too many args to 'call' or 'exec'\n%s",
				CTXMGR->location().c_str());
	
	String function = args[0];
	
	/*************************************************************************\
	|* Check that the function exists
	\*************************************************************************/
	if (_functions.count(function) == 0)
		FATAL(ERR_FUNCTION, "Cannot find function '%s'\n%s",
				args[0].c_str(), CTXMGR->location().c_str());

	/*************************************************************************\
	|* Generate the push-to-stack values, if this is a 'call', but not if this
	|* is an 'exec'.
	\*************************************************************************/
	if (isCall)
		_functions[function].enstack(assembly);

	/*************************************************************************\
	|* If we have arguments then move them over to the fnx locations.
	\*************************************************************************/
	if (args.size() > 1)
		{
        /********************************************************************\
        |* Add a token into the block
        \********************************************************************/
		for (int i=1; i<args.size(); i++)
			{
			/****************************************************************\
			|* Look up the registers being passed, and see if we need to sign
			|* or zero-extend them
			\****************************************************************/
			switch (_regSize[args[i]])
				{
				case 4:
					snprintf(buf, 1024, "_xfer32 %s,f%d\n",
										args[i].c_str(), i-1);
					break;
					
				case 2:
					snprintf(buf, 1024, "_xfer16 %s,f%d\n"
										"\tlda #$0\n"
										"\tsta f%d+2\n"
										"\tsta f%d+3\n",
										args[i].c_str(), i-1, i-1, i-1);
					break;
				case 1:
					snprintf(buf, 1024, "lda %s\n"
										"\tsta f%d\n"
										"\tlda #$0\n"
										"\tsta f%d+1\n"
										"\tsta f%d+2\n"
										"\tsta f%d+3\n",
										args[i].c_str(), i-1, i-1, i-1, i-1);
					break;
				}
			assembly.push_back(buf);
			}
		}

	/*************************************************************************\
	|* Jump to the subroutine
	\*************************************************************************/
	snprintf(buf, 1024, "jsr %s\n", function.c_str());
	assembly.push_back(buf);

	/*************************************************************************\
	|* Restore any pushed values, if this is a 'call', but not if 'exec'
	\*************************************************************************/
	if (isCall)
		_functions[function].destack(assembly);

	/*************************************************************************\
	|* Insert the assembly into the current source
	\*************************************************************************/
	_src.insert(_at, join(assembly, "\n") + "\n");

	/*************************************************************************\
	|* Mark the function as used
	\*************************************************************************/
	_functions[function].setUsed(true);
	
	return ok;
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
						   int pass)
	{
	int ok 			= 0;
	StringList args = split(argString, " \t");
	if (args.size() != 2)
		FATAL(ERR_META, "Illegal meta-construct 'move'\n%s",
			CTXMGR->location().c_str());
		
	/*************************************************************************\
	|* Determine src target-type and value
	\*************************************************************************/
	int64_t v1, v2;
	String arg1 	= trim(args[0]);
	TargetType t1	= _determineTarget(arg1, v1);
		
	/*************************************************************************\
	|* Determine dst target-type and value
	\*************************************************************************/
	String arg2 	= trim(args[1]);
	TargetType t2	= _determineTarget(arg2, v2);
	if (t2 == IMMEDIATE)
		FATAL(ERR_META, "Cannot move to immediate value\n%s",
			CTXMGR->location().c_str());
	
	/*************************************************************************\
	|* Make sure any registers that need banks remapped, have that happen
	\*************************************************************************/
	_surfaceRegs(t1, t2, v1, v2, tokens, pass);
	
	/*************************************************************************\
	|* Handle register -> register move
	\*************************************************************************/
	Token::TokenInfo opInfo;
	if (IS_REG(t1) && IS_REG(t2))
		{
		// Update the current register size
		_regSize[arg1] = extent;
		_regSize[arg2] = extent;
	
        /*********************************************************************\
        |* Copy the correct number of bytes across. Bytes are left-justified
        |* in the 4-byte range
        \*********************************************************************/
		int base1	= (t1 == REG_MAIN)	? REGMAIN_BASE
					: (t1 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
		int base2	= (t2 == REG_MAIN)	? REGMAIN_BASE
					: (t2 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
					
		int addr1	= base1 + ((v1 % 16) * 4);
		int addr2	= base2 + ((v2 % 16) * 4);
		for (int i=0; i<extent; i++)
			{
			String arg	= toHexString(addr1+i, "$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);

			arg			= toHexString(addr2+i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			}
		}

	/*************************************************************************\
	|* Handle register -> memory move
	\*************************************************************************/
	else if (IS_REG(t1) && (t2 == ABSOLUTE))
		{
		// Update the current register size
		_regSize[arg1] = extent;
		
		int base1	= (t1 == REG_MAIN)	? REGMAIN_BASE
					: (t1 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
	
		_engine.eval(arg2);
		int addr1	= base1 + ((v1 % 16) * 4);
		int addr2 	= (int) _engine.result();
		
		for (int i=0; i<extent; i++)
			{
			String arg	= toHexString(addr1+i, "$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);

			arg			= toHexString(addr2+i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			}
		}

	/*************************************************************************\
	|* Handle immediate -> register move
	\*************************************************************************/
	else if ((t1 == IMMEDIATE) && (IS_REG(t2)))
		{
		// Update the current register size
		_regSize[arg2] = extent;
		
		_engine.eval(arg1.substr(1));		// Trim off the preceding #
		int64_t value	= _engine.result();
		
		int base2		= (t2 == REG_MAIN)	? REGMAIN_BASE
						: (t2 == REG_FN)	? REGFN_BASE
						:					  REGSCRATCH_BASE;
		
		int addr2	= base2 + ((v2 % 16) * 4);
		
		// To optimise stores, we want to find any reoccurrences of the
		// known byte quantity
		IndexedByte b[4];
		for (int i=0; i<extent; i++)
			{
			b[i].idx 	= i;
			b[i].byte 	= value & 0xFF;
			value 		= value >> 8;
			}
	
		::qsort(b, extent, sizeof(IndexedByte), _compareIndexedByte);
		
		
		int v = -1;
		for (int i=0; i<extent; i++)
			{
			if (v != b[i].byte)
				{
				String arg	= toHexString(b[i].byte, "#$");
				opInfo 		= Token::parsePrefix("lda");
				_handle6502(opInfo, arg, tokens, pass);
				v = b[i].byte;
				}
			
			String arg	= toHexString(addr2 + b[i].idx, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			}
		}

	/*************************************************************************\
	|* Handle memory -> register move
	\*************************************************************************/
	else if ((t1 == ABSOLUTE) && (IS_REG(t2)))
		{
		// Update the current register size
		_regSize[arg2] = extent;

		int base2	= (t2 == REG_MAIN)	? REGMAIN_BASE
					: (t2 == REG_FN)	? REGFN_BASE
					:					  REGSCRATCH_BASE;
	
		_engine.eval(arg1);
		int addr1 	= (int) _engine.result();
		int addr2	= base2 + ((v2 % 16) * 4);
		
		for (int i=0; i<extent; i++)
			{
			String arg	= toHexString(addr1+i, "$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);

			arg			= toHexString(addr2+i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
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
			String arg	= toHexString(addr1+i, "$");
			opInfo 		= Token::parsePrefix("lda");
			_handle6502(opInfo, arg, tokens, pass);

			arg			= toHexString(addr2+i, "$");
			opInfo 		= Token::parsePrefix("sta");
			_handle6502(opInfo, arg, tokens, pass);
			}
		}
	
	else
		FATAL(ERR_META, "Illegal meta operation\n%s",
			CTXMGR->location().c_str());
		
	return ok;
	}
	
/*****************************************************************************\
|* Handle any 6502-native opcode
\*****************************************************************************/
int Scanner::_handle6502(Token::TokenInfo info,
						   String args,
						   TokenList &tokens,
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
			int offset = _evaluateLabel(args) - _current;
			if (offset != 0)
				offset -= 2;
			bytes[0] = (uint32_t)(offset) & 0xFF;
			}
		else
			{
			bool isJxx = (info.which == P_JMP) || (info.which == P_JSR);
			int addr   = _evaluateLabel(args);
			if (isJxx || (addr > 0xFF))
				{
				amode 		= A_ABSOLUTE;
				bytes[0] 	= (addr	   )  & 0xFF;
				bytes[1] 	= (addr >> 8) & 0xFF;
				}
			else
				{
				amode = A_ZEROPAGE;
				bytes[0] = addr & 0xFF;
				}
			}
		}
	else if (args[0] == '(')
		{
		// Handle indirect addressing
		if (endsWith(args, ",x)", false))
			{
			int addr 	= _evaluateLabel(args.substr(1, args.length()-4));
			bytes[0] 	= addr & 0xFF;
			amode 		= A_XINDEX_INDIRECT;
			}
		else if (endsWith(args, "),y", false))
			{
			int addr 	= _evaluateLabel(args.substr(1, args.length()-4));
			bytes[0] 	= addr & 0xFF;
			amode 		= A_INDIRECT_YINDEX;
			}
		else
			{
			int addr 	= _evaluateLabel(args.substr(1, args.length()-2));
			bytes[0] 	= (addr	    ) & 0xFF;
			bytes[1] 	= (addr >> 8) & 0xFF;
			amode 		= A_INDIRECT;
			}
		}
	else if (endsWith(args, ",x", false))
		{
		String arg = args.substr(0, args.length()-2);
		int addr   = _evaluateLabel(arg);
		if (addr <= 0xFF)
			{
			bytes[0] 	= addr & 0xFF;
			amode 		= A_ZEROPAGE_XINDEX;
			}
		else
			{
			bytes[0] 	= (addr     ) & 0xFF;
			bytes[1] 	= (addr >> 8) & 0xFF;
			amode 		= A_ABSOLUTE_XINDEX;
			}
		}
	else if (endsWith(args, ",y", false))
		{
		String arg = args.substr(0, args.length()-2);
		int addr   = _evaluateLabel(arg);
		if ((pass == 2) && addr == UNDEFINED_VALUE)
			FATAL(ERR_PARSE, "Undefined symbol '%s'\n%s",
				arg.c_str(), CTXMGR->location().c_str());
				
		if (addr <= 0xFF)
			{
			bytes[0] 	= addr & 0xFF;
			amode = A_ZEROPAGE_YINDEX;
			}
		else
			{
			bytes[0] 	= (addr     ) & 0xFF;
			bytes[1] 	= (addr >> 8) & 0xFF;
			amode = A_ABSOLUTE_YINDEX;
			}
		}
	else
		{
		FATAL(ERR_PARSE, "Unknown addressing mode '%s'\n%s",
			args.c_str(), CTXMGR->location().c_str());
		}
	
	/************************************************************************\
    |* Check that the addressing mode is legal for this opcode
    \************************************************************************/
    int opcode = Token::opcode(info.which, amode);
    if ((pass == 2) && (opcode < 0))
		FATAL(ERR_PARSE, "Illegal addressing mode '%s' for %s\n%s",
				args.c_str(), info.name.c_str(),
				CTXMGR->location().c_str());
	
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
	if (pass == 2)
		{
		t.setAddrMode(amode);
		if (_showListing)
			fprintf(stderr, "%04x %s\n", _current, t.toString().c_str());
		}
	if (_emit(tokens, t, amode))
		_current += numbytes;
	return 0;
	}
	
/*****************************************************************************\
|* Set the assembly origin by inserting a token into the stream
\*****************************************************************************/
int Scanner::_setOrigin(TokenList &tokens, String args)
	{
	Token t(T_DIRECTIVE, P_ORG);
	_engine.eval(args);
	_current = _engine.result() & 0xFFFF;
	t.setAddr(_current);
	_emit(tokens, t, A_NONE);
	return 0;
	}

/*****************************************************************************\
|* Process if blocks
\*****************************************************************************/
int Scanner::_if(TokenList &tokens, String condition)
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
int Scanner::_else(TokenList &tokens)
	{
	if (_ifState.size() > 0)
		{
		bool state = _ifState.back();
		_ifState.pop_back();
		_ifState.push_back(!state);
		}
	else
		FATAL(ERR_IF, "Found an ELSE without IF\n%s",
			CTXMGR->location().c_str());
		
	return 0;
	}
	
/*****************************************************************************\
|* Process endif blocks
\*****************************************************************************/
int Scanner::_endif(TokenList &tokens)
	{
	if (_ifState.size() > 0)
		_ifState.pop_back();
	else
		FATAL(ERR_IF, "Found an ENDIF without IF\n%s",
			CTXMGR->location().c_str());
		
	return 0;
	}

/*****************************************************************************\
|* Reserve space for the data in the arguments
\*****************************************************************************/
int Scanner::_reserveBytes(TokenList &tokens, int size, String args)
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
					word = "";
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
	
	_emit(tokens, t, A_NONE);
	_current += t.data().size();
	
	return 0;
	}

/*****************************************************************************\
|* Embed an address into the byte stream
\*****************************************************************************/
int Scanner::_embedAddress(TokenList &tokens, String args)
	{
	// FIXME - needs implementation
	return 0;
	}
	
/*****************************************************************************\
|* Does this line start with a label
\*****************************************************************************/
Token Scanner::_hasLabel(String& s)
	{
	Token t;
	String label = "";
	
	int idx = 0;
	while ((idx < s.length()) && (!::isspace(s[idx])))
		{
		label += s[idx];
		idx ++;
		
		// In case there's no ' ' after the ':'
		if (label.back() == ':')
			break;
		}
	
	/*************************************************************************\
    |* See if this is a global or local label
    \*************************************************************************/
	bool isLabel	= (label.back() == ':');
	label.pop_back();
	
	if (isLabel)
		{
		CTXMGR->addLabel(label, _current);
		
		String sanitised = label;
		if (sanitised[0] == '@')
			sanitised =label.substr(1);

		t.setType(T_LABEL);
		t.setWhich(P_LABEL);
		t.setArg1(CTXMGR->identifier()+"_"+sanitised);
		s = trim(s.substr(idx));
		}
		
	return t;
	}
	
/*****************************************************************************\
|* Push a context onto the stack
\*****************************************************************************/
void Scanner::_pushContext(const String &s)
	{
	String name = "";
	String type = "";
	String word = "";
	int line 	= -1;
	int state 	= 0;
	
	std::stringstream ss(s);
	while (ss >> quoted(word, '\''))
		{
		switch (state)
			{
			case 0:
			case 1:
				break;
			case 2:
				type = word;
				break;
			case 3:
				name = word;
				break;
			case 4:
				line = std::stoi(word);
				break;
			default:
				fprintf(stderr, "Error: found '%s'\n", word.c_str());
				break;
			}
		state ++;
		}
	
	if ((name.length() > 0) && (type.length() > 0) && (line >= 0))
		CTXMGR->push(name, type, line);
	else
		FATAL(ERR_CTX, "Found an ill-defined context at\n%s",
			  CTXMGR->location().c_str());
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
int Scanner::_next(void)
	{
	if (_atEnd())
		return EOF;
		
	int c = _src[_at++];
	if (c == '\n')
		{
		CTXMGR->incLine();
		}
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
String Scanner::_nextLine(int &state)
	{
	String s = "";
	state	 = SCAN_MORE;
	
	while (!_atEnd())
		{
		int c = _src[_at++];
		if (c == '\n')
			{
			CTXMGR->incLine();
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
					Token token,
					AddressingMode amode)
	{
	bool emit = shouldEvaluate();
	if (emit)
		{
		tokens.push_back(token);
		token.setAddrMode(amode);
		_listing += /* toHexString(_current) + " : "  + */
				 token.toString(_current)+"\n";
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

/*****************************************************************************\
|* Evaluate a branch value
\*****************************************************************************/
int Scanner::_evaluateLabel(String s)
	{
	String vars = CTXMGR->labelValues();
	vars += "\n" +s;

	Engine& e 		= Engine::getInstance();
	e.eval(vars);
	return (int) e.result();
	}
