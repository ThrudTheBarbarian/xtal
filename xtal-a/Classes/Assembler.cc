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
#include "ContextMgr.h"
#include "Engine.h"
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

	_symbols				= _ap->listOf("-D", "--define", "Runtime",
										  "Define constants in the symbol table");
    _debugLevel      		= _ap->flagFor("-d", "--debug", 0,
										  "General",
										  "increase the debug level");
    _output					= _ap->stringFor("-o", "--output", "/tmp/out.com",
										  "General",
										  "Output filename");
    _listFile				= _ap->stringFor("-l", "--list", "",
										  "General",
										  "Output listing filename");
	_includeDirs			= _ap->listOf("-i", "--include",
										  "Runtime",
										  "Locations to look for library files");
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
		FATAL(ERR_NO_SOURCE_FILES, "Cannot read input file(s)");
		
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
	//fprintf(stderr, "=====\n%s\n=====\n", source.c_str());

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

	
	//scanner.engine().dumpVars();
	
	/*************************************************************************\
	|* Run a sanity check on the if blocks being properly closed
	\*************************************************************************/
	if (scanner.ifState().size() != 0)
		FATAL(ERR_IF, "Unterminated IF block");

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
				FATAL(ERR_ORIGIN, "No origin defined!");
			_blocks.back().add(token);
			}
		else if (token.type() == T_6502)
			{
			if (!foundOrigin)
				FATAL(ERR_ORIGIN, "No origin defined!");
			_blocks.back().add(token);
			}
		}
			
	/*************************************************************************\
	|* For each output block, finalise it, and write to disk
	\*************************************************************************/
	FILE *fp = fopen(_output.c_str(), "wb");
	if (fp == NULL)
		FATAL(ERR_OUTPUT, "Cannot open %s for write", _output.c_str());
		
	for (OutputBlock& block : _blocks)
		{
		block.finalise();
		block.write(fp);
		}
	
	fclose(fp);
	
	return ok;
	}

/*****************************************************************************\
|* Private method : Generic run method
\*****************************************************************************/
void Assembler::_report(std::string where, std::string msg)
	{
	fprintf(stderr, "Error %s:%s\n%s",
			where.c_str(),
			msg.c_str(),
			CTXMGR->location().c_str());
	}


/*****************************************************************************\
|* Look for .include statements. This isn't the most efficient method, by some
|* way, but given how small 8-bit programs are, simplicity wins :)
\*****************************************************************************/
String Assembler::_preparse(String src)
	{
	auto ctxMgr					= ContextMgr::sharedInstance();
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
						  "Malformed .include directive '%s'\n%s",
						  line.c_str(), CTXMGR->location().c_str());
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
						  "Cannot satisfy .include directice '%s'",
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
					FATAL(ERR_MACRO, "Unnamed macro found: '%s'", line.c_str());
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
					FATAL(ERR_FUNCTION, "Unnamed function found: '%s'\n%s",
						line.c_str(), CTXMGR->location().c_str());
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
							FATAL(ERR_FUNCTION, "Illegal clobber: '%s'\n%s",
								line.c_str(), CTXMGR->location().c_str());
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
		candidates.push_back(dir + slash + path);
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
