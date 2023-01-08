//
//  OutputBlock.h
//  xtal-a
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
    GETSET(bool, isData, IsData);	// If this block contains no code
    
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
        |* Add raw data into the block
        \********************************************************************/
        void add(char *data, int len);
        void add(const char *data, int len);
        void add(uint8_t *data, int len);
        void add(const uint8_t *data, int len);

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
        
        /********************************************************************\
        |* Return the number of bytes in the block
        \********************************************************************/
        int size(void);
        
        /********************************************************************\
        |* Reset the block so it can be re-used
        \********************************************************************/
        void clear(void);
        
        /********************************************************************\
        |* Add the 16-bit checksum to the end of the block
        \********************************************************************/
        void addChecksum(void);
        
	};

typedef std::vector<OutputBlock> BlockList;

#endif /* OutputBlock_h */
