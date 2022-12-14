//
//  Assembler.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/27/22.
//

#include <iostream>
#include <fstream>
#include <regex>

#include "ArgParser.h"
#include "Assembler.h"
#include "Engine.h"
#include "Locator.h"
#include "Scanner.h"
#include "Stringutils.h"
#include "Token.h"

#define CTXMGR					ContextMgr::sharedInstance()

static int _debugLevel;

typedef enum
	{
	NORMAL = 0,
	MACRO,
	FUNCTION
	} SearchMode;

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Assembler::Assembler()
		  :_ap(nullptr)
	{
	}

/*****************************************************************************\
|* Destructor
\*****************************************************************************/
Assembler::~Assembler(void)
	{
	delete _ap;
	}

/*****************************************************************************\
|* Run the compiler
\*****************************************************************************/
int Assembler::main(int argc, const char *argv[])
	{
	_ap = new ArgParser(argc, argv);

	/*************************************************************************\
	|* Start off with a default value for where xtal is installed
	\*************************************************************************/
	const char *base = "/opt/xtal";
	if (getenv("XTAL_BASEDIR") != NULL)
		base = getenv("XTAL_BASEDIR");


	_symbols				= _ap->listOf("-D", "--define", "Runtime",
										  "Define constants in the symbol table");
    _debugLevel      		= _ap->flagFor("-d", "--debug", 0,
										  "General",
										  "increase the debug level");
    _output					= _ap->stringFor("-o", "--output", "a.out",
										  "General",
										  "Output filename");
    _hexOutput				= _ap->stringFor("-oh", "--output-hex", "",
										  "General",
										  "Output hex-dump filename");
    _listFile				= _ap->stringFor("-l", "--list", "",
										  "General",
										  "Output listing filename");
	_includeDirs			= _ap->listOf("-i", "--include",
										  "Runtime",
										  "Locations to look for library files");
	_baseDir				= _ap->stringFor("-xb", "--xtal-base-dir", base,
										  "Runtime",
										  "Compiler base directory");
	_outputSymbols			= _ap->flagFor("-s", "--symbol-table", false,
										  "General",
										  "Embed a symbol table in output");
	_includeDirs.push_back("/opt/xtal/lib");
	
	/*************************************************************************\
	|* Check on help
	\*************************************************************************/
	bool help				= _ap->flagFor("-h", "--help", 0,
										  "General",
										  "This wonderful help text");
	
	if (help)
		_ap->usage(true);
		
	/*************************************************************************\
	|* Construct the input by catenating any argument names without switches
	\*************************************************************************/
	StringList sourceFiles 	= _ap->remainingArgs();

	std::string input;
	for (std::string filename : sourceFiles)
		{
		std::ifstream in(filename, std::ios::in);
		if (in)
			{
			std::string contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();
			
			String directive = ".push context file '" + filename + "' 1\n";
			input += directive + trim(contents) + "\n.pop context\n";
			}
		else
			{
			WARN("Couldn't read file %s\n", filename.c_str());
			}
		}
		
	if (input.length() == 0)
		FATAL(ERR_NO_SOURCE_FILES, "Cannot read input file(s)\n");
		
	/*************************************************************************\
	|* Handle any includes to create the composite source
	\*************************************************************************/
	input = _preparse(input);
	
	return _run(input);
	}

/*****************************************************************************\
|* Error handling - display an informative message
\*****************************************************************************/
void Assembler::error(String msg)
	{
	_report("", msg);
	}

	
#pragma mark - Private Methods


/*****************************************************************************\
|* Private method : Generic run method
\*****************************************************************************/
int Assembler::_run(std::string source)
	{
	int ok = 0;
	std::vector<Token> tokens;

	/*************************************************************************\
	|* Create the scanner and insert any starting symbols
	\*************************************************************************/
	Scanner scanner;
	scanner.insertSymbols(_symbols);
	scanner.setMacros(_macros);
	scanner.setFunctions(_functions);
	
	/*************************************************************************\
	|* Parse the token-stream into a bunch of output blocks
	\*************************************************************************/
	bool foundOrigin = false;
	int64_t where;
	if (scanner.engine().symbolValue("org", where))
		source = ".org " + std::to_string(where)+"\n"+source;
	
	/*************************************************************************\
	|* Set the source
	\*************************************************************************/
	scanner.setSrc(source);
	if (_debugLevel > 1)
		scanner.setShowListing(true);
		
	/*************************************************************************\
	|* Run the first pass of the assembler, so any variables with forward refs
	|* are picked up correctly
	\*************************************************************************/
	scanner.engine().labelMap().clear();
	while (scanner.scan(tokens, 1) == Scanner::SCAN_MORE)
		;

	/*************************************************************************\
	|* Add in any called functions that we discovered and carry on from where
	|* we left off
	\*************************************************************************/
	while (scanner.appendUsedFunctions())
		while (scanner.scan(tokens, 1) == Scanner::SCAN_MORE)
			;

	if (_debugLevel > 1)
		{
		fprintf(stderr, "\n=====\n\n");
		scanner.engine().dumpVars();
		fprintf(stderr, "\n=====\n\n%s\n", LOCATOR->labelValues().c_str());
		fprintf(stderr, "\n=====\n\n");
		}
	
	/*************************************************************************\
	|* Run a sanity check on the if blocks being properly closed
	\*************************************************************************/
	if (scanner.ifState().size() != 0)
		FATAL(ERR_IF, "Unterminated IF block\n");

	/*************************************************************************\
	|* Run the second pass of the assembler, getting all the references right
	\*************************************************************************/
	scanner.engine().labelMap().clear();
	tokens.clear();
	scanner.reset();
	scanner.setSrc(source);
	while (scanner.scan(tokens, 2) == Scanner::SCAN_MORE)
		;
		
	/*************************************************************************\
	|* Add in any called functions that we discovered and carry on from where
	|* we left off
	\*************************************************************************/
	while (scanner.appendUsedFunctions())
		while (scanner.scan(tokens, 2) == Scanner::SCAN_MORE)
			;
			
	/*************************************************************************\
	|* If we're optimising, do so.
	|*
	|* Actually this is a  lot harder than it seems at first glance. Given
	|* that we can JSR through a vector, it's not clear that removing
	|* redundant LDA in LDA/STA/LDA/STA (for example) is guaranteed to be
	|* safe.
	|*
	|* Need to noodle on this a lot more
	\*************************************************************************/
	

	/*************************************************************************\
	|* Output the listing if we have the output path
	\*************************************************************************/
	if (_listFile.length() > 0)
		{
		FILE *fp = fopen(_listFile.c_str(), "w");
		if (fp != NULL)
			{
			fputs(scanner.listing().c_str(), fp);
			fclose(fp);
			}
		}
		
	/*************************************************************************\
	|* Get the tokens to output themselves
	\*************************************************************************/
	for (Token& token : tokens)
		{
		if (token.which() == P_ORG)
			{
			foundOrigin = true;
			OutputBlock block(token.addr());
			_blocks.push_back(block);
			}
		else if (token.which() == P_BYTE)
			{
			if (!foundOrigin)
				FATAL(ERR_ORIGIN, "No origin defined!\n");
			_blocks.back().add(token);
			}
		else if (token.type() == T_6502)
			{
			if (!foundOrigin)
				FATAL(ERR_ORIGIN, "No origin defined!\n");
			_blocks.back().add(token);
			}
		}

	/*************************************************************************\
	|* Find the address and size of the largest block
	\*************************************************************************/
	if (_outputSymbols)
		{
        /********************************************************************\
        |* Determine how much space we can use per symbol table block
        \********************************************************************/
		uint16_t symAddress = 0;
		uint16_t symLength	= 0;
		for (OutputBlock& block : _blocks)
			{
			if (block.size() > symLength)
				{
				symLength 	= block.size();
				symAddress	= block.baseAddress();
				}
			}
		if (symLength < 256)
			symLength = 256;
		
        /********************************************************************\
        |* Start adding tokens from the Engine's symbol table
        \********************************************************************/
		Engine::Symbols &symbols = scanner.engine().symbols();
		OutputBlock symBlock(symAddress);
		symBlock.add("`SYM", 4);	// RTS then SYM
		symBlock.setIsData(true);	// Not code
		
		for (Elements<String,Engine::Symbol> kv : symbols)
			{
			if (symBlock.size() -2 > symLength)
				{
				symBlock.addChecksum();
				_blocks.insert(_blocks.begin(), symBlock);
				symBlock.clear();
				symBlock.add("`SYM", 4);	// RTS then SYM
				}
			
			uint8_t hdr[3];
			hdr[0] = kv.value.value & 0xFF;
			hdr[1] = (kv.value.value >> 8) & 0xFF;
			hdr[2] = (uint8_t)(kv.key.length());
			symBlock.add(hdr, 3);
			symBlock.add(kv.key.c_str(), (int) kv.key.length());
			}
		
        /********************************************************************\
        |* Add any trailing block
        \********************************************************************/
		if (symBlock.size() > 6)
			{
			symBlock.addChecksum();
			_blocks.insert(_blocks.begin(), symBlock);
			symBlock.clear();
			symBlock.add("`SYM", 4);	// RTS then SYM
			}
		}

	/*************************************************************************\
	|* Finally write a block to set the execution address explicitly
	\*************************************************************************/
	OutputBlock exBlk(0x02E0);
	uint16_t vec = 0;
	for (int i=0; i<_blocks.size(); i++)
		{
		if (!_blocks[i].isData())
			{
			vec = _blocks[i].baseAddress();
			break;
			}
		}
	uint8_t start[2];
	start[0] = vec & 0xFF;
	start[1] = vec >> 8;
	exBlk.add(start, 2);
	_blocks.push_back(exBlk);

	/*************************************************************************\
	|* For each output block, finalise it, and write to disk
	\*************************************************************************/
	FILE *fp = fopen(_output.c_str(), "wb");
	if (fp == NULL)
		FATAL(ERR_OUTPUT, "Cannot open %s for write\n", _output.c_str());
		
	for (OutputBlock& block : _blocks)
		{
		block.finalise();
		block.write(fp);
		}
	fclose(fp);
			
	/*************************************************************************\
	|* If we're writing hex-dumps, do it from the blocks
	\*************************************************************************/
	if (_hexOutput.length() > 0)
		{
		fp = fopen(_hexOutput.c_str(), "wb");
		if (fp == NULL)
			FATAL(ERR_OUTPUT, "Cannot open %s for hex write\n", _hexOutput.c_str());

		for (OutputBlock& block : _blocks)
			block.writeHex(fp);
		fclose(fp);
		}
		
	return ok;
	}

/*****************************************************************************\
|* Private method : Generic run method
\*****************************************************************************/
void Assembler::_report(std::string where, std::string msg)
	{
	fprintf(stderr, "Error %s:%s\n", where.c_str(), msg.c_str());
	}


/*****************************************************************************\
|* Look for .include statements. This isn't the most efficient method, by some
|* way, but given how small 8-bit programs are, simplicity wins :)
\*****************************************************************************/
String Assembler::_preparse(String src)
	{
	bool needsPass 				= true;
	
	while (needsPass)
		{
		needsPass 		 		= false;
		String result    		= "";
		StringList lines 		= split(src, '\n');
		SearchMode currentMode	= NORMAL;
		Macro macro;
		Function function;
		
		for (String line : lines)
			{
			String lc = lcase(line);
			if (trim(lc)[0] == ';')
				continue;
				
			if (lc.find(".include ") != std::string::npos)
				{
				StringList words = split(trim(line), " \t");
				if (words.size() != 2)
					{
					FATAL(ERR_INCLUDE,
						  "Malformed .include directive '%s'\n",
						  line.c_str());
					}
				String fname   = trim(words[1]);
				String content = _find(fname);
				if (content.size() > 0)
					{
					needsPass = true;
					result += ".push context file '" + fname + "' 1\n";
					result += trim(content) + "\n";
					result += ".pop context\n";
					}
				else
					{
					FATAL(ERR_INCLUDE,
						  "Cannot satisfy .include directice '%s'\n",
						  line.c_str());
					}
				}
			else if (lc.find(".macro") != std::string::npos)
				{
				result += ";" + line + "\n";
				currentMode = MACRO;
				StringList words = split(trim(line), " \t");
				if (words.size() > 1)
					{
					macro.reset();
					macro.setName(words[1]);
					
					String ctx = ".push context macro '" + words[1] + "' 1";
					macro.lines().push_back(ctx);
					}
				else
					FATAL(ERR_MACRO, "Unnamed macro found: '%s'\n", line.c_str());
				}
			else if (lc.find(".endmacro") != std::string::npos)
				{
				result += ";" + line + "\n";
				currentMode = NORMAL;
				macro.lines().push_back(".pop context");
				_macros[macro.name()] = macro;
				}
			else if (lc.find(".function") != std::string::npos)
				{
				StringList words = split(trim(line), " \t");
				if (words.size() > 1)
					{
					if (!_functionIsDefined(words[1]))
						{
						function.reset();
						function.setName(words[1]);
						
						String ctx = ".push context function '" + words[1] + "' 1";
						function.lines().push_back(ctx);
						result += ";" + line + "\n";
						currentMode = FUNCTION;
						}
					}
				else
					FATAL(ERR_FUNCTION, "Unnamed function found: '%s'\n",
						line.c_str());
				}
			else if (lc.find(".endfunction") != std::string::npos)
				{
				result += ";" + line + "\n";
				currentMode = NORMAL;
				function.lines().push_back(".pop context");
				_functions[function.name()] = function;
				function.reset();
				}
			else
				{
				if (currentMode == MACRO)
					{
					macro.lines().push_back(line);
					result += ";\t"+line+"\n";
					
					std::regex expr("\\%[0-9]+");
					std::smatch m;
					String::const_iterator start(line.cbegin());
					while (regex_search(start, line.cend(), m, expr))
						{
						String arg = m[0];
						int argId = std::stoi(arg.substr(1));
						if (macro.args() < argId)
							macro.setArgs(argId);
							
						start = m.suffix().first;
						}
					}
				else if (currentMode == FUNCTION)
					{
					String trimmed = trim(lc);
					if (trimmed.starts_with(".clobber"))
						{
						StringList words = split(trimmed, " \t");
						if (words.size() < 2)
							FATAL(ERR_FUNCTION, "Illegal clobber: '%s'\n",
								line.c_str());
						String arg = words[1];
						StringList regs = split(arg, ",");
						for (String reg : regs)
							function.clobbers().push_back(trim(reg));
						}
					else
						function.lines().push_back(line);
						
					result += ";\t" + line + "\n";
					}
				else
					result += trim(line) + "\n";
				}
			}
		src = result;
		}
	return src;
	}

/*****************************************************************************\
|* Find an include file
\*****************************************************************************/
String Assembler::_find(String path)
	{
	String src 		= "";
	char slash 		= std::filesystem::path::preferred_separator;
	
	StringList candidates;
	candidates.push_back(path);
	for (String dir : _includeDirs)
		{
		candidates.push_back(dir + slash + path);
		candidates.push_back(dir + slash + path + ".s");
		}
		
	for (String fullpath : candidates)
		{
		if (std::filesystem::exists(fullpath))
			{
			std::ifstream in(fullpath, std::ios::in);
			if (in)
				{
				in.seekg(0, std::ios::end);
				src.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&src[0], src.size());
				in.close();
				src = trim(src) + '\n';
				break;
				}
			}
		}
	return src;
	}


/*****************************************************************************\
|* Determins if a function has been defined already
\*****************************************************************************/
bool Assembler::_functionIsDefined(String fnName)
	{
	return _functions.count(fnName) > 0;
	}
	
#pragma mark - Helper Functions
/*****************************************************************************\
|* Implement the callback for logging vs debuglevel
\*****************************************************************************/
int debugLevel(void)
	{
	return _debugLevel;
	}
