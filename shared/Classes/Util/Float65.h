//
//  Float65.h
//  float65
//
//  Created by Simon Gornall on 1/21/23.
//

#ifndef Float65_h
#define Float65_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Float65
	{
    NON_COPYABLE_NOR_MOVEABLE(Float65)

	public:
		typedef struct
			{
			union
				{
				uint8_t b[4];
				uint32_t l;
				};
			} f32;

	/************************************************************************\
    |* Properties
    \************************************************************************/
    
    private:
        f32 _val;
		
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit Float65();
        explicit Float65(String hex);
        explicit Float65(const float f);

        /********************************************************************\
        |* Convert a hex string to F65 format
        \********************************************************************/
		void set(String hex);

        /********************************************************************\
        |* Convert a float to F65 format
        \********************************************************************/
		void set(float f);
		
        /********************************************************************\
        |* Convert F65 format to ascii
        \********************************************************************/
        String toString(void);
        
        /********************************************************************\
        |* Convert F65 format to float
        \********************************************************************/
        float toFloat(void);
        
	};

#endif /* Float65_h */
