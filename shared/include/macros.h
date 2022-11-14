//
//  macros.h
//  dbio
//
//  Created by Thrud The Barbarian on 7/17/20.
//

#ifndef macros_h
#define macros_h

#include <stdlib.h>
#include "debug.h"

/*****************************************************************************\
|* These will release the memory they point to (from various allocators) and
|* then null out the pointer
\*****************************************************************************/
#define FREE(x)						do 								\
										{ 							\
										::free((void *)x); 			\
										x = nullptr; 				\
										} 							\
									while (false)

#define DELETE(x)					do 								\
										{ 							\
										delete x;			 		\
										x = nullptr; 				\
										} 							\
									while (false)

#define DELETE_ARRAY(x)				do 								\
										{ 							\
										delete [] x; 				\
										x = nullptr; 				\
										} 							\
									while (false)

/*****************************************************************************\
|* Xcode provides these sometimes, depending on whether you include some magic
|* header file...
\*****************************************************************************/
#ifndef MIN
#	define MIN(x,y)  (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#	define MAX(x,y)  (((x) > (y)) ? (x) : (y))
#endif

#ifndef ABS
#	define ABS(x)    (((x) < 0) ? -(x) : (x))
#endif

/*****************************************************************************\
|* It's just *so* hard to leave ObjC behind...
\*****************************************************************************/
#ifndef YES
#   define YES true
#endif

#ifndef NO
#   define NO false
#endif

/*****************************************************************************\
|* This really ought to have been a part of the standard :)
\*****************************************************************************/
#define forever for(;;)

/*****************************************************************************\
|* Use RIAA to hold a lock
\*****************************************************************************/
#if THREADSAFE_COLLECTIONS
#  define GUARD	std::lock_guard<std::recursive_mutex> lock(_mutex)
#else
#  define GUARD
#endif

/*****************************************************************************\
|* Timing macro between TICK and TOCK. The 'stt' must be co-defined to be the
|* same variable
\*****************************************************************************/
#define TICK(stt)   struct timeval stt;                                     \
                    gettimeofday(&stt, NULL)                                \
                            
#define TOCK(stt,end,dt,r)                                                  \
                    struct timeval end,dt;                                  \
                    gettimeofday(&end, NULL);                               \
                    timersub(&end, &stt, &dt);                              \
                    double r = dt.tv_sec + ((double)(dt.tv_usec)/1000000)   \


#endif /* macros_h */
