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
 
	typedef std::vector<Token> 		TokenList;
	typedef std::vector<bool> 		BoolList;
	typedef std::map<String, int> 	IntMap;
	typedef std::map<String, bool>	BoolMap;
	
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
    GETSET(String, src, Src);					// Source code for the compiler
    GET(int, at);								// Location in the source code
	GETSET(int, current, Current);				// Current memory location
	GETSET(MacroMap, macros, Macros);			// Map of macros from assembler
	GETSET(FunctionMap, functions, Functions);	// Map of funcs from assembler
	GET(BoolList, ifState);						// 'if' state hierarchy
	GET(String, listing);						// String form of output
	GET(int, labelId);							// Current numeric label id
	GETSET(bool, showListing, ShowListing);		// Show internal listing
	GETSET(String, currentLine, CurrentLine);	// Current source line
	
    private:
		Engine& 	_engine;			// YACC-based expression-parsing engine
		int			_pageIndex[4];		// Current 'page' for banked addresses

        /*********************************************************************\
        |* Infer the types and sizes of the registers being used
        \*********************************************************************/
		IntMap 		_regSize;			// Map of byte-count to register name
		IntMap 		_regSign;			// Map of signed to register name
		BoolMap		_regSigned;			// true == signed arithmetic
		
        /*********************************************************************\
        |* Get the next character in the input stream
        \*********************************************************************/
        int _next(void);
   
        /*********************************************************************\
        |* Get the next word in the input stream
        \*********************************************************************/
        String _firstWord(String s, int &state);
      
        /*********************************************************************\
        |* Get the next line in the input stream
        \*********************************************************************/
        String _nextLine(int &state);
        
		/*********************************************************************\
        |* Are we at the end of 'file'
        \*********************************************************************/
        bool _atEnd(void);
        
		/*********************************************************************\
        |* Push a context into the stack
        \*********************************************************************/
		void _pushContext(const String& s);
        
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
        int _if(TokenList &tokens, String condition);

        /*********************************************************************\
        |* Handle an else statement
        \*********************************************************************/
        int _else(TokenList &tokens);

        /*********************************************************************\
        |* End an if block
        \*********************************************************************/
        int _endif(TokenList &tokens);

        /*********************************************************************\
        |* Reserve space for the data in the arguments
        \*********************************************************************/
        int _reserveBytes(TokenList &tokens, int size, String args);

		/*********************************************************************\
        |* Embed an address into the byte stream
        \*********************************************************************/
        int _embedAddress(TokenList &tokens, String arg);

        /*********************************************************************\
        |* Set the origin
        \*********************************************************************/
		int _setOrigin(TokenList &tokens, String arg);

        /*********************************************************************\
        |* Add a macro
        \*********************************************************************/
        int _macro(TokenList &tokens, Macro macro, String args);

        /*********************************************************************\
        |* Make sure that any registers that need banks remapped, have that
        |* happen by writing to the correct registers in page 0
        \*********************************************************************/
		void _surfaceRegs(TargetType t1,
						  TargetType t2,
						  int64_t v1,
						  int64_t v2,
						  TokenList &tokens,
						  int pass);

        /*********************************************************************\
        |* Make sure that any registers that need banks remapped, have that
        |* happen by writing to the correct registers in page 0. This is for
        |* 3-argument checks
        \*********************************************************************/
		void _surfaceRegs3(TargetType t1,
						   TargetType t2,
						   TargetType t3,
						   int64_t v1,
						   int64_t v2,
						   int64_t v3,
						   TokenList &tokens,
						   int pass);

        /*********************************************************************\
        |* Insert a 3-register operation into the source code, so it will be
        |* acted on next
        \*********************************************************************/
		void _insertOp(TargetType t1,
					   TargetType t2,
					   TargetType t3,
					   int64_t v1,
					   int64_t v2,
					   int64_t v3,
					   String stem,
					   int overflowPos
					   );
					   
        /*********************************************************************\
        |* Handle 'move'
        \*********************************************************************/
		int _handleMove(Token::TokenInfo info,
						int extent,
						String args,
						TokenList &tokens,
						int pass);

        /*********************************************************************\
        |* Handle 'mul'
        \*********************************************************************/
		int _handleMath(String word,
					    Token::TokenInfo info,
					    int extent,
					    String args,
					    TokenList &tokens,
					    int pass,
						String op);
 
        /*********************************************************************\
        |* Handle any meta opcodes
        \*********************************************************************/
		int _handleMeta(String word,
						Token::TokenInfo info,
						int extent,
						String args,
						TokenList &tokens,
						int pass);

        /*********************************************************************\
        |* Handle the call/exec opcodes
        \*********************************************************************/
		int _handleCall(Token::TokenInfo info,
						int extent,
						String args,
						TokenList &tokens,
						int pass,
						bool isCall);

        /*********************************************************************\
        |* Handle any 6502-native opcode
        \*********************************************************************/
		int _handle6502(Token::TokenInfo info,
						String args,
						TokenList &tokens,
						int pass);

        /*********************************************************************\
        |* Emit a token into the token stream
        \*********************************************************************/
		bool _emit(TokenList &tokens,
				   Token token,
				   AddressingMode amode);
		
        /*********************************************************************\
        |* Return the target-type and value of a string argument
        \*********************************************************************/
		TargetType _determineTarget(String s, int64_t &value);
		
		
        /*********************************************************************\
        |* Evaluate a branch value
        \*********************************************************************/
		int _evaluateLabel(String s);


        /*********************************************************************\
        |* Handle register hints
        \*********************************************************************/
        void _setRegister(String args);

    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Scanner(String src = "");

        /*********************************************************************\
        |* Scan for tokens
        \*********************************************************************/
        int scan(TokenList &tokens, int pass);

        /*********************************************************************\
        |* Add symbols to the engine
        \*********************************************************************/
        void insertSymbols(StringList symbols);

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

        /*********************************************************************\
        |* Append any used functions to the end of the source
        \*********************************************************************/
        bool appendUsedFunctions(void);
	};

#endif /* Scanner_h */
