//
//  Scanner.h
//  as8
//
//  Created by Thrud The Barbarian on 10/28/22.
//

#ifndef Scanner_h
#define Scanner_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

#include "Token.h"

class Assembler;
class Engine;

class Scanner
	{
    NON_COPYABLE_NOR_MOVEABLE(Scanner)
 
	public:
		enum
			{
			SCAN_COMPLETE = 0,
			SCAN_MORE
			};
	
		typedef enum
			{
			REG_MAIN	= 0,
			REG_FN,
			REG_SCRATCH,
			ABSOLUTE,
			IMMEDIATE
			} TargetType;
 
	typedef std::vector<Token> TokenList;
	typedef std::vector<bool> BoolList;
	
	static const int PAGEIDX0			= 0x80;
	static const int PAGEIDX1			= 0x81;
	static const int PAGEIDX2			= 0x82;
	static const int PAGEIDX3			= 0x83;
	static const int REGMAIN_BASE		= 0xC0;
	static const int REGFN_BASE			= 0xA0;
	static const int REGSCRATCH_BASE	= 0xB0;
	
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    GETSET(String, src, Src);			// Source code for the compiler
    GET(int, at);						// Location in the source code
	GETSET(int, current, Current);		// Current memory location
	GETSET(MacroMap, macros, Macros);	// Map of macros from assembler
	GET(BoolList, ifState);				// 'if' state hierarchy
	GET(String, listing);				// String form of output
	
    private:
		Engine& 	_engine;			// YACC-based expression-parsing engine
		String		_majorLabel;		// Current major label
		int			_pageIndex[4];		// Current 'page' for banked addresses
		
        /*********************************************************************\
        |* Get the next character in the input stream
        \*********************************************************************/
        int _next(int &line);
   
        /*********************************************************************\
        |* Get the next word in the input stream
        \*********************************************************************/
        String _firstWord(String s, int &state);
      
        /*********************************************************************\
        |* Get the next line in the input stream
        \*********************************************************************/
        String _nextLine(int &line, int &state);
        
		/*********************************************************************\
        |* Are we at the end of 'file'
        \*********************************************************************/
        bool _atEnd(void);
        
		/*********************************************************************\
        |* Is the start of the line a label
        \*********************************************************************/
		Token _hasLabel(String& s);

        /*********************************************************************\
        |* "put back" a character in the input stream
        \*********************************************************************/
        void _putBack(void);

        /*********************************************************************\
        |* Begin an if block
        \*********************************************************************/
        int _if(TokenList &tokens, int& line, String condition);

        /*********************************************************************\
        |* Handle an else statement
        \*********************************************************************/
        int _else(TokenList &tokens, int& line);

        /*********************************************************************\
        |* End an if block
        \*********************************************************************/
        int _endif(TokenList &tokens, int& line);

        /*********************************************************************\
        |* Reserve space for the data in the arguments
        \*********************************************************************/
        int _reserveBytes(TokenList &tokens, int& line, int size, String args);

		/*********************************************************************\
        |* Embed an address into the byte stream
        \*********************************************************************/
        int _embedAddress(TokenList &tokens, int& line, String arg);

        /*********************************************************************\
        |* Set the origin
        \*********************************************************************/
		int _setOrigin(TokenList &tokens, int& line, String arg);

        /*********************************************************************\
        |* Add a macro
        \*********************************************************************/
        int _macro(TokenList &tokens, int& line, Macro macro, String args);

        /*********************************************************************\
        |* Handle 'move''
        \*********************************************************************/
		int _handleMove(Token::TokenInfo info,
						int extent,
						String args,
						TokenList &tokens,
						int& line,
						int pass);

        /*********************************************************************\
        |* Handle any meta opcodes
        \*********************************************************************/
		int _handleMeta(Token::TokenInfo info,
						int extent,
						String args,
						TokenList &tokens,
						int& line,
						int pass);

        /*********************************************************************\
        |* Handle any 6502-native opcode
        \*********************************************************************/
		int _handle6502(Token::TokenInfo info,
						String args,
						TokenList &tokens,
						int& line,
						int pass);

        /*********************************************************************\
        |* Emit a token into the token stream
        \*********************************************************************/
		bool _emit(TokenList &tokens,
				   int& line,
				   Token token,
				   AddressingMode amode);
		
        /*********************************************************************\
        |* Return the target-type and value of a string argument
        \*********************************************************************/
		TargetType _determineTarget(String s, int64_t &value, int line);
		
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Scanner(String src = "");

        /*********************************************************************\
        |* Scan for tokens
        \*********************************************************************/
        int scan(TokenList &tokens, int& line, int pass);

        /*********************************************************************\
        |* Add symbols to the engine
        \*********************************************************************/
        void insertSymbols(StringList symbols);

        /*********************************************************************\
        |* Add macros to the scanner
        \*********************************************************************/
        void insertMacros(MacroMap macros);

        /*********************************************************************\
        |* Reset the scanner to a position
        \*********************************************************************/
        void reset (int to=0);

        /*********************************************************************\
        |* Return the engine being used
        \*********************************************************************/
        Engine& engine(void)	{ return _engine; }

        /*********************************************************************\
        |* Return the engine being used
        \*********************************************************************/
        bool shouldEvaluate(void);
	};

#endif /* Scanner_h */