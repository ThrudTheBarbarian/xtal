//
//  structures.h
//  dbio
//
//  Created by Thrud The Barbarian on 7/18/20.
//

#ifndef structures_h
#define structures_h

#include <map>

#include <stdint.h>
#include <string>
#include <vector>

/*****************************************************************************\
|* Basically a clone of NSRange
\*****************************************************************************/
typedef struct Range
	{
	uint64_t    location;		// first index
	int64_t     length;			// length of range
 
    Range(uint64_t offset, int64_t len)
        {
        location = offset;
        length   = len;
        }
    
    Range()
        {
        location    = 0;
        length      = 0;
        }
        
	} Range;

#define NotFound        ((uint64_t)-1)

/*****************************************************************************\
|* Used to iterate over a map using the for: approach, as in:
|*
|*  for (Elements<int, std::string> kv : my_map )
|*		{
|*		std::cout << kv.key << " --> " << kv.value;
|*		}
\*****************************************************************************/
template <class K, class T>
struct Elements {
    K const& key;
    T& value;
 
    Elements(std::pair<K const, T>& pair)
        : key(pair.first)
        , value(pair.second)
    { }
};

typedef std::string String;
typedef std::vector<String> StringList;

#endif /* structures_h */
