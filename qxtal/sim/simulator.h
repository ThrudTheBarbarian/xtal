#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>

#include <cstdarg>
#include <map>

#include "properties.h"
#include "debug.h"

/*****************************************************************************\
|* Client interface definition
\*****************************************************************************/
class Simulator;
typedef std::function<void(const Simulator& sim, uint32_t addr, int data)> SIM_CB;

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

		signals:

		};

#endif // SIMULATOR_H
