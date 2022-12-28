//
//  Driver.cc
//  xtal
//
//  Created by Simon Gornall on 11/25/22.
//
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <unistd.h>

#include <errno.h>

namespace fs = std::filesystem;

#include "ArgParser.h"
#include "Driver.h"
#include "Stringutils.h"

#define DEFAULT_ORG		0x6000

static int _debugLevel;

/****************************************************************************\
|* Constructor
\****************************************************************************/
Driver::Driver()
	{
	}
	
/*****************************************************************************\
|* Run the Driver
\*****************************************************************************/
int Driver::main(int argc, const char *argv[])
	{
	int ok = 0;
	
	/*************************************************************************\
	|* Start off with a default value for where xtal is installed
	\*************************************************************************/
	const char *base = "/opt/xtal";
	if (getenv("XTAL_BASEDIR") != NULL)
		base = getenv("XTAL_BASEDIR");

	/*************************************************************************\
	|* Configure options and flags
	\*************************************************************************/
	_ap = new ArgParser(argc, argv);
	
	_baseDir 		= _ap->stringFor("-xb", "--xtal-base-dir", base,
										"Config", "Install dir of xtal");
	
	_symbols		= _ap->listOf("-D", "--define", "Runtime",
										"Define constants in the symbol table");
	_includeDirs	= _ap->listOf("-i", "--include", "Runtime",
										"Locations to look for library files");
									 
									 
    _debugLevel     = _ap->flagFor("-d", "--debug", 0,
										"General", "increase the debug level");
    _output			= _ap->stringFor("-o", "--output", "/tmp/out.com",
										"General", "Output binary filename");
    _hexOutput		= _ap->stringFor("-oh", "--output-hex", "",
										"General",
										"Output hex-dump filename");
    _listFile		= _ap->stringFor("-l", "--list", "",
										"General",
										 "Output listing filename");
    _dumpTree		= _ap->flagFor("-T", "--dump-ast", false,
										"General",
										 "Output Abstract Syntax Tree");

	/*************************************************************************\
	|* Check for help
	\*************************************************************************/
	bool help		= _ap->flagFor("-h", "--help", false,
									"General", "Show this wonderful help");
	if (help)
		_ap->usage(true);
		
	/*************************************************************************\
	|* Run through the symbols to see if org= has been defined. If not,
	|* define it to be the default value
	\*************************************************************************/
	bool foundOrg = false;
	for (String symbol : _symbols)
		{
		if (lcase(symbol).starts_with("org="))
			{
			foundOrg = true;
			break;
			}
		}
	
	if (!foundOrg)
		{
		char buf[1024];
		snprintf(buf, 1024, "org=0x%x", DEFAULT_ORG);
		_symbols.push_back(buf);
		}
		
	/*************************************************************************\
	|* Construct a commandline arg and call the compiler
	\*************************************************************************/
	String cCmd = _cCmd();
	if (_debugLevel > 0)
		printf("%s\n", cCmd.c_str());
	
	int status = system(cCmd.c_str());
	
	if  (status == 0)
		{
		String aCmd = _aCmd();
		if (_debugLevel > 0)
			printf("%s\n", aCmd.c_str());
		system(aCmd.c_str());
		}
	//remove(_asmFile.c_str());
	return ok;
	}


#pragma mark - private methods

/*****************************************************************************\
|* Create the commandline for the compiler
\*****************************************************************************/
String Driver::_cCmd(void)
	{
	char buf[1024];
	snprintf(buf, 1024, "%s/bin/xtal-c ", _baseDir.c_str());
	String cCmd = buf;
	
	for (int i=0; i<_debugLevel; i++)
		cCmd += "-d ";
	
	if (_dumpTree)
		cCmd += "-T ";
		
	_asmFile = fs::temp_directory_path().string() + randomString(8) + ".asm";
	cCmd += "-o " + _asmFile + " ";
	
	cCmd += "-xb " + _baseDir + " ";
	
	for (String srcfile : _ap->remainingArgs())
		cCmd += "'" + srcfile + "' ";
	
	return cCmd;
	}

/*****************************************************************************\
|* Create the commandline for the assembler
\*****************************************************************************/
String Driver::_aCmd(void)
	{
	char buf[1024];
	snprintf(buf, 1024, "%s/bin/xtal-a ", _baseDir.c_str());
	
	String aCmd = buf;
	if (_symbols.size() > 0)
		for (String sym : _symbols)
			aCmd += "-D '"+sym+"' ";
			
	if (_includeDirs.size() > 0)
		for (String idir : _includeDirs)
			aCmd += "-i '"+idir + "' ";

	for (int i=0; i<_debugLevel; i++)
		aCmd += "-d ";
	
	aCmd += "-o '" + _output + "' ";
	
	if (_hexOutput != "")
		aCmd += "-oh '" + _hexOutput + "' ";
	
	if (_listFile != "")
		aCmd += "-l '" + _listFile + "' ";
	
	aCmd += "-xb " + _baseDir + " ";
	
	aCmd += _asmFile;
	
	return aCmd;
	}
