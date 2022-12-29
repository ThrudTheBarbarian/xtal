//
//  RegisterFile.h
//  xtal
//
//  Created by Thrud The Barbarian on 10/26/22.
//
/*****************************************************************************\
|* Registers
|* =========
|*
|* Registers on the 6502 are ... limited. The compiler therefore uses zero-page
|* space as storage for generic 32-bit (int,float), 16-bit (int) and 8-bit (int)
|* "registers", keeping track of what has been used where.
|*
|* The expansion board also allows the remapping of page-0 to increase the
|* number of registers available. Memory use is as follows:
|*
|*	$1C,$1D		: 65536 sets of 8K banks to function RAM at $6000 .. $7FFF
|*	$1E,$1F		: 65536 sets of 8K banks to data/fn RAM at $8000 .. $9FFF
|*	$80			: Page index for $C0..$CF  } compiler uses values here as
|*	$81			: Page index for $D0..$DF  } 8, 16, or 32-bit vars as
|*	$82			: Page index for $E0..$EF  } necessary
|*	$83			: Page index for $F0..$FF
|*	$84			: varargs count,
|*	$85			: cmd buffer
|*	$86			: fifo #1
|*	$87			: fifo #2
|*	$88..$8B	: floating point reg #1
|*	$8C..$8F	: floating point reg #2
|*	$90..$91	: Stack pointer
|*	$92..$9F	: global vars (not swapped, 14 bytes)
|*	$A0..$AF	: 16 bytes of function-return-and-args. Not swapped
|*  * $B0..$BF	: 16 bytes of function scratch-space [indexed by $1C,$1D].
|*				  Not preserved over fn-calls
|*  * $C0..$EF	: page-indexed variables [8,16,32-bit]
|*  * $F0..$FF	: float workspace (stock) or swapped (extended) vars
|*
|* >  means these variables are affected by the indices in zero-page
|*    ($80…$83 or $1C,D)
|*
\*****************************************************************************/
#ifndef RegisterFile_h
#define RegisterFile_h

#include <cstdio>
#include <string>

#include "properties.h"
#include "macros.h"

#include "Register.h"

class RegisterFile
	{
    NON_COPYABLE_NOR_MOVEABLE(RegisterFile)
 
	/***************************************************************************\
    |* Properties
    \***************************************************************************/
    
    private:
        /********************************************************************\
        |* Fill up (or clear) the register map space for a given register
        \********************************************************************/
        static void _populate(Register &r, bool clear = false);
		
        /********************************************************************\
        |* Find space in the register file for another register
        \********************************************************************/
        static int _findSpace(int bytes);
		
    public:
        /***********************************************************************\
        |* Constructors and Destructor
        \***********************************************************************/
        explicit RegisterFile();

        /***********************************************************************\
        |* Set the file pointer for assembly hinting
        \***********************************************************************/
        static void setOutputFile(FILE *fp);

        /***********************************************************************\
        |* free all registers
        \***********************************************************************/
        static void clear(void);
        
        /***********************************************************************\
        |* Allocate a register
        \***********************************************************************/
        static Register allocate(Register::RegType type);
        static Register allocateForPrimitiveType(int ptype);
        
        /***********************************************************************\
        |* Set the file pointer for assembly hinting
        \***********************************************************************/
        static bool widen(Register& reg, int oldWidth, int newWidth);
        
        /***********************************************************************\
        |* Free up a register
        \***********************************************************************/
        static bool free(Register& reg);
            
        /***********************************************************************\
        |* Debugging: dump out register allocations
        \***********************************************************************/
        static void dump(void);
    
	};

#endif /* RegisterFile_h */
