#include "macros.h"
#include "simulator.h"


static const uint8_t insnLength[256] =
	{
	1,2,1,1,1,2,2,1,1,2,1,1,1,3,3,1, 2,2,1,1,1,2,2,1,1,3,1,1,1,3,3,1,
	3,2,1,1,2,2,2,1,1,2,1,1,3,3,3,1, 2,2,1,1,1,2,2,1,1,3,1,1,1,3,3,1,
	1,2,1,1,1,2,2,1,1,2,1,1,3,3,3,1, 2,2,1,1,1,2,2,1,1,3,1,1,1,3,3,1,
	1,2,1,1,1,2,2,1,1,2,1,1,3,3,3,1, 2,2,1,1,1,2,2,1,1,3,1,1,1,3,3,1,
	1,2,1,1,2,2,2,1,1,1,1,1,3,3,3,1, 2,2,1,1,2,2,2,1,1,3,1,1,1,3,1,1,
	2,2,2,1,2,2,2,1,1,2,1,1,3,3,3,1, 2,2,1,1,2,2,2,1,1,3,1,1,3,3,3,1,
	2,2,1,1,2,2,2,1,1,2,1,1,3,3,3,1, 2,2,1,1,1,2,2,1,1,3,1,1,1,3,3,1,
	2,2,1,1,2,2,2,1,1,2,1,1,3,3,3,1, 2,2,1,1,1,2,2,1,1,3,1,1,1,3,3,1
	};

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Simulator::Simulator(int maxRam,
					 QObject *parent)
		  :QObject{parent}
		  ,_maxRam(maxRam)
		  ,_traceFile(stderr)
	{
	/*************************************************************************\
	|* Set up the dynamic memory arrays
	\*************************************************************************/
	_mem				= new uint8_t[_maxRam];
	_memState			= new uint8_t[_maxRam];
	_readCbs			= new SIM_CB[_maxRam];
	_writeCbs			= new SIM_CB[_maxRam];
	_execCbs			= new SIM_CB[_maxRam];
	_profileData.cycles	= new uint64_t[_maxRam];
	_profileData.branch	= new uint64_t[_maxRam];
	_profileData.extra	= new uint64_t[_maxRam];
	_profileData.mflag	= new uint64_t[_maxRam];

	for (int i=0; i<_maxRam; i++)
		{
		_readCbs[i]		= nullptr;
		_mem[i]			= 0x0;
		_memState[i]	= MS_UNDEFINED | MS_INVALID;
		}

	/*************************************************************************\
	|* And other state
	\*************************************************************************/
	_regs.s			= 0xFF;
	_regs.p_valid	= 0xFF;
	setFlags(0xFF, 0x34);
	}

/*****************************************************************************\
|* Destructor
\*****************************************************************************/
Simulator::~Simulator(void)
	{
	DELETE_ARRAY(_mem);
	DELETE_ARRAY(_memState);
	DELETE_ARRAY(_readCbs);
	DELETE_ARRAY(_writeCbs);
	DELETE_ARRAY(_execCbs);
	DELETE_ARRAY(_profileData.cycles);
	DELETE_ARRAY(_profileData.branch);
	DELETE_ARRAY(_profileData.extra);
	DELETE_ARRAY(_profileData.mflag);
	}

/*****************************************************************************\
|* Add a callback to the memory flags
\*****************************************************************************/
void Simulator::addCallback(SIM_CB cb, uint32_t address, CallbackType type)
	{
	if (address < _maxRam)
		{
		_memState[address] |= MS_CALLBACK;
		switch (type)
			{
			case CB_READ:
				_readCbs[address] = cb;
				break;
			case CB_WRITE:
				_writeCbs[address] = cb;
				break;
			case CB_EXEC:
				_execCbs[address] = cb;
				break;
			default:
				WARN("Unknown callback type %d requested!", type);
			}
		}
	else
		WARN("Attempt to add callback to 0x%x, which is > max ram (0x%x)",
			 address, _maxRam);
	}


/*****************************************************************************\
|* Log a warning if the debuglevel is high enough
\*****************************************************************************/
int Simulator::warn(const char *format, ...)
	{
	char buf[16384];
	int size;
	va_list ap;

	if (_debug >= DBG_MESSAGE)
		{
		/*************************************************************************\
		|* Print original message to a string
		\*************************************************************************/
		va_start(ap, format);
		vsnprintf(buf, 1024, format, ap);
		va_end(ap);

		/*************************************************************************\
		|* Print to stderr always
		\*************************************************************************/
		if (_debug < DBG_TRACE || _traceFile != stderr)
			size = fprintf(stderr, "sim: %s\n", buf);

		/*************************************************************************\
		|* And print to trace file, if trace is active
		\*************************************************************************/
		if (_debug >= DBG_TRACE)
			size = fprintf(_traceFile, "%08" PRIX64 ": %s\n", _cycles, buf);
		}
	else
		size = 0;

	return size;
	}

/*****************************************************************************\
|* Set the current error if we haven't already seen one
\*****************************************************************************/
void Simulator::setError(ErrorCode e, uint32_t addr)
	{
	if (e < 0 && !_error)
		{
		_error			= e;
		_errorAddress	= addr;
		}
	}

/*****************************************************************************\
|* Determine if we ought to exit based on the error
\*****************************************************************************/
bool Simulator::shouldExit(void)
	{
	int e = 0;
	switch (_error)
		{
		case E_NONE:				// Never exit and don't print the error
			return 0;
		case E_RD_UNINIT:
		case E_WR_ROM:
			e = (_errorLevel >= EL_FULL);
			break;
		case E_EX_UNINIT:
		case E_RD_UNDEF:
		case E_WR_UNDEF:
			e = (_errorLevel >= EL_MEMORY);
			break;
		case E_EX_UNDEF:
		case E_BREAK:
		case E_INVALID_INSN:
		case E_CALL_RET:
		case E_CYCLE_LIMIT:
		case E_USER:
			// Exit always
			return 1;
		}

	if (!e)
		{
		warn("%s at address %04x", errorString(_error).c_str(), _errorAddress);
		_error = E_NONE;
		return 0;
		}
	else
		return _error;
	}

/*****************************************************************************\
|* Return a string for an error-code
\*****************************************************************************/
String Simulator::errorString(ErrorCode e)
	{
	static String errors[] =
		{
		"no error",
		"instruction read from undefined memory",
		"instruction read from uninitialized memory",
		"read from undefined memory",
		"read from uninitialized memory",
		"write to undefined memory",
		"write to read-only memory",
		"BRK instruction executed",
		"invalid instruction executed",
		"return from emulator",
		"cycle limit reached",
		"user defined error"
		};

	if (e > E_NONE)
		e = E_NONE;
	else if (e < E_USER)
		e = E_USER;
	return errors[-e];
	}


/*****************************************************************************\
|* Set the processor status flags
\*****************************************************************************/
void Simulator::setFlags(uint8_t mask, uint8_t value)
	{
	_regs.p = (_regs.p & ~mask) | value;
	_regs.p_valid &= ~mask;
	}



/*****************************************************************************\
|* Set a limit on simulation time
\*****************************************************************************/
void Simulator::setCycleLimit(uint64_t limit)
	{
	_cycleLimit = (limit) ? _cycles + limit : 0;
	}

/*****************************************************************************\
 * |* Add memory to the simulator
\*****************************************************************************/
void Simulator::addRAM(uint32_t address, uint32_t length, bool zero)
	{
	uint32_t end = address + length;
	if (address >= _maxRam)
		return;
	if (end >= _maxRam)
		end = _maxRam;

	uint8_t flags = (zero)
		  ? (MS_UNDEFINED | MS_ROM | MS_INVALID)
		  : MS_UNDEFINED;

	for (; address < end; address ++)
		{
		_memState[address] &= ~flags;
		if (zero)
			_mem[address] = 0;
		}
	}


/*****************************************************************************\
 * |* Add initialised memory to the simulator
\*****************************************************************************/
void Simulator::addRAM(uint32_t address, uint8_t *data, uint32_t length)
	{
	uint32_t end = address + length;
	if (address >= _maxRam)
		return;
	if (end >= _maxRam)
		end = _maxRam;

	for (; address < end; address++, data++)
		{
		_memState[address]	&= ~(MS_UNDEFINED | MS_ROM | MS_INVALID);
		_mem[address]		 = *data;
		}
	}

#pragma mark -- Private Methods




/*****************************************************************************\
|* Return any known label for this address
\*****************************************************************************/
const String& Simulator::_getLabel(uint32_t address)
	{
	static const String none = "";

	AddressMap::iterator it = _labels.find(address);
	if (it != _labels.end())
		return  it->second;
	return none;
	}


/*****************************************************************************\
|* Get the processor status flags
\*****************************************************************************/
uint8_t Simulator::_getFlags(uint8_t mask)
	{
	if (0 != (_regs.p_valid & mask))
		{
		setError(E_EX_UNINIT, _regs.pc);
		warn("using uninitialized flags ($%02X) at PC=$%4X",
					  _regs.p_valid & mask, _regs.pc);
		}
	return _regs.p & mask;
	}
