//
//  Compiler.cc
//  xtal
//
//  Created by Thrud The Barbarian on 10/25/22.
//

#include <iostream>
#include <fstream>

#include "Compiler.h"
#include "debug.h"

#include "ArgParser.h"
#include "ASTNode.h"
#include "Emitter.h"
#include "Expression.h"
#include "Register.h"
#include "Stringutils.h"
#include "Scanner.h"
#include "Token.h"

static int _debugLevel;

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Compiler::Compiler()
		 :_ap(nullptr)
		 ,_hadError(false)
		 ,_line(0)
	{
	_hadError 	= false;
	_emitter	= new Emitter();
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

    _debugLevel      		= _ap->flagFor("-d", "--debug", 0);
    std::string output		= _ap->stringFor("-o", "--output", "/tmp/out.s");

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
			input += trim(contents) + '\n';
			}
		else
			{
			WARN("Couldn't read file %s\n", filename.c_str());
			}
		}
		
	if (input.length() == 0)
		FATAL(ERR_NO_SOURCE_FILES, "Cannot read input file(s)");
	return _run(input);
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
	int ok = 0;
	
	Scanner scanner(source);
	Token t;
	
	scanner.scan(t, _line);
	ASTNode *node = Expression::binary(scanner, t, _line, 0);
	Register r = _emitter->emit(node);
	_emitter->printReg(r);

	printf("%s\n%s\n%s\n---\n",
		_emitter->preamble().c_str(),
		_emitter->output().c_str(),
		_emitter->postamble().c_str());
	
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
	
	
#pragma mark - Helper Functions
/*****************************************************************************\
|* Implement the callback for logging vs debuglevel
\*****************************************************************************/
int debugLevel(void)
	{
	return _debugLevel;
	}