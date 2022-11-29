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
#include "A8Emitter.h"
#include "Expression.h"
#include "Register.h"
#include "RegisterFile.h"
#include "Stringutils.h"
#include "Scanner.h"
#include "Statement.h"
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
	_emitter	= new A8Emitter();
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
	ASTNode *tree = stmt.compoundStatement(t, _line);
	_emitter->emit(tree, none, 0);
	_emitter->postamble();
	
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
