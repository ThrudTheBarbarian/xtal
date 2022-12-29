//
//  Locator.cc
//  xtal-a
//
//  Created by Simon Gornall on 11/17/22.
//

#include "Engine.h"
#include "Locator.h"
#include "Stringutils.h"

/******************************************************************************\
|* The global instance
\******************************************************************************/
std::shared_ptr<Locator> Locator::_instance = nullptr;

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Locator::Locator()
	{
	reset();
	_labels.clear();
	}

/******************************************************************************\
|* Return the shared instance
\******************************************************************************/
std::shared_ptr<Locator> Locator::sharedInstance(void)
    {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (!_instance)
        _instance = std::shared_ptr<Locator>(new Locator);

    return _instance;
    }

/******************************************************************************\
|* Create and push the current context
\******************************************************************************/
Locator::Context& Locator::push(const String name,
									  ContextType type,
									  int64_t line)
	{
	_nextContextId ++;
	Context ctx = {.name	 	= name,
				   .type	 	= type,
				   .line 		= line,
				   .contextId	= _nextContextId };
	
	_ctxList.push_back(ctx);
	int idx = (int)(_ctxList.size()-1);
	
	switch (type)
		{
		case C_FILE:
			_fileList.push_back(idx);
			break;
		
		default:
			_blockList.push_back(idx);
			break;
		}
		
	return _ctxList.back();
	}
/******************************************************************************\
|* Create and push the current context
\******************************************************************************/
Locator::Context& Locator::push(const String name,
									  String type,
									  int64_t line)
	{
	ContextType typeId = C_UNKNOWN;
	String lc = lcase(type);
	if (lc == "file")
		typeId = C_FILE;
	else if (lc == "macro")
		typeId = C_MACRO;
	else if (lc == "function")
		typeId = C_FUNCTION;
	else if (lc == "class")
		typeId = C_CLASS;
	else if (lc == "method")
		typeId = C_METHOD;
	else if (lc == "block")
		typeId = C_BLOCK;
	else if (lc == "if")
		typeId = C_IF;
	else if (lc == "while")
		typeId = C_WHILE;
		
	if (typeId != C_UNKNOWN)
		return push(name, typeId, line);
	FATAL(ERR_CTX, "Unknown context-type '%s' encountered", type.c_str());
	}

/******************************************************************************\
|* Increment the current line number within the current context. Do it in both
|* the current file and block context (if we're in a block, we're guaranteed
|* to be in a file)
\******************************************************************************/
int64_t Locator::incLine(int64_t delta)
	{
	int64_t currentCtxLine = -1;
	/*
	if (_fileList.size() > 0)
		{
		int idx = _fileList.back();
		_ctxList[idx].line += delta;
		currentCtxLine = _ctxList[idx].line;
		}
		
	if (_blockList.size() > 0)
		{
		int idx = _blockList.back();
		_ctxList[idx].line += delta;
		currentCtxLine = _ctxList[idx].line;
		}
	*/
	if (_ctxList.size() > 0)
		{
		_ctxList.back().line += delta;
		currentCtxLine = _ctxList.back().line;
		}
	return currentCtxLine;
	}

/******************************************************************************\
|* Reset all the contexts
\******************************************************************************/
void Locator::reset()
	{
	_ctxList.clear();
	_fileList.clear();
	_blockList.clear();
	_nextContextId = 0;
	}
	
/******************************************************************************\
|* Pop a context
\******************************************************************************/
void Locator::pop(void)
	{
	if (_ctxList.size() > 0)
		{
		Context ctx = _ctxList.back();
		switch (ctx.type)
			{
			case C_FILE:
				_fileList.pop_back();
				break;
				
			default:
				_blockList.pop_back();
				break;
			}
		_ctxList.pop_back();
		}
	else
		FATAL(ERR_CTX, "Cannot find context to pop!\n");
	}

/******************************************************************************\
|* Return a location based on the current context hierarchy.
\******************************************************************************/
String Locator::location(void)
	{
	String msg 		= "";
	String prefix 	= "";
	String at 		= "in ";
	for (auto i = _ctxList.rbegin(); i != _ctxList.rend(); ++i)
		{
		msg += prefix + at + _type(i->type)  + " " + i->name
			 + " at line " + std::to_string(i->line)
			 + "\n";
		prefix = ".. ";
		at = "referenced from ";
		}
		
	return msg;
	}


/******************************************************************************\
|* Return the current context identifier for label naming
\******************************************************************************/
String Locator::identifier(void)
	{
	String ident = "Uninitialised";
	
	if (_ctxList.size() > 0)
		{
		Context ctx 	= _ctxList.back();
		String type 	= ucase(_type(ctx.type));
		
		ident			= type.substr(0,1)
						//+ "_"
						//+ ctx.name
						//+ "_"
						+ std::to_string(ctx.contextId);
		
		}
	else
		FATAL(ERR_CTX, "Cannot find context to identify!\n");
	
	return ident;
	}

/******************************************************************************\
|* Add a context-specific, or global label to point to a memory location
|* within the current context
\******************************************************************************/
void Locator::addLabel(String label, int location)
	{
	if (_ctxList.size() > 0)
		{
		int contextId = -1;
		if (label.substr(0,1) == "@")
			{
			String symbolName = label.substr(1);
			Engine::getInstance().updateSymbol(symbolName, location);
			}
		else
			contextId = _ctxList.back().contextId;
		
		_labels[contextId][label] = location;
		}
	else
		FATAL(ERR_CTX, "Cannot find context to add label to!\n");
	}


/******************************************************************************\
|* Return a context-specific, or global label. If it doesn't exist, return -1
\******************************************************************************/
bool Locator::labelValue(String label, int& value)
	{
	bool ok = false;
	value = -1;
	if (_ctxList.size() > 0)
		{
		int contextId = -1;
		if (label.substr(0,1) != "@")
			contextId = _ctxList.back().contextId;
		
		if (_labels.count(contextId))
			{
			if (_labels[contextId].count(label))
				{
				value = _labels[contextId][label];
				ok 	  = true;
				}
			}
		}
		
	return ok;
	}

/******************************************************************************\
|* Return all the context label values
\******************************************************************************/
String Locator::labelValues(void)
	{
	String results = "";
	
	if (_ctxList.size() > 0)
		{
		int contextId = _ctxList.back().contextId;
		
		if (_labels.count(contextId))
			for (Elements<String,int>kv : _labels[contextId])
				results += kv.key + " = " + std::to_string(kv.value) + "\n";
		}
		
	return results;
	}
	
#pragma mark - Private methods


/******************************************************************************\
|* Private method : Return a human-readable version of the type
\******************************************************************************/
String Locator::_type(ContextType type)
	{
	String name = "Unknown";
	
	switch (type)
		{
		case C_FILE:
			name = "file";
			break;
		case C_MACRO:
			name = "macro";
			break;
		case C_FUNCTION:
			name = "function";
			break;
		case C_CLASS:
			name = "class";
			break;
		case C_METHOD:
			name = "method";
			break;
		case C_BLOCK:
			name = "block";
			break;
		case C_IF:
			name = "if";
			break;
		case C_WHILE:
			name = "while";
			break;
		default:
			FATAL(ERR_CTX, "Asked for unknown context type\n");
		}
		
	return name;
	}
