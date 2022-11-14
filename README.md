# xtal
eXTended Atari Language - a language to take advantage of the extended XL/XE FPGA board

## Introduction
This project is the recognition that extending the hardware on an XL/XE isn't enough to be useful in and of itself. We need to be able to *use* the hardware. 

I've been noodling off and on regarding creating a new programming language for the 8-bits h.. for about a decade or so. The thing is that I look at Action! or cc65 and ask myself "is this likely to be better than that", and the answer is always "probably not".

But now we have [some custom hardware](https://forums.atariage.com/topic/275693-to-infinity-and-beyond-new-hardware/page/9/#comments) to drive, and that adds a whole bunch of new opportunities... I think one of the obvious benefits to using the board is the memory management. It seems to me that there's a couple of things that could really help out the 6502:

- A more-capable ALU. The Apple-II actually had a built-in virtual-machine that was 16-bit ("Sweet 16") to provide a general purpose way of using 16-bit arithmetic, and to reduce complexity when writing more involved code. I'm proposing to extend that extension, and introduce a simple 8,16 and 32 bit environment that's easily accessible via the 6502.

- There is a lot of memory on the FPGA board. More than any 6502 could realistically use. As a matter of course, I'm planning on having the memory in page-0 served by the DRAM on the board rather than from the internals of the machine.


##Memory

BASIC takes over the top half of page-0. Getting rid of the need for BASIC (and providing an alternative) therefore gives a huge landscape of page-0 space for use and abuse and the fine-grained remapping abilities are what makes this whole idea really appealing. The proposal is (this is from the top of my RegisterFile.h header :)


```
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
|*		$1C,$1D		: 65536 sets of 8K banks to function RAM at $6000 .. $7FFF
|*		$1E,$1F		: 65536 sets of 8K banks to data/fn RAM at $8000 .. $9FFF
|*		$80			: Page index for $C0..$CF  } compiler uses values here as
|*		$81			: Page index for $D0..$DF  } 8, 16, or 32-bit vars as
|*		$82			: Page index for $E0..$EF  } necessary
|*		$83			: Page index for $F0..$FF
|*		$84			: varargs count,
|*		$85			: varargs page index (vars start @0)
|*		$87			: cmd buffer
|*		$88			: cmd fifo #1
|*		$89			: cmd fifo #2
|*		$8A..$9F	: global vars (not swapped, 22 bytes)
|*		$A0..$AF	: 16 bytes of function-return-and-args. Not swapped
|*	  * $B0..$BF	: 16 bytes of function scratch-space [indexed by $1C,$1D].
|*								Not preserved over fn-calls
|*	  * $C0..$FF	: page-indexed variables [8,16,32-bit]
|*	
|*	  * means these locns are affected by zero-page indices ($80…$83 or $1C,D)
|*
\*****************************************************************************/
```

So let's go through this...

- The two pairs (\$1C,\$1D) and (\$1E,\$1F) allow remapping of the fixed address-ranges identified above, just by writing to these zero-page memory locations. Put \$0000 into {\$1C,\$1D} and you have the "normal" memory there, put \$0001 into the pair, and the memory at \$6000..\$7FFF is now the next bank up. Jumping ahead slightly, but it simultaneously remaps \$B0..\$BF as well, so each set of 8K "function space" can have 16 bytes of dedicated zero-page RAM associated with it. {\$1E,\$1F} does the same for the RAM at \$8000..\$9FFFF, and while this is "intended" to be for data (and note that screen-memory would fit here too) rather than functions, if you happen to have a function that's >8K, you could use both areas for functions.
- The next 4 bytes (\$80..\$83) each control 16 bytes in page-0. They allow remapping each section of 16 bytes to any of 256 possible locations - this gives you 4 x 16 x 256 bytes of zero-page RAM, or 16K. That's a lot of fast-access variables.
- There's a couple of dedicated bytes for varargs calls, which can supply up to 32 bytes of var-args to a function call. I think that's quite a lot, really, given that strings/arrays will be represented by pointers (2 bytes) rather than inline data.
- There is a command-buffer byte, which accepts a command (0..256) and whenever any byte is written to the fifo #1, that command will execute. If the command needs more than one byte, they are written to fifo #2 until there's only one left, and then write to fifo #1.
- Then (\$86..\$9F) there's 22 bytes of un-swapped data, free for the compiler to associate as it sees fit. I'm thinking global variable stuff here - with a "usage" heuristic to determine which are the lucky dedicated variables to use. I'll do register-colouring to see if we can re-use any of them at different times (eg: loop vars) 
- Then(\$A0..\$AF)  we have 16 bytes of function interchange memory. I think 16 bytes is pretty reasonable. The compiler ought to be able to know what the function signatures are, so it can pack in bytes, words, longs etc. efficiently. Anything that spills over can go on the stack, or maybe it's just a language limitation, and you ought to pass a pointer to a class instead. This memory will be used for passing return-values from a function back as well, and since I intend to allow multiple return values, the same rules apply to both in and out values.
- Then (\$B0..\$BF) there's the function scratch-space that's per-function
- Finally (\$C0..\$FF) you have the remappable memory for lots of zero-page variables.

## Command FIFO

The buffer / arguments at {\$87..\$89} allow commands to be piped from the 6502 to the FPGA. Generally, I'm intending this as the interface to speed up things that take a while on the 6502 (example: floating point multiply) and offload that to the FPGA (or even perhaps the RP2040). Instead of executing the code to perform the task, the 6502 does a few load/stores to the fifo areas, and the effects are stored in the 'registers' that the 6502 can then read to get a result.

It can however be used for other things, examples might be:

```
  
	Eg: clear register 0. 5 clocks to clear 4 bytes, not 14
		LDA #ZEROZERO.4
		STA $87		
  
	Eg: clear register 2. 10(5) clocks to clear 4 bytes, not 14
		LDA #ZERO.4
		STA $87
		LDA #2
		STA $88
		
	Eg: clear registers 0-7. 15(10) clocks to clear 32 bytes, not 98
		LDA #ZERO_MULTIPLE.4
		STA $87
		LDA #7
		STA $89
		LDA #0
		STA $88
	
	Eg: copy register 1 to register 2, 15(10) clocks, not 20
		LDA #MOVE.4
		STA $87
		LDA #2
		STA $89
		LDA #1
		STA $88
```

## ALU

Back in the day, the Apple-II had a '[Sweet16](https://en.wikipedia.org/wiki/SWEET16)' VM built into the ROM logic, which would
pretend to be a 16-bit CPU, implemneted using the 6502. Similarly, for the arcade version of BattleZone, Atari implemented an external "[math box](https://6502disassembly.com/va-battlezone/mathbox.html)" ALU for the 6502 so it could keep up with the 3D geometry requirements. 

In the spirit of that, I'm planning on introducing an external ALU (driven by the FPGA). If we adhere to the above definition of the registers r0..r15, then the assembler directive

```
div.4 r8, r9
```

would be implemented as 

```
; div.4 r8 r9
LDA #OP_DIV32
STA $86
LDA #9 
STA $88
LDA #8
STA $87
```

... with every successive store to \$87 implementing the 32-bit divide operation (to be useful, there probably ought to be other interspersed calls :). Given the clock-speed disparity between modern architectures and the 6502, the speed-up ought to be significant - I'd expect all results for simple maths operations to be available on the next 6502 clock cycle.

So there you have it, the reason for a new language for the A8 in this day and age. The assembler is mostly written ([feature list](/xtal-a/Docs/AssemblyFormat.txt)), as I type, meaning I can start to pay more attention to the higher-level language.
