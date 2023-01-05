#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>

#include <cstdarg>
#include <map>

#include "properties.h"
#include "debug.h"

/*****************************************************************************\
|* Simulator definition
\*****************************************************************************/
class Simulator : public QObject
	{
		Q_OBJECT

		public:
			/*********************************************************************\
			|* Memory value returns
			\*********************************************************************/
			enum
				{
				INVALID_ADDRESS = 0x100
				};

			/*********************************************************************\
			|* Flag bit values
			\*********************************************************************/
			enum {
				 FLAG_C = 0x01,
				 FLAG_Z = 0x02,
				 FLAG_I = 0x04,
				 FLAG_D = 0x08,
				 FLAG_B = 0x10,
				 FLAG_V = 0x40,
				 FLAG_N = 0x80
				};

			/*********************************************************************\
			|* Memory states
			\*********************************************************************/
			typedef enum
				{
				MS_UNDEFINED	= 1,
				MS_ROM			= 2,
				MS_INVALID		= 4,
				MS_CALLBACK		= 8
				} MemoryState;

			/*********************************************************************\
			|* Debug level
			\*********************************************************************/
			typedef enum
				{
				DBG_NONE		= 0,
				DBG_MESSAGE,
				DBG_TRACE
				} DebugLevel;

			/*********************************************************************\
			|* Simulator error levels. Allows simulation to return on only
			|* certain errors
			\*********************************************************************/
			typedef enum
				{
				EL_NONE			= 0, // Only return on unhandled errors, ie: BRK,
									 // illegal instructions, undefined memory

				EL_MEMORY,			 // Also return on most memory errors, ignore
									 // write-to-ROM and read-uninitialised

				EL_FULL,			 // Return on all errors

				EL_DEFAULT = EL_MEMORY
				} ErrorLevel;

			/*********************************************************************\
			|* Simulator error codes
			\*********************************************************************/
			typedef enum
				{						// Error level
				E_NONE			= 0,
				E_EX_UNDEF		= -1,   // 0
				E_EX_UNINIT		= -2,   // 1
				E_RD_UNDEF		= -3,   // 1
				E_RD_UNINIT		= -4,   // 2
				E_WR_UNDEF		= -5,   // 1
				E_WR_ROM		= -6,   // 2
				E_BREAK			= -7,   // 0
				E_INVALID_INSN	= -8,   // 0
				E_CALL_RET		= -9,   // 0
				E_CYCLE_LIMIT	= -10,  // 0
				E_USER			= -11   // 0
				} ErrorCode;

			/*********************************************************************\
			|* Callback types
			\*********************************************************************/
			typedef enum
				{
				CB_READ			= 0,
				CB_WRITE,
				CB_EXEC
				} CallbackType;

			/*********************************************************************\
			|* Processor registers
			\*********************************************************************/
			typedef struct
				{
				uint16_t pc;		// Program counter
				uint8_t a;			// Accumulator
				uint8_t x;			// X register
				uint8_t y;			// Y register
				uint8_t s;			// stack pointer
				uint8_t p;			// processor status register
				uint8_t p_valid;	// Which flags in the status are ok to use
				} Registers;

			/*********************************************************************\
			|* Profiling parameters
			\*********************************************************************/
			typedef struct
				{
				uint64_t *cycles;		// Total # cycles executing this insn
				uint64_t *branch;		// Times this branch was taken
				uint64_t *extra;		// Number of extra cycles for crossing pages
				uint64_t *mflag;		// Number of times this ins actually modifies flags
				uint64_t branch_skip;   // Number of branches skipped
				uint64_t branch_taken;  // Number of branches taken
				uint64_t branch_extra;  // Extra cycles per branch to other page
				uint64_t abs_x_extra;   // Extra cycles per ABS,X crossing page
				uint64_t abs_y_extra;   // Extra cycles per ABS,Y crossing page
				uint64_t ind_y_extra;   // Extra cycles per (),Y crossing page
				uint64_t instructions;  // Number of instructions
				} ProfileData;

			/*********************************************************************\
			|* Type of map used for label mapping address:string
			\*********************************************************************/
			typedef std::map<uint32_t, String> AddressMap;

			/*********************************************************************\
			|* std::bind target for callbacks
			\*********************************************************************/
			typedef std::function<ErrorCode(Simulator* sim,
											uint32_t addr,
											int data)> SIM_CB;

		/*************************************************************************\
		|* Properties
		\*************************************************************************/
		GETSET(DebugLevel, debug, Debug);			// Debugging level for the sim
		GET(ErrorCode, error);						// Any current error code
		GETSET(ErrorLevel, errorLevel, ErrorLevel);	// Error level to return on
		GET(uint32_t, errorAddress);				// Address of error
		GET(uint64_t, cycles);						// Number of cycles passed
		GETSET(bool, doProfiling, DoProfiling);		// Whether to profile
		GET(Registers, regs);						// Registers in the CPU
		GETSET(int, maxRam, MaxRam);				// Amount of memory to offer
		GET(ProfileData, profileData);				// Profiling data
		GET(bool, writeMem);						// Profiler: detect writes
		GETSET(AddressMap, labels, Labels);			// Assembly labels
		GETSET(FILE*, traceFile, TraceFile);		// Where to trace to

		/*************************************************************************\
		|* Internal state
		\*************************************************************************/
		private:
			/*********************************************************************\
			|* Simulator state
			\*********************************************************************/
			uint8_t * _mem;							// RAM
			uint8_t * _memState;					// RAM state
			SIM_CB * _readCbs;						// Read callbacks
			SIM_CB * _writeCbs;						// Write callbacks
			SIM_CB * _execCbs;						// Execute callbacks
			int _cycleLimit;						// Limit on simulation time

			/*********************************************************************\
			|* Set the processor status flags with a mask
			\*********************************************************************/
			uint8_t _getFlags(uint8_t mask);

			/*********************************************************************\
			|* Return any known label for this address
			\*********************************************************************/
			const String& _getLabel(uint32_t address);


			/*********************************************************************\
			|* Read (PC); set appropriate error status
			\*********************************************************************/
			uint8_t _readPC(void);

			/*********************************************************************\
			|* Read a byte at an address; set appropriate error status
			\*********************************************************************/
			uint8_t  _readByte(uint32_t addr);

			/*********************************************************************\
			|* Read a byte using the X-indexed addressing mode
			\*********************************************************************/
			uint8_t  _readIndX(uint32_t addr);

			/*********************************************************************\
			|* Read a byte using the Y-indexed addressing mode
			\*********************************************************************/
			uint8_t  _readIndY(uint32_t addr);

			/*********************************************************************\
			|* Read a word at an address; set appropriate error status
			\*********************************************************************/
			uint16_t _readWord(uint32_t addr);

			/*********************************************************************\
			|* Write a byte to an address; set appropriate error status
			\*********************************************************************/
			void _writeByte(uint32_t address, uint8_t val);

			/*********************************************************************\
			|* Write a byte using the X-indexed addressing mode
			\*********************************************************************/
			void _writeIndX(uint32_t address, uint8_t val);

			/*********************************************************************\
			|* Write a byte using the Y-indexed addressing mode
			\*********************************************************************/
			void _writeIndY(uint32_t address, uint8_t val);


			/*********************************************************************\
			|* Instructions
			\*********************************************************************/
			void _adc(uint8_t val);
			void _sbc(uint8_t val);
			void _branch(int8_t off, uint8_t mask, int cond);
			void _bit(uint32_t address);
			void _jsr(uint32_t address);
			void _rts(void);
			void _rti(void);

			/*********************************************************************\
			|* Extra cycles due to page boundaries
			\*********************************************************************/
			void _handleExtraAbsoluteX(uint32_t address);
			void _handleExtraAbsoluteY(uint32_t address);


			/*********************************************************************\
			|* Runtime: process the next instruction
			\*********************************************************************/
			void _next(void);

			/*********************************************************************\
			|* Trace printing: trace the execution
			\*********************************************************************/
			void _traceRegs(void);
			void _printCurrentInsn(uint16_t pc, char *buf, int hint);
			void _printMemCount(char *buf, uint32_t address, unsigned len);
			void _printMem(char *buf, uint32_t addess);

			char * _printLabelMax(char *buf, const String& label, int max);
			char * _printIndLabel(char *buf, uint16_t address, char idx, int hint);
			char * _printZpLabel(char *buf, uint16_t address, char idx);
			char * _printAbsLabel(char *buf, uint16_t address, char idx);

			char * _hex2(char *c, uint8_t x);
			char * _hex4(char *c, uint16_t x);
			char * _hex8(char *c, uint32_t x);

		public:
			/*********************************************************************\
			|* Constructor / Destructor
			\*********************************************************************/
			explicit Simulator(int maxRam = 0x10000,
							   QObject *parent = nullptr);
			~Simulator(void);

			/*********************************************************************\
			|* Add a callback
			\*********************************************************************/
			void addCallback(SIM_CB cb, uint32_t address, CallbackType type);

			/*********************************************************************\
			|* Add a callback over a range of addresses
			\*********************************************************************/
			void addCallback(SIM_CB cb,
							uint32_t address,
							uint32_t len,
							CallbackType type);

			/*********************************************************************\
			|* Determine if we ought to exit based on the error
			\*********************************************************************/
			bool shouldExit(void);

			/*********************************************************************\
			|* Log a warning if the debuglevel is high enough
			\*********************************************************************/
			int warn(const char *format, ...);

			/*********************************************************************\
			|* Log an error
			\*********************************************************************/
			int error(const char *format, ...);

			/*********************************************************************\
			|* Log an error
			\*********************************************************************/
			String errorString(ErrorCode e);

			/*********************************************************************\
			|* Set the current error status
			\*********************************************************************/
			void setError(ErrorCode e, uint32_t address);

			/*********************************************************************\
			|* Set the processor status flags with a mask
			\*********************************************************************/
			void setFlags(uint8_t mask, uint8_t value);

			/*********************************************************************\
			|* Set the cycle limit
			\*********************************************************************/
			void setCycleLimit(uint64_t limit);

			/*********************************************************************\
			|* Add memory to the simulator
			\*********************************************************************/
			void addRAM(uint32_t address, uint32_t length, bool zero=false);

			/*********************************************************************\
			|* Add initialised RAM memory to the simulator
			\*********************************************************************/
			void addRAM(uint32_t address, uint8_t *data,  uint32_t length);

			/*********************************************************************\
			|* Add initialised RAM memory to the simulator
			\*********************************************************************/
			void addROM(uint32_t address, uint8_t *data,  uint32_t length);

			/*********************************************************************\
			|* Get a byte of memory with a check. Returns INVALID_ADDRESS if not
			|* a valid address to read
			\*********************************************************************/
			int getByte(uint32_t address);

		};

#endif // SIMULATOR_H
