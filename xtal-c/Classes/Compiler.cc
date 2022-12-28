//
//  Compiler.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include <iostream>
#include <fstream>
#include <filesystem>

#include "Compiler.h"
#include "debug.h"

#include "ArgParser.h"
#include "ASTNode.h"
#include "A8Emitter.h"
#include "Expression.h"
#include "Register.h"
#include "RegisterFile.h"
#include "Stringutils.h"
#include "Scanner.h"
#include "Statement.h"
#include "SymbolTable.h"
#include "Token.h"

namespace fs = std::filesystem;

static int _debugLevel;

String fileContent(String& path, bool& ok);

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Compiler::Compiler()
		 :_ap(nullptr)
		 ,_hadError(false)
		 ,_line(0)
	{
	_hadError 	= false;
	_emitter	= new A8Emitter();
	SYMTAB->setEmitter(_emitter);
	}

/*****************************************************************************\
|* Destructor
\*****************************************************************************/
Compiler::~Compiler()
	{
	delete _emitter;
	delete _ap;
	}

/*****************************************************************************\
|* Run the compiler
\*****************************************************************************/
int Compiler::main(int argc, const char *argv[])
	{
	_ap = new ArgParser(argc, argv);

	/*************************************************************************\
	|* Start off with a default value for where xtal is installed
	\*************************************************************************/
	const char *base = "/opt/xtal";
	if (getenv("XTAL_BASEDIR") != NULL)
		base = getenv("XTAL_BASEDIR");

    _dumpAST				= _ap->flagFor("-T", "--dump-AST", false,
										   "Runtime",
										   "Whether to dump the AST tree");
    _debugLevel      		= _ap->flagFor("-d", "--debug", 0,
										   "Runtime",
										   "Increase the debugging level");
    std::string output		= _ap->stringFor("-o", "--output", "/tmp/out.s"
										   "Runtime",
										   "Where to write the assembly output");
	_baseDir				= _ap->stringFor("-xb", "--xtal-base-dir", base,
										  "Runtime",
										  "Compiler base directory");
	
	/*************************************************************************\
	|* Construct the input by catenating any argument names without switches
	\*************************************************************************/
	StringList sourceFiles 	= _ap->remainingArgs();

	std::string input;
	for (std::string filename : sourceFiles)
		{
		bool ok = false;
		input += trim(fileContent(filename, ok));
		if (!ok)
			{
			WARN("Couldn't read file %s\n", filename.c_str());
			}
		}
		
	if (input.length() == 0)
		FATAL(ERR_NO_SOURCE_FILES, "Cannot read input file(s)");

	/*************************************************************************\
	|* Recursively look for #import <filename> and prepend it to the source
	\*************************************************************************/
	_handleImports(input);

	FILE *fp = fopen(output.c_str(), "w");
	if (fp == NULL)
		FATAL(ERR_OUTPUT, "Cannot open '%s' for write", output.c_str());
	_emitter->setOfp(fp);
	
	int ok = _run(input);
	fclose(fp);
	_emitter->setOfp(nullptr);
	return ok;
	}

/*****************************************************************************\
|* Error handling - display an informative message
\*****************************************************************************/
void Compiler::error(int line, std::string msg)
	{
	_report(line, "", msg);
	}
	
#pragma mark - Private Methods


/*****************************************************************************\
|* Private method : Generic run method
\*****************************************************************************/
int Compiler::_run(std::string source)
	{
	Register none(Register::NO_REGISTER);
	int ok = 0;
	
	Scanner scanner(source);
	Token t;
	Statement stmt(scanner, _emitter);
		
	scanner.scan(t, _line);
	_emitter->preamble();
	ASTNode *tree = stmt.globalDeclaration(t, _line);
	_emitter->postamble();
	
	if (_dumpAST && (tree != nullptr))
		tree->dump();
		
	return ok;
	}

/*****************************************************************************\
|* Private method : Generic run method
\*****************************************************************************/
void Compiler::_report(int line, std::string where, std::string msg)
	{
	fprintf(stderr, "[line:%d] Error %s:%s\n",
			line,
			where.c_str(),
			msg.c_str());
	}
	

/*****************************************************************************\
|* Private method : Import any library code
|*
|* we look for '#import <path>' and if we find one, we:
|*	- remove it from the source code
|*  - search for the named .xt file in the library location(s)
|* 		- if found, place it at the end of the source
|*  - search for the named .h file in the library location(s)
|* 		- if found, place it at the start of the source
\*****************************************************************************/
void Compiler::_handleImports(String& src)
	{
	bool needsAnotherPass = true;
	
	/*************************************************************************\
    |* Continue until we're done
    \*************************************************************************/
	while (needsAnotherPass)
		{
		needsAnotherPass = false;
		int len = (int) src.length();
		
		size_t at = src.find("#import");
		if (at != String::npos)
			{
			String search = "#import";
			String name = "";
			
			/*****************************************************************\
			|* Look for <..> until we find a newline, and parse out the content
			\*****************************************************************/
			at += 7;
			while ((at < len) && (src[at] != '\n') && (src[at] != '<'))
				{
				search += src[at];
				at ++;
				}
			if (src[at] == '<')
				{
				search += src[at];
				at ++;
				}
			while ((at < len) && (src[at] != '\n') && (src[at] != '>'))
				{
				name += src[at];
				search += src[at];
				at ++;
				}
			if (src[at] == '>')
				{
				search += src[at];
				at ++;
				}

			/*****************************************************************\
			|* If name is empty, bail
			\*****************************************************************/
			if (name.length() == 0)
				FATAL(ERR_INCLUDE, "Cannot find empty import");
			

			/*****************************************************************\
			|* replace the import command with spaces
			\*****************************************************************/
			String spaces = std::string(search.length(), ' ');
			src = replace(src, search, spaces, false);
			
			/*****************************************************************\
			|* See if we can find name.h
			\*****************************************************************/
			char buf[1024];
			snprintf(buf, 1024, "%s/lib/modules/%s/%s.h",
								_baseDir.c_str(),
								name.c_str(),
								name.c_str());
			if (fs::exists(buf))
				{
				bool ok;
				String headerPath = buf;
				String content = fileContent(headerPath, ok);
				if (!ok)
					{
					WARN("Couldn't import header from %s\n", name.c_str());
					}
				else
					src = content + "\n" + src;
				}

			/*****************************************************************\
			|* See if we can find name.xt
			\*****************************************************************/
			snprintf(buf, 1024, "%s/lib/modules/%s/%s.xt",
								_baseDir.c_str(),
								name.c_str(),
								name.c_str());
			if (fs::exists(buf))
				{
				bool ok;
				String headerPath = buf;
				String content = fileContent(headerPath, ok);
				if (!ok)
					{
					WARN("Couldn't import header from %s\n", name.c_str());
					}
				else
					src = src + "\n" + content;
				}

			/*****************************************************************\
			|* See if we can find name.s
			\*****************************************************************/
			snprintf(buf, 1024, "%s/lib/modules/%s/%s.s",
								_baseDir.c_str(),
								name.c_str(),
								name.c_str());
			if (fs::exists(buf))
				{
				snprintf(buf, 1024, ".include modules/%s/%s.s",
								name.c_str(),
								name.c_str());
				_emitter->append(buf, Emitter::PREAMBLE);
				}
			}
		}
	}
	
#pragma mark - Helper Functions
/*****************************************************************************\
|* Implement the callback for logging vs debuglevel
\*****************************************************************************/
int debugLevel(void)
	{
	return _debugLevel;
	}

/*****************************************************************************\
|* Return the contents of a file
\*****************************************************************************/
String fileContent(String& path, bool& ok)
	{
	String contents = "";
	
	std::ifstream in(path, std::ios::in);
	if (in)
		{
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		ok = true;
		}
	else
		ok = false;
	return contents;
	}
