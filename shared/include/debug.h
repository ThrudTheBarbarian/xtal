//
//  debug.h
//  dbio
//
//  Created by Thrud The Barbarian on 7/17/20.
//

#ifndef debug_h
#define debug_h

#include <iostream>
#include <cstdio>
#include <cstring>
#include <sys/time.h>

#ifndef __FILENAME__
#  define __FILENAME__ (strrchr(__FILE__, '/')                      \
                        ? strrchr(__FILE__, '/') + 1                \
                        : __FILE__)
#endif

/*****************************************************************************\
|* Some level of cross-platform support
\*****************************************************************************/
#if defined(DEBUG)
#   if defined(_MSC_VER)
#       // "(void)0," is for suppressing C4127 warning in
#       // "assert(false)", "assert(true)" and the like
#       define DBG_ASSERT( x )       if ( !((void)0,(x))) { __debugbreak(); }
#                                    //if ( !(x)) WinDebugBreak()
#   else
#       include <assert.h>
#       define DBG_ASSERT            assert
#   endif
#else
#   define DBG_ASSERT( x )           {}
#endif

/*****************************************************************************\
|* The intent here is that logging should be done via macros, so that any
|* given logger can be plugged in by redefining the macro at some later
|* stage. The macros here just dump to stdout
\*****************************************************************************/

enum
	{
	DEBUG_NONE		= 0,
	DEBUG_DEFAULT	= 1,
	DEBUG_CHATTY	= 2,
	DEBUG_VERBOSE	= 3
	};

# define MSG(...) 													\
	{																\
    char s_str[65535];												\
    snprintf(s_str, 65535, __VA_ARGS__);							\
    std::cout << s_str << std::endl;								\
    }

# define LOG(...) 													\
	{																\
    char s_str[65535];												\
    snprintf(s_str, 65535, __VA_ARGS__);							\
    std::cout << "[" << __FILENAME__ << " "						    \
			  << __FUNCTION__ << ": " << __LINE__ << "] " 		    \
			  << s_str << std::endl;								\
    }

# define FATAL(error, ...) 											\
	{																\
    char s_str[65535];												\
    snprintf(s_str, 65535, __VA_ARGS__);							\
    std::cout << "[" << __FILENAME__ << " "						    \
			  << __FUNCTION__ << ": " << __LINE__ << "] " 		    \
			  << s_str												\
			  << " at " << LOCATOR->location() 						\
			  << std::endl;											\
    std::exit(error);												\
    }

#define DBG_NONE(...)
#define DBG_DEFAULT(...)											\
    do {                                                            \
    if (debugLevel() > DEBUG_NONE)                       		 	\
        LOG(__VA_ARGS__)                                            \
    } while (0)

#define DBG_CHATTY(...)												\
    do {                                                            \
    if (debugLevel() > DEBUG_DEFAULT)                     			\
        LOG(__VA_ARGS__)                                            \
    } while (0)

#define DBG_VERBOSE(...)											\
    do {                                                            \
    if (debugLevel() > DEBUG_CHATTY)								\
        LOG(__VA_ARGS__)                                            \
    } while (0)


#if defined(DEBUG)

extern int debugLevel();

# define WARN(...)                                                  \
    {                                                               \
    if (debugLevel() > 0)                                           \
        LOG(__VA_ARGS__)                                            \
    }

# define INFO(...)                                                  \
    {                                                               \
    if (debugLevel() > 1)                                           \
        LOG(__VA_ARGS__)                                            \
    }
#else
# define WARN(...)
# define INFO(...)
#endif // DEBUG defined

enum
	{
	ERR_NO_SOURCE_FILES			= 1,
	ERR_LEX_BAD_CHAR,
	ERR_AST_UNKNOWN_TOKEN,
	ERR_AST_UNKNOWN_OPERATOR,
	ERR_AST_PRIORITY,
	ERR_AST_SYNTAX,
	ERR_REG_NOEXIST,
	ERR_REG_ALLOC,
	ERR_PARSE,
	ERR_EQUATE,
	ERR_EMIT,
	ERR_RUNTIME,
	ERR_ORIGIN,
	ERR_OUTPUT,
	ERR_INCLUDE,
	ERR_MACRO,
	ERR_META,
	ERR_IF,
	ERR_CTX,
	ERR_FUNCTION,
	ERR_TYPE,
	ERR_TOKEN,
	ERR_ARRAY,
	ERR_LOCATION,
	ERR_MAX
	};

#endif /* debug_h */
