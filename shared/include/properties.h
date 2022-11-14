//
//  properties.h
//  dbio
//
//  Created by Thrud The Barbarian on 7/17/20.
//

#ifndef properties_h
#define properties_h

#include <inttypes.h>

/*****************************************************************************\
|* Getter / Setter macros for properties within classes.
\*****************************************************************************/
#define GETSET(type, var, accessor)			\
	protected:								\
	   type _##var;							\
	public:									\
       inline type& var()					\
	   {									\
		  return _##var;					\
	   }									\
       inline void set##accessor(type val)	\
	   {									\
		  _##var = val;						\
	   }

#define GETSETPUBLIC(type, var, accessor)	\
	public:									\
	   type _##var;							\
       inline type& var()					\
	   {									\
		  return _##var;					\
	   }									\
       inline void set##accessor(type val)	\
	   {									\
		  _##var = val;						\
	   }

#define SET(type, var, accessor)			\
	protected:								\
	   type _##var;							\
	public:									\
       inline void set##accessor(type &val)	\
	   {									\
		  _##var = val;						\
	   }

#define GET(type, var)						\
	protected:								\
	   type _##var;							\
	public:									\
       inline type& var()					\
	   {									\
		  return _##var;					\
	   }

/*****************************************************************************\
|* Make a class non-copiable or moveable
\*****************************************************************************/
#define NON_COPYABLE_NOR_MOVEABLE(T)		\
	  T(T const &) = delete;				\
	  void operator=(T const &t) = delete;	\
	  T(T &&) = delete;
	  
/*****************************************************************************\
|* Make a class non-copiable
\*****************************************************************************/
#define NON_COPYABLE(T)						\
	  T(T const &) = delete;				\
	  void operator=(T const &t) = delete;

#include "structures.h"

#endif /* properties_h */
