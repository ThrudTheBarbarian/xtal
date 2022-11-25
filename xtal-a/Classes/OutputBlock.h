//
//  OutputBlock.h
//  as8
//
//  Created by Thrud The Barbarian on 10/31/22.
//

#ifndef OutputBlock_h
#define OutputBlock_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

class Token;

class OutputBlock
	{
	/************************************************************************\
    |* Properties
    \************************************************************************/
    GET(int, baseAddress);			// Where this block starts
    
    private:
		std::vector<uint8_t> _data;	// Actual data
    public:
        /********************************************************************\
        |* Constructors and Destructor
        \********************************************************************/
        explicit OutputBlock(int baseAddress);

        /********************************************************************\
        |* Add a token into the block
        \********************************************************************/
        void add(Token& token);

        /********************************************************************\
        |* Write the length to the block
        \********************************************************************/
        void finalise(void);

        /********************************************************************\
        |* Write the block to a file
        \********************************************************************/
        void write(FILE *fp);
        
        /********************************************************************\
        |* Write the block to a file as hex data
        \********************************************************************/
        void writeHex(FILE *fp);
	};

typedef std::vector<OutputBlock> BlockList;

#endif /* OutputBlock_h */
