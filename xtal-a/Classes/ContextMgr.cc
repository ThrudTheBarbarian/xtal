//
//  ContextMgr.cc
//  xtal-a
//
//  Created by Simon Gornall on 11/17/22.
//

#include "ContextMgr.h"
#include "Stringutils.h"

// FIXME: Contexts aren't storing line numbers correctly

/******************************************************************************\
|* The global notification center
\******************************************************************************/
std::shared_ptr<ContextMgr> ContextMgr::_instance = NULL;

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
ContextMgr::ContextMgr()
	{
	}

/******************************************************************************\
|* Return the shared instance
\******************************************************************************/
std::shared_ptr<ContextMgr> ContextMgr::sharedInstance(void)
    {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (!_instance)
        _instance = std::shared_ptr<ContextMgr>(new ContextMgr);

    return _instance;
    }

/******************************************************************************\
|* Create and push the current context
\******************************************************************************/
ContextMgr::Context& ContextMgr::push(const String name,
									  ContextMgr::Type type,
									  int64_t line)
	{
	Context ctx = {.name = name,
				   .type = type,
				   .line = line};
	
	_ctxList.push_back(ctx);
	switch (type)
		{
		case FILE:
			_fileList.push_back(&_ctxList.back());
			break;
		
		default:
			_blockList.push_back(&_ctxList.back());
			break;
		}
		
	return _ctxList.back();
	}
/******************************************************************************\
|* Create and push the current context
\******************************************************************************/
ContextMgr::Context& ContextMgr::push(const String name,
									  String type,
									  int64_t line)
	{
	Type typeId = UNKNOWN;
	String lc = lcase(type);
	if (lc == "file")
		typeId = FILE;
	else if (lc == "macro")
		typeId = MACRO;
	else if (lc == "function")
		typeId = FUNCTION;
	else if (lc == "class")
		typeId = CLASS;
	else if (lc == "method")
		typeId = METHOD;
	else if (lc == "block")
		typeId = BLOCK;
		
	if (typeId != UNKNOWN)
		return push(name, typeId, line);
	FATAL(ERR_CTX, "Unknown context-type '%s' encountered", type.c_str());
	}

/******************************************************************************\
|* Increment the current line number within the current context
\******************************************************************************/
int64_t ContextMgr::incLine(int64_t delta)
	{
	int64_t currentCtxLine = -1;
	
	if (_fileList.size() > 0)
		{
		_fileList.back()->line += delta;
		currentCtxLine = _fileList.back()->line;
		}
		
	if (_blockList.size() > 0)
		{
		_blockList.back()->line += delta;
		currentCtxLine = _blockList.back()->line;
		}
	
	return currentCtxLine;
	}

/******************************************************************************\
|* Reset all the contexts
\******************************************************************************/
void ContextMgr::reset()
	{
	_ctxList.clear();
	_fileList.clear();
	_blockList.clear();
	}
	
/******************************************************************************\
|* Pop a context
\******************************************************************************/
void ContextMgr::pop(void)
	{
	if (_ctxList.size() > 0)
		{
		Context ctx = _ctxList.back();
		switch (ctx.type)
			{
			case FILE:
				_fileList.pop_back();
				break;
				
			default:
				_blockList.pop_back();
				break;
			}
		_ctxList.pop_back();
		}
	else
		FATAL(ERR_CTX, "Cannot find context to pop!");
	}

/******************************************************************************\
|* Return a location based on the current context hierarchy. \******************************************************************************/
String ContextMgr::location(void)
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
String ContextMgr::identifier(int idx)
	{
	String ident = "Uninitialised";
	
	if (_ctxList.size() > 0)
		{
		Context ctx 	= _ctxList.back();
		String type 	= ucase(_type(ctx.type));
		
		ident			= type.substr(0,1)
						+ "_"
						+ ctx.name
						+ std::to_string(idx);
		
		}
	else
		FATAL(ERR_CTX, "Cannot find context to pop!");
	
	return ident;
	}
	
#pragma mark - Private methods


/******************************************************************************\
|* Private method : Return a human-readable version of the type
\******************************************************************************/
String ContextMgr::_type(ContextMgr::Type type)
	{
	String name = "Unknown";
	
	switch (type)
		{
		case FILE:
			name = "file";
			break;
		case MACRO:
			name = "macro";
			break;
		case FUNCTION:
			name = "function";
			break;
		case CLASS:
			name = "class";
			break;
		case METHOD:
			name = "method";
			break;
		case BLOCK:
			name = "block";
			break;
		}
		
	return name;
	}