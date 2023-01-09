#include "macros.h"
#include "simulator.h"
#include "instructions.h"

#define HAVE_BUILTIN_EXPECT 1
#include "likely.h"

/*****************************************************************************\
|* Lengths of each instruction
\*****************************************************************************/
static const uint8_t _insnLength[256] =
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
|* Addressing modes of each instruction
\*****************************************************************************/

static const AddressingMode _insnMode[256] =
	{
	/* 00 */	aIMP, aXIN, anon, anon, anon, aZPG, aZPG, anon,
	/* 08 */	aIMP, aIMM, aACC, anon, anon, aABS, aABS, anon,
	/* 10 */	aREL, aINY, anon, anon, anon, aZPX, aZPX, anon,
	/* 18 */	aIMP, aABY, anon, anon, anon, aABX, aABX, anon,
	/* 20 */	aABS, aXIN, anon, anon, aZPG, aZPG, aZPG, anon,
	/* 28 */	aIMP, aIMM, aACC, anon, aABS, aABS, aABS, anon,
	/* 30 */	aREL, aINY, anon, anon, anon, aZPX, aZPX, anon,
	/* 38 */	aIMP, aABY, anon, anon, anon, aABX, aABX, anon,
	/* 40 */	aIMP, aXIN, anon, anon, anon, aZPG, aZPG, anon,
	/* 48 */	aIMP, aIMM, aACC, anon, aABS, aABS, aABS, anon,
	/* 50 */	aREL, aINY, anon, anon, anon, aZPX, aZPX, anon,
	/* 58 */	aIMP, aABY, anon, anon, anon, aABX, aABX, anon,
	/* 60 */	aIMP, aXIN, anon, anon, anon, aZPG, aZPG, anon,
	/* 68 */	aIMP, aIMM, aACC, anon, aIND, aABS, aABS, anon,
	/* 70 */	aREL, aINY, anon, anon, anon, aZPX, aZPX, anon,
	/* 78 */	aIMP, aABY, anon, anon, anon, aABX, aABX, anon,
	/* 80 */	anon, aXIN, anon, anon, aZPG, aZPG, aZPG, anon,
	/* 88 */	aIMP, anon, aIMP, anon, aABS, aABS, aABS, anon,
	/* 90 */	aREL, aINY, anon, anon, aZPX, aZPX, aZPY, anon,
	/* 98 */	aIMP, aABY, aIMP, anon, anon, aABX, anon, anon,
	/* A0 */	aIMM, aXIN, aIMM, anon, aZPG, aZPG, aZPG, anon,
	/* A8 */	aIMP, aIMM, aIMP, anon, aABS, aABS, aABS, anon,
	/* B0 */	aREL, aINY, anon, anon, aZPX, aZPX, aZPY, anon,
	/* B8 */	aIMP, aABY, aIMP, anon, aABX, aABX, aABY, anon,
	/* C0 */	aIMM, aXIN, anon, anon, aZPG, aZPG, aZPG, anon,
	/* C8 */	aIMP, aIMM, aIMP, anon, aABS, aABS, aABS, anon,
	/* D0 */	aREL, aINY, anon, anon, anon, aZPX, aZPX, anon,
	/* D8 */	aIMP, aABY, anon, anon, anon, aABX, aABX, anon,
	/* E0 */	aIMM, aXIN, anon, anon, aZPG, aZPG, aZPG, anon,
	/* E8 */	aIMP, aIMM, aIMP, anon, aABS, aABS, aABS, anon,
	/* F0 */	aREL, aINY, anon, anon, anon, aZPX, aZPX, anon,
	/* F8 */	aIMP, aABY, anon, anon, anon, aABX, aABX, anon,
	};

static const InsnType _insnTypes[256] =
	{
	/* 00 */	iBRK, iORA, inon, inon, inon, iORA, iASL, inon,
	/* 08 */	iPHP, iORA, iASL, inon, inon, iORA, iASL, inon,
	/* 10 */	iBPL, iORA, inon, inon, inon, iORA, iASL, inon,
	/* 18 */	iCLC, iORA, inon, inon, inon, iORA, iASL, inon,
	/* 20 */	iJSR, iAND, inon, inon, iBIT, iAND, iROL, inon,
	/* 28 */	iPLP, iAND, iROL, inon, iBIT, iAND, iROL, inon,
	/* 30 */	iBMI, iAND, inon, inon, inon, iAND, iROL, inon,
	/* 38 */	iSEC, iAND, inon, inon, inon, iAND, iROL, inon,
	/* 40 */	iRTI, iEOR, inon, inon, inon, iEOR, iLSR, inon,
	/* 48 */	iPHA, iEOR, iLSR, inon, iJMP, iEOR, iLSR, inon,
	/* 50 */	iBVC, iEOR, inon, inon, inon, iEOR, iLSR, inon,
	/* 58 */	iCLI, iEOR, inon, inon, inon, iEOR, iLSR, inon,
	/* 60 */	iRTS, iADC, inon, inon, inon, iADC, iROR, inon,
	/* 68 */	iPLA, iADC, iROR, inon, iJMP, iADC, iROR, inon,
	/* 70 */	iBVS, iADC, inon, inon, inon, iADC, iROR, inon,
	/* 78 */	iSEI, iADC, inon, inon, inon, iADC, iROR, inon,
	/* 80 */	inon, iSTA, inon, inon, iSTY, iSTA, iSTX, inon,
	/* 88 */	iDEY, inon, iTXA, inon, iSTY, iSTA, iSTX, inon,
	/* 90 */	iBCC, iSTA, inon, inon, iSTY, iSTA, iSTX, inon,
	/* 98 */	iTYA, iSTA, iTXS, inon, inon, iSTA, inon, inon,
	/* A0 */	iLDY, iLDA, iLDX, inon, iLDY, iLDA, iLDX, inon,
	/* A8 */	iTAY, iLDA, iTAX, inon, iLDY, iLDA, iLDX, inon,
	/* B0 */	iBCS, iLDA, inon, inon, iLDY, iLDA, iLDX, inon,
	/* B8 */	iCLV, iLDA, iTSX, inon, iLDY, iLDA, iLDX, inon,
	/* C0 */	iCPY, iCMP, inon, inon, iCPY, iCMP, iDEC, inon,
	/* C8 */	iINY, iCMP, iDEX, inon, iCPY, iCMP, iDEC, inon,
	/* D0 */	iBNE, iCMP, inon, inon, inon, iCMP, iDEC, inon,
	/* D8 */	iCLD, iCMP, inon, inon, inon, iCMP, iDEC, inon,
	/* E0 */	iCPX, iSBC, inon, inon, iCPX, iSBC, iINC, inon,
	/* E8 */	iINX, iSBC, iNOP, inon, iCPX, iSBC, iINC, inon,
	/* F0 */	iBEQ, iSBC, inon, inon, inon, iSBC, iINC, inon,
	/* F8 */	iSED, iSBC, inon, inon, inon, iSBC, iINC, inon,
	};

/*****************************************************************************\
|* Macros to make the code easier to read : Status flags
\*****************************************************************************/
#define SETZ(a) setFlags(FLAG_Z,  ((a) & 0xFF)	? 0		 : FLAG_Z)
#define SETC(a) setFlags(FLAG_C,  (a)			? FLAG_C : 0)
#define SETV(a) setFlags(FLAG_V,  (a)			? FLAG_V : 0)
#define SETD(a) setFlags(FLAG_D,  (a)			? FLAG_D : 0)
#define SETN(a) setFlags(FLAG_N,  ((a)&0x80)	? FLAG_N : 0)
#define SETI(a) setFlags(FLAG_I,  (a)			? FLAG_I : 0)
#define GETC    _getFlags(FLAG_C)
#define GETD    _getFlags(FLAG_D)

/*****************************************************************************\
|* Macros to make the code easier to read : Operations
\*****************************************************************************/
#define ZP_R1   val = _readByte(data & 0xFF)
#define ZP_W1   _writeByte(data & 0xFF, val)
#define ZPX_R1  val = _readByte((data + _regs.x) & 0xFF)
#define ZPX_W1  _writeByte((data + _regs.x) & 0xFF, val)
#define ZPY_R1  val = _readByte((data + _regs.y) & 0xFF)
#define ZPY_W1  _writeByte((data + _regs.y) & 0xFF, val)
#define ABS_R1  val = _readByte(data)
#define ABS_W1  _writeByte(data, val)
#define ABX_R1  val = _readByte(data + _regs.x)
#define ABX_W1  _writeByte(data + _regs.x, val)
#define ABY_R1  val = _readByte(data + _regs.y)
#define ABY_W1  _writeByte(data + _regs.y, val)
#define IND_X(op)  val = _readIndX(data); op
#define IND_Y(op)  val = _readIndY(data); op
#define INDW_X(op) op; _writeIndX(data, val)
#define INDW_Y(op) op; _writeIndY(data, val)

#define ORA _regs.a |= val; SETZ(_regs.a); SETN(_regs.a)
#define AND _regs.a &= val; SETZ(_regs.a); SETN(_regs.a)
#define EOR _regs.a ^= val; SETZ(_regs.a); SETN(_regs.a)
#define ADC _adc(val)
#define SBC _sbc(val)
#define ASL SETC(val & 0x80); val = (val << 1) & 0xFF; SETZ(val); SETN(val)
#define ROL val = (val << 1) | (GETC ? 1 : 0); SETC(val & 256); val &= 0xFF; SETZ(val); SETN(val)
#define LSR SETC(val & 1); val=(val >> 1) & 0xFF; SETZ(val); SETN(val)
#define ROR val |= (GETC ? 256 : 0); SETC(val & 1); val = (val >> 1) & 0xFF; SETZ(val); SETN(val)

#define SET_ZN   SETN(val); SETZ(val)
#define DEC val = (val - 1) & 0xFF; SET_ZN
#define INC val = (val + 1) & 0xFF; SET_ZN
#define CMP val = (_regs.a + 0x100 - val); SET_ZN; SETC(val > 0xFF)
#define CPX val = (_regs.x + 0x100 - val); SET_ZN; SETC(val > 0xFF)
#define CPY val = (_regs.y + 0x100 - val); SET_ZN; SETC(val > 0xFF)

#define LDA SET_ZN; _regs.a = val
#define LDX SET_ZN; _regs.x = val
#define LDY SET_ZN; _regs.y = val
#define STA val = _regs.a
#define STX val = _regs.x
#define STY val = _regs.y
#define POP  _regs.s = (_regs.s + 1) & 0xFF; val = _readByte(0x100 + _regs.s)
#define PUSH(val) _cycles += 3; \
				  _writeByte(0x100 + _regs.s,val); \
				  _regs.s = (_regs.s - 1) & 0xFF

// Complete ops
#define ZP_R(op)   _cycles += 3; ZP_R1; op
#define ZP_W(op)   _cycles += 3; op; ZP_W1
#define ZP_RW(op)  _cycles += 5; ZP_R1; op; ZP_W1

#define ABS_R(op)  _cycles += 4; ABS_R1; op
#define ABS_W(op)  _cycles += 4; op; ABS_W1
#define ABS_RW(op) _cycles += 6; ABS_R1; op; ABS_W1

#define ZPX_R(op)   _cycles += 4; ZPX_R1; op
#define ZPX_W(op)   _cycles += 4; op; ZPX_W1
#define ZPX_RW(op)  _cycles += 6; ZPX_R1; op; ZPX_W1

#define ZPY_R(op)   _cycles += 4; ZPY_R1; op
#define ZPY_W(op)   _cycles += 4; op; ZPY_W1

#define ABX_R(op)   _cycles += 4; _handleExtraAbsoluteX(data); ABX_R1; op
#define ABX_W(op)   _cycles += 5; op; ABX_W1
#define ABX_RW(op)  _cycles += 7; ABX_R1; op; ABX_W1

#define ABY_R(op)   _cycles += 4; _handleExtraAbsoluteY(data); ABY_R1; op
#define ABY_W(op)   _cycles += 5; op; ABY_W1

#define IMM(op)     _cycles += 2; val = data; op
#define IMP_A(op)   _cycles += 2; val = _regs.a; op; _regs.a = val; SET_ZN
#define IMP_Y(op)   _cycles += 2; val = _regs.y; op; _regs.y = val; SET_ZN
#define IMP_X(op)   _cycles += 2; val = _regs.x; op; _regs.x = val; SET_ZN
#define TXS()       _cycles += 2; _regs.s = _regs.x;

#define BRA_0(a)    _branch(data, a, 0)
#define BRA_1(a)    _branch(data, a, 1)
#define JMP()      _cycles += 3; _regs.pc = data
#define JMP16()    _cycles += 5; _regs.pc = _readWord(data)
#define JSR()      _jsr(data)
#define RTS()      _rts()
#define RTI()      _rti()

#define CL_F(f)   _cycles += 2; setFlags(f, 0)
#define SE_F(f)   _cycles += 2; setFlags(f, f)

#define POP_P  _cycles += 4; POP; setFlags(0xFF, val | 0x30)
#define POP_A  _cycles += 4; POP; LDA

#define BIT_ZP   _cycles += 3; _bit(data & 0xFF)
#define BIT_ABS  _cycles += 4; _bit(data)

/*****************************************************************************\
|* Macros to aid in printing out the instructions in a trace
\*****************************************************************************/
#define PSTR(str)	memcpy(buf, str, strlen(str)); buf += strlen(str)
#define PHX2(val)	buf = _hex2(buf, val)
#define PHX4(val)	buf = _hex4(buf, val)
#define PNAM(name)  PSTR(name); buf++;
#define PLAB(val)	buf = _printAbsLabel(buf, val, 0)
#define PLABX(val)	buf = _printAbsLabel(buf, val, 'X')
#define PLABY(val)	buf = _printAbsLabel(buf, val, 'Y')
#define PLZP(val)	buf = _printZpLabel(buf, val, 0)
#define PLZPX(val)	buf = _printZpLabel(buf, val, 'X')
#define PLZPY(val)	buf = _printZpLabel(buf, val, 'Y')
#define PLIDX(val)	buf = _printIndLabel(buf, val, 'X', hint)
#define PLIDY(val)	buf = _printIndLabel(buf, val, 'Y', hint)
#define PLIND(val)	buf = _printIndLabel(buf, val, 0, hint)
#define PXTRA(b,r,h) if (h) buf[18] = ((((b)+(r))^(b))&0xFF00) ? '*' : buf[18]

#define INSPRT_IMM(name)	PNAM(name); PSTR("#$"); PHX2(data)
#define INSPRT_BRA(name)	PNAM(name); PXTRA(pc+2, (int8_t)(data), 1); \
										PLAB(pc+2+(int8_t)data)
#define INSPRT_ABS(name)	PNAM(name); PLAB(data)
#define INSPRT_ABXW(name)	PNAM(name); PLABX(data)
#define INSPRT_ABYW(name)	PNAM(name); PLABY(data)
#define INSPRT_ABX(name)	PNAM(name); PXTRA(data,_regs.x, hint); PLABX(data)
#define INSPRT_ABY(name)	PNAM(name); PXTRA(data,_regs.y, hint); PLABY(data)
#define INSPRT_ZPG(name)	PNAM(name); PLZP(data)
#define INSPRT_ZPX(name)	PNAM(name); PLZPX(data)
#define INSPRT_ZPY(name)	PNAM(name); PLZPY(data)
#define INSPRT_IDX(name)	PNAM(name); PLIDX(data)
#define INSPRT_IDY(name)	PNAM(name); PXTRA(_readWord(data),_regs.y, hint); \
										PLIDY(data)
#define INSPRT_IDYW(name)	PNAM(name); PLIDY(data)
#define INSPRT_IND(name)	PNAM(name); PLIND(data)
#define INSPRT_IMP(name)	PNAM(name)
#define INSPRT_ACC(name)	PNAM(name); PSTR("A")

static const char *_hexDigits = "0123456789ABCDEF";



#pragma mark -- Callbacks
/*****************************************************************************\
|* Read (PC); set appropriate error status
\*****************************************************************************/
static Simulator::ErrorCode _rtsCallback(Simulator *sim,
										 Simulator::Registers *regs,
										 uint32_t address,
										 int data)
	{
	return Simulator::E_CALL_RET;
	}



/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Simulator::Simulator(int maxRam,
					 QObject *parent)
		  :QObject{parent}
		  ,_maxRam(maxRam)
		  ,_traceFile(stderr)
		  ,_cycles(0)
		  ,_cycleLimit(0)
		  ,_errorLevel(EL_NONE)
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
		_writeCbs[i]	= nullptr;
		_execCbs[i]		= nullptr;
		_mem[i]			= 0x0;
		_memState[i]	= MS_UNDEFINED | MS_INVALID;
		}

	/*************************************************************************\
	|* And other state
	\*************************************************************************/
	_regs = {0, 0, 0, 0, 0xFF, 0, 0xFF};
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

				// Allow read from next location, as CPU always reads 2 bytes
				if (address + 1 < _maxRam)
					_memState[address + 1] &= ~(MS_UNDEFINED | MS_INVALID);
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
|* Add a callback over a range of memory
\*****************************************************************************/
void Simulator::addCallback(SIM_CB cb,
							uint32_t address,
							uint32_t len,
							CallbackType type)
	{
	for (int i=address; (i<address + len) && (i<_maxRam); i++)
		addCallback(cb, i, type);
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
		/*********************************************************************\
		|* Print original message to a string
		\*********************************************************************/
		va_start(ap, format);
		vsnprintf(buf, 16384, format, ap);
		va_end(ap);

		/*********************************************************************\
		|* Print to stderr always
		\*********************************************************************/
		if (_debug < DBG_TRACE || _traceFile != stderr)
			size = fprintf(stderr, "sim: %s\n", buf);

		/*********************************************************************\
		|* And print to trace file, if trace is active
		\*********************************************************************/
		if (_debug >= DBG_TRACE)
			size = fprintf(_traceFile, "%08" PRIX64 ": %s\n", _cycles, buf);
		}
	else
		size = 0;

	return size;
	}

/*****************************************************************************\
|* Print an error
\*****************************************************************************/
int Simulator::error(const char *format, ...)
	{
	char buf[16384];
	int size;
	va_list ap;

	/*************************************************************************\
	|* Print original message to a string
	\*************************************************************************/
	va_start(ap, format);
	vsnprintf(buf, 16384, format, ap);
	va_end(ap);

	/*************************************************************************\
	|* Print to stderr always
	\*************************************************************************/
	if (_debug < DBG_TRACE || _traceFile != stderr)
		size = fprintf(stderr, "sim: ERROR, %s\n", buf);

	/*************************************************************************\
	|* And print to trace file, if trace is active
	\*************************************************************************/
	if (_debug >= DBG_TRACE)
		size = fprintf(_traceFile, "%08" PRIX64 ": ERROR, %s\n", _cycles, buf);

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
		warn("%s at address $%04x", errorString(_error).c_str(), _errorAddress);
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
|* Add memory to the simulator
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
|* Add initialised memory to the simulator
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

/*****************************************************************************\
|* Add initialised memory to the simulator
\*****************************************************************************/
void Simulator::addROM(uint32_t address, uint8_t *data, uint32_t length)
	{
	uint32_t end = address + length;
	if (address >= _maxRam)
		return;
	if (end >= _maxRam)
		end = _maxRam;

	for (; address < end; address++, data++)
		{
		_memState[address]	&= ~(MS_UNDEFINED | MS_INVALID);
		_memState[address]	|= MS_ROM;
		_mem[address]		 = *data;
		}
	}


/*****************************************************************************\
|* Run until some error completion
\*****************************************************************************/
Simulator::ErrorCode Simulator::run(uint32_t address, Registers *regs)
	{
	if (regs != nullptr)
		_regs = *regs;

	_error		= E_NONE;
	_regs.pc	= address & 0xFFFF;
	while (!shouldExit())
		_next();

	if (regs)
		*regs = _regs;

	return _error;
	}

/*****************************************************************************\
|* Call a subroutine
\*****************************************************************************/
Simulator::ErrorCode Simulator::call(uint32_t address, Registers *regs)
	{
	/*************************************************************************\
	|* If we're supplied registers, use them
	\*************************************************************************/
	if (regs)
		_regs = *regs;

	/*************************************************************************\
	|* Save off the PC
	\*************************************************************************/
	uint16_t oldPC = _regs.pc;

	/*************************************************************************\
	|* use 0xFFFF as the return address and set up the callback
	\*************************************************************************/
	_regs.pc = 0xFFFF;

	addCallback(::_rtsCallback, 0xFFFF, CB_EXEC);

	/*************************************************************************\
	|* Execute the JSR and continue
	\*************************************************************************/
	_jsr(address);

	ErrorCode err = run(address, regs);

	/*************************************************************************\
	|* If we got here from a JSR return, just return ok
	\*************************************************************************/
	if (err == E_CALL_RET)
		{
		// Return to old address
		_regs.pc = oldPC;
		err = _error = E_NONE;
		}

	return err;
	}


/*****************************************************************************\
|* Set the trace output
\*****************************************************************************/
void Simulator::setTraceFile(FILE *fp)
	{
	_traceFile = (fp == nullptr) ? stderr : fp;
	}


/*****************************************************************************\
|* Determine if the instruction at an address is a branch
\*****************************************************************************/
bool Simulator::isBranch(uint32_t address)
	{
	uint32_t insn = _mem[address & 0xFFFF];
	switch (insn)
		{
		case 0x10:
		case 0x30:
		case 0x50:
		case 0x70:
		case 0x90:
		case 0xB0:
		case 0xD0:
		case 0xF0:
			return 1;
		default:
			return 0;
		}
	}


/*****************************************************************************\
|* Return a byte to the outside world
\*****************************************************************************/
int  Simulator::getByte(uint32_t address)
	{
	if (address >= _maxRam)
		return INVALID_ADDRESS;
	if (_memState[address] & MS_INVALID)
		return INVALID_ADDRESS;
	return _mem[address];
	}

/*****************************************************************************\
|* Return a copy of the profile data
\*****************************************************************************/
Simulator::ProfileData Simulator::profileInfo(void)
	{
	ProfileData copy = _profileData;
	return copy;
	}

#pragma mark -- labels


/*****************************************************************************\
|* Labels: add a label
\*****************************************************************************/
void Simulator::addLabel(uint32_t address, String label)
	{
	if (label.length() > 0)
		_labels[address] = label;
	}

/*****************************************************************************\
|* Labels: load labels from a file
\*****************************************************************************/
int Simulator::loadLabels(String path)
	{
	int ok		= -1;
	int line	= 0;

	FILE *fp = fopen(path.c_str(), "r");
	if (fp)
		{
		ok = 0;
		forever
			{
			char name[1024], str[256];
			unsigned addr = 0, page = 0;

			int e = fscanf(fp, "%255[^\n\r]\n", str);
			if (e == EOF)
				{
				fclose(fp);
				break;
				}

			line ++;
			// Try parsing a CC65 line:
			if (2 != sscanf(str, "al %6x .%31s", &addr, name))
				{
				// No, try parsing MADS line:
				if (3 != sscanf(str, "%02x %04x %31s", &page, &addr, name))
					{
					// Ignore line
					error("%s[%d]: invalid line on label file",
						 path.c_str(), line);
					continue;
					}
				}
			if (addr <= 0xFFFF && page == 0)
				addLabel(addr, name);
			}
		}

	return ok;
	}


#pragma mark -- Profiling

/*****************************************************************************\
|* Save a profile
\*****************************************************************************/
int Simulator::saveProfile(String path)
	{
	int ok				= 1;
	uint16_t version	= 0x101;
	int e				= 0;
	FILE *fp			= fopen(path.c_str(), "wb");
	ProfileData pd		= _profileData;

	if (fp)
		{
		e = fprintf(fp, "SIM:PROF\n") < 0;
		e |= fwrite(&version, sizeof(version), 1, fp) < 1;
		e |= fwrite(&_maxRam, sizeof(_maxRam), 1, fp) < 1;
		e |= fwrite(&pd.cycles[0],    sizeof(pd.cycles[0]), _maxRam, fp) < _maxRam;
		e |= fwrite(&pd.branch[0],    sizeof(pd.branch[0]), _maxRam, fp) < _maxRam;
		e |= fwrite(&pd.extra[0],     sizeof(pd.extra[0]), _maxRam, fp) < _maxRam;
		e |= fwrite(&pd.mflag[0],     sizeof(pd.mflag[0]), _maxRam, fp) < _maxRam;
		e |= fwrite(&pd.branch_skip,  sizeof(pd.branch_skip), 1, fp) < 1;
		e |= fwrite(&pd.branch_taken, sizeof(pd.branch_taken), 1, fp) < 1;
		e |= fwrite(&pd.branch_extra, sizeof(pd.branch_extra), 1, fp) < 1;
		e |= fwrite(&pd.abs_x_extra,  sizeof(pd.abs_x_extra), 1, fp) < 1;
		e |= fwrite(&pd.abs_y_extra,  sizeof(pd.abs_y_extra), 1, fp) < 1;
		e |= fwrite(&pd.ind_y_extra,  sizeof(pd.ind_y_extra), 1, fp) < 1;
		e |= fwrite(&pd.instructions, sizeof(pd.instructions), 1, fp) < 1;
		e |= fclose(fp) != 0;

		if (e)
			error("Can't save profile data", strerror(errno));
		else
			ok = 0;
		}

	return ok;
	}



/*****************************************************************************\
|* Save a profile
\*****************************************************************************/
int Simulator::loadProfile(String path)
	{
	int e					= 0;
	uint16_t version		= 0;
	FILE *fp				= fopen(path.c_str(), "rb");
	if(!fp)
		{
		if (errno == ENOENT)
			{
			warn("missing profile data");
			return 0;
			}

		error("can't load profile data", strerror(errno));
		return 1;
		}

	char buf[32];
	if (!fgets(buf, 16, fp) || strcmp(buf, "SIM:PROF\n") )
		{
		fclose(fp);
		error("not a profile data file");
		return 1;
		}

	if (fread(&version, sizeof(version), 1, fp) < 1 || version != 0x100 )
		{
		fclose(fp);
		error("invalid profile data file version %04x", version);
		return 1;
		}

	if (fread(&_maxRam, sizeof(_maxRam), 1, fp) < 1)
		{
		fclose(fp);
		error("Cannot read maximum memory for profile file");
		return 1;
		}

	e |= fread(&_profileData.cycles[0],    sizeof(_profileData.cycles[0]), _maxRam, fp) < _maxRam;
	e |= fread(&_profileData.branch[0],    sizeof(_profileData.branch[0]), _maxRam, fp) < _maxRam;
	e |= fread(&_profileData.extra[0],     sizeof(_profileData.extra[0]), _maxRam, fp) < _maxRam;
	e |= fread(&_profileData.mflag[0],     sizeof(_profileData.mflag[0]), _maxRam, fp) < _maxRam;
	e |= fread(&_profileData.branch_skip,  sizeof(_profileData.branch_skip), 1, fp) < 1;
	e |= fread(&_profileData.branch_taken, sizeof(_profileData.branch_taken), 1, fp) < 1;
	e |= fread(&_profileData.branch_extra, sizeof(_profileData.branch_extra), 1, fp) < 1;
	e |= fread(&_profileData.abs_x_extra,  sizeof(_profileData.abs_x_extra), 1, fp) < 1;
	e |= fread(&_profileData.abs_y_extra,  sizeof(_profileData.abs_y_extra), 1, fp) < 1;
	e |= fread(&_profileData.ind_y_extra,  sizeof(_profileData.ind_y_extra), 1, fp) < 1;
	e |= fread(&_profileData.instructions, sizeof(_profileData.instructions), 1, fp) < 1;

	if (e)
		error("can't read profile data", strerror(errno));
	fclose(fp);

	return e;
	}


#pragma mark -- Instruction disassembly


/*****************************************************************************\
|* Trace printing: disassemble
\*****************************************************************************/
char * Simulator::disassemble(char *buf, uint32_t address)
	{
	_printCurrentInsn(address, buf, 0);
	return buf;
	}

/*****************************************************************************\
|* Get instruction information
\*****************************************************************************/
Simulator::InstructionInfo Simulator::insnInfo(uint32_t address)
	{
	static InstructionInfo invalid = {0,0,0, "",MS_INVALID,0,0,""};

	if (address >= _maxRam)
		return invalid;

	InstructionInfo info;

	info.addr		= (uint16_t) address;
	info.insn		= _mem[address];
	info.bytes		= _insnLength[info.insn];
	info.arg1		= ((info.bytes > 1) && (address < _maxRam-1))
					? _mem[address+1]
					: 0;
	info.arg2		= ((info.bytes > 1) && (address < _maxRam-2))
					? _mem[address+2]
					: 0;
	info.state		= _memState[address];
	info.label		= _getLabel(address);

	int vec		= address;
	int8_t rel	= (int8_t)(info.arg1);

	switch (_insnMode[info.insn])
		{
		case anon:
			warn("Attempt to get info for invalid instructin at $%04x", address);
			break;

		case aACC:
		case aIMM:
		case aIMP:
			break;

		case aZPG:
		case aABS:
			 _getLabel(vec, 16, info.argLabel);
			break;

		case aZPX:
		case aABX:
			_getLabel(vec + _regs.x, 16, info.argLabel);
			break;

		case aZPY:
		case aABY:
			_getLabel(vec + _regs.y, 16, info.argLabel);
			break;

		case aIND:
			vec = _readWord(address);
			_getLabel(vec, 16, info.argLabel);
			break;

		case aXIN:
			vec = _readWord(address + _regs.x);
			_getLabel(vec, 16, info.argLabel);
			break;

		case aINY:
			vec = _readWord(address) + _regs.y;
			_getLabel(vec, 16, info.argLabel);
			break;

		case aREL:;
			_getLabel(vec + rel + 2, 16, info.argLabel);
			break;
		}

	return info;
	}

#pragma mark -- Private Methods


/*****************************************************************************\
|* Read (PC); set appropriate error status
\*****************************************************************************/
uint8_t Simulator::_readPC(void)
	{
	uint32_t address = _regs.pc;

	if (likely(!(_memState[address] & (MS_UNDEFINED | MS_INVALID))))
		return _mem[address];
	else
		{
		if (_memState[address] & MS_UNDEFINED)
			setError(E_EX_UNDEF, address);
		else
			setError(E_EX_UNINIT, address);
		return _mem[address];
		}
	}


/*****************************************************************************\
|* Read a byte at an address; set appropriate error status
\*****************************************************************************/
uint8_t Simulator::_readByte(uint32_t address)
	{
	if (likely(!(_memState[address] & (MS_UNDEFINED|MS_INVALID|MS_CALLBACK))))
		return _mem[address];
	else
		{
		/*********************************************************************\
		|* Unusual memory
		\*********************************************************************/
		if ((_memState[address] & MS_CALLBACK) && _readCbs[address])
			{
			ErrorCode e = _readCbs[address](this, &_regs, address, CB_READ);
			setError(e, address);
			_writeMem = true;
			return e;
			}
		else
			{
			if (_memState[address] & MS_UNDEFINED)
				{
				_writeMem = true;
				setError(E_RD_UNDEF, address);
				}
			else if(_memState[address] & MS_INVALID)
				{
				_writeMem = true;
				setError(E_RD_UNINIT, address);
				_memState[address] &= ~MS_INVALID; // Initializes the memory
				}
			return _mem[address];
			}
		}
	}

/*****************************************************************************\
|* Read a word at an address; set appropriate error status
\*****************************************************************************/
uint16_t Simulator::_readWord(uint32_t address)
	{
	uint16_t d1 = _readByte(address);
	return d1 | (_readByte(address + 1) << 8);
	}

/*****************************************************************************\
|* Read a byte using the X-indexed addressing mode
\*****************************************************************************/
uint8_t Simulator::_readIndX(uint32_t address)
	{
	_cycles += 6;
	address = _readWord((address + _regs.x) & 0xFF);
	return _readByte(address);
	}

/*****************************************************************************\
|* Read a byte using the Y-indexed addressing mode
\*****************************************************************************/
uint8_t Simulator::_readIndY(uint32_t address)
	{
	_cycles += 5;
	address = _readWord(address & 0xFF);

	if (unlikely(((address & 0xFF) + _regs.y) > 0xFF))
		{
		_cycles++;
		if (_doProfiling)
			{
			_profileData.ind_y_extra ++;
			_profileData.extra[(_regs.pc-2) & 0xFFFF] ++;
			}
		}
	return _readByte(0xFFFF & (address + _regs.y));
	}

/*****************************************************************************\
|* Write a byte at an address; set appropriate error status
\*****************************************************************************/
void Simulator::_writeByte(uint32_t address, uint8_t val)
	{
	if (likely(!(_memState[address]) && (!_doProfiling)))
		_mem[address] = val;
	else
		{
		if (!_memState[address])
			{
			if (val != _mem[address])
				{
				_writeMem		= true;
				_mem[address]	= val;
				}
			return;
			}
		_writeMem = true;

		if (likely(!(_memState[address] & ~MS_INVALID)))
			{
			_mem[address]		= val;
			_memState[address]	= 0;
			}
		else if ((_memState[address] & MS_CALLBACK) && _writeCbs[address])
			setError(_writeCbs[address](this, &_regs, address, val), address);

		else if (_memState[address] & MS_UNDEFINED)
			setError(E_WR_UNDEF, address);

		else if (_memState[address] & MS_ROM)
			setError(E_WR_ROM, address);
		}
	}


/*****************************************************************************\
|* Write a byte using the X-indexed addressing mode
\*****************************************************************************/
void Simulator::_writeIndX(uint32_t address, uint8_t val)
	{
	_cycles += 6;
	address = _readWord((address + _regs.x) & 0xFF);
	_writeByte(address, val);
	}

/*****************************************************************************\
|* Write a byte using the Y-indexed addressing mode
\*****************************************************************************/
void Simulator::_writeIndY(uint32_t address, uint8_t val)
	{
	_cycles += 6;
	address = _readWord(address & 0xFF);
	_writeByte(0xFFFF & (address + _regs.y), val);
	}


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
|* Return any known label for this address
\*****************************************************************************/
void Simulator::_getLabel(uint32_t address, int slack, String &label)
	{
	static const String none = "";

	if (slack == 0)
		{
		label =_getLabel(address);
		return;
		}

	int delta = 1;
	while (delta <= slack)
		{
		AddressMap::iterator it = _labels.find(address+delta);
		if (it != _labels.end())
			label = it->second + " + "+ std::to_string(delta);

		it = _labels.find(address-delta);
		if (it != _labels.end())
			label = it->second + " - " + std::to_string(delta);

		delta ++;
		}

	label = none;
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

#pragma mark -- instructions


/*****************************************************************************\
|* Instruction: ADC
\*****************************************************************************/
void Simulator::_adc(uint8_t val)
	{
	if (GETD)
		{
		/*********************************************************************\
		|* Decimal mode
		\*********************************************************************/

		// Z flag is computed as the binary version.
		uint32_t tmp = _regs.a + val + (GETC ? 1 : 0);
		SETZ(tmp);

		// ADC value is computed in decimal
		tmp = (_regs.a & 0xF) + (val & 0xF) + (GETC ? 1 : 0);
		if (tmp >= 10)
			tmp = (tmp - 10) | 16;
		tmp += (_regs.a & 0xF0) + (val & 0xF0);
		SETN(tmp);
		SETV(!((_regs.a ^ val) & 0x80) && ((val ^ tmp) & 0x80));
		if (tmp > 0x9F)
			tmp += 0x60;
		SETC(tmp > 0xFF);
		_regs.a = tmp & 0xFF;
		}
	else
		{
		/*********************************************************************\
		|* Binary mode
		\*********************************************************************/

		uint32_t tmp = _regs.a + val + (GETC ? 1 : 0);
		SETV(((~(_regs.a ^ val)) & (_regs.a ^ tmp)) & 0x80);

		SETC(tmp > 0xFF);
		SETN(tmp);
		SETZ(tmp);
		_regs.a = tmp & 0xFF;
		}
	}


/*****************************************************************************\
|* Instruction: SBC
\*****************************************************************************/
void Simulator::_sbc(uint8_t val)
	{
	if (GETD)
		{
		/*********************************************************************\
		|* Decimal mode
		\*********************************************************************/

		val = val ^ 0xFF;

		// Z and V flags computed as the binary version.
		unsigned tmp = _regs.a + val + (GETC ? 1 : 0);
		SETV((((_regs.a ^ val)) & (_regs.a ^ tmp)) & 0x80);
		SETZ(tmp);

		// ADC value is computed in decimal
		tmp = (_regs.a & 0xF) + (val & 0xF) + (GETC ? 1 : 0);
		if (tmp < 0x10)
			tmp = (tmp - 6) & 0x0F;

		tmp += (_regs.a & 0xF0) + (val & 0xF0);
		if (tmp < 0x100)
			tmp = (tmp - 0x60) & 0xFF;

		SETN(tmp);
		SETC(tmp > 0xFF);
		_regs.a = tmp & 0xFF;
		}
	else
		{
		/*********************************************************************\
		|* Binary mode
		\*********************************************************************/

		unsigned tmp = _regs.a + 0xFF - val + (GETC ? 1 : 0);
		SETV((((_regs.a ^ val)) & (_regs.a ^ tmp)) & 0x80);
		SETC(tmp > 0xFF);
		SETN(tmp);
		SETZ(tmp);
		_regs.a = tmp & 0xFF;
		}
	}

/*****************************************************************************\
|* Instruction: branch ops
\*****************************************************************************/
void Simulator::_branch(int8_t off, uint8_t mask, int cond)
	{
	_cycles += 2;
	if (!_getFlags(mask) == !cond)
		{
		_cycles++;
		if (_doProfiling)
			{
			_profileData.branch[(_regs.pc-2) & 0xFFFF] ++;
			_profileData.branch_taken ++;
			}

		uint16_t val = (_regs.pc + off) & 0xFFFF;
		if ((val & 0xFF00) != (_regs.pc & 0xFF00))
			{
			_cycles++;
			if (_doProfiling)
				{
				_profileData.extra[(_regs.pc-2) & 0xFFFF] ++;
				_profileData.branch_extra ++;
				}
			}
		_regs.pc = val;
		}
	else if (_doProfiling)
		_profileData.branch_skip ++;
	}


/*****************************************************************************\
|* Instruction: BIT. Special case BIT insns as sometimes are used to SKIP
\*****************************************************************************/
void Simulator::_bit(uint32_t address)
	{
	if ((_memState[address] & MS_INVALID) && !(_memState[address] &  MS_CALLBACK))
		_regs.p_valid |= (FLAG_N | FLAG_V | FLAG_Z);
	else
		{
		uint8_t val = _readByte(address);
		SETN(val);
		SETV(val & 0x40);
		SETZ(_regs.a & val);
		}
	}


/*****************************************************************************\
|* Instruction: JSR
\*****************************************************************************/
void Simulator::_jsr(uint32_t address)
	{
	_regs.pc = (_regs.pc - 1) & 0xFFFF;
	PUSH(_regs.pc >> 8);
	PUSH(_regs.pc);
	_regs.pc = address;
	}


/*****************************************************************************\
|* Instruction: RTS
\*****************************************************************************/
void Simulator::_rts(void)
	{
	uint32_t val;
	POP;
	_regs.pc = val;
	POP;
	_regs.pc |= (val << 8);
	_regs.pc = (_regs.pc + 1) & 0xFFFF;
	_cycles += 6;
	}


/*****************************************************************************\
|* Instruction: RTI
\*****************************************************************************/
void Simulator::_rti(void)
	{
	uint32_t val;
	POP_P;		// already adds 4 cycles
	POP;
	_regs.pc = val;
	POP;
	_regs.pc |= (val << 8);
	_regs.pc = (_regs.pc) & 0xFFFF;
	_cycles += 2;
	}


/*****************************************************************************\
|* Extra cycles due to page boundaries on X index
\*****************************************************************************/
void Simulator::_handleExtraAbsoluteX(uint32_t address)
	{
	if (((address & 0xFF) + _regs.x) > 0xFF)
		{
		_cycles++;
		if (_doProfiling)
			{
			_profileData.extra[(_regs.pc-3) & 0xFFFF] ++;
			_profileData.abs_x_extra ++;
			}
		}
	}

/*****************************************************************************\
|* Extra cycles due to page boundaries on Y index
\*****************************************************************************/
void Simulator::_handleExtraAbsoluteY(uint32_t address)
	{
	if (((address & 0xFF) + _regs.y) > 0xFF)
		{
		_cycles++;
		if (_doProfiling)
			{
			_profileData.extra[(_regs.pc-3) & 0xFFFF] ++;
			_profileData.abs_y_extra ++;
			}
		}
	}


/*****************************************************************************\
|* Run the next instruction
\*****************************************************************************/
void Simulator::_next(void)
	{
	uint32_t val;
	uint64_t old_cycles = 0;
	Registers old_regs = {0,0,0,0,0,0,0};

	/*************************************************************************\
	|* Handle the return-from-processing states
	\*************************************************************************/
	if (_execCbs[_regs.pc] != nullptr)
		{
		setError(_execCbs[_regs.pc](this, &_regs, _regs.pc, CB_EXEC), _regs.pc);
		if (shouldExit())
			return;
		}

	if (_debug >= DBG_TRACE)
		_traceRegs();

	if (_cycleLimit && _cycles >= _cycleLimit)
		{
		setError(E_CYCLE_LIMIT, _regs.pc);
		return;
		}

	/*************************************************************************\
	|* Read instruction and data - always prefetched in real 6502 CPU
	\*************************************************************************/
	uint32_t insn = _readPC();
	uint32_t data = _readByte(_regs.pc + 1);

	// And if instruction is 3 bytes, read high byte of data
	if (_insnLength[insn] > 2)
		data |= ((uint32_t)(_readByte(_regs.pc + 2))) << 8;

	/*************************************************************************\
	|* If profiling, store old info
	\*************************************************************************/
	if (_doProfiling)
		{
		old_cycles	= _cycles;
		old_regs	= _regs;
		_writeMem	= false;
		}

	/*************************************************************************\
	|* Update PC
	\*************************************************************************/
	_regs.pc += _insnLength[insn];

	/*************************************************************************\
	|* Actually do it
	\*************************************************************************/
	switch (insn)
		{
		case 0x00:  setError(E_BREAK, _regs.pc-1); break;
		case 0x01:  IND_X(ORA);             break;
		case 0x05:  ZP_R(ORA);              break;
		case 0x06:  ZP_RW(ASL);             break;
		case 0x08:  PUSH(_getFlags(0xFF));	break; // PHP
		case 0x09:  IMM(ORA);               break;
		case 0x0A:  IMP_A(ASL);             break;
		case 0x0D:  ABS_R(ORA);             break;
		case 0x0E:  ABS_RW(ASL);            break;
		case 0x10:  BRA_0(FLAG_N);          break; // BPL
		case 0x11:  IND_Y(ORA);             break;
		case 0x15:  ZPX_R(ORA);             break;
		case 0x16:  ZPX_RW(ASL);            break;
		case 0x18:  CL_F(FLAG_C);           break; // CLC
		case 0x19:  ABY_R(ORA);             break;
		case 0x1d:  ABX_R(ORA);             break;
		case 0x1e:  ABX_RW(ASL);            break;
		case 0x20:  JSR();                  break; // JSR
		case 0x21:  IND_X(AND);             break;
		case 0x24:  BIT_ZP;                 break;
		case 0x25:  ZP_R(AND);              break;
		case 0x26:  ZP_RW(ROL);             break;
		case 0x28:  POP_P;                  break; // PLP
		case 0x29:  IMM(AND);               break;
		case 0x2a:  IMP_A(ROL);             break;
		case 0x2c:  BIT_ABS;                break;
		case 0x2d:  ABS_R(AND);             break;
		case 0x2e:  ABS_RW(ROL);            break;
		case 0x30:  BRA_1(FLAG_N);          break;
		case 0x31:  IND_Y(AND);             break;
		case 0x35:  ZPX_R(AND);             break;
		case 0x36:  ZPX_RW(ROL);            break;
		case 0x38:  SE_F(FLAG_C);           break; // SEC
		case 0x39:  ABY_R(AND);             break;
		case 0x3d:  ABX_R(AND);             break;
		case 0x3e:  ABX_RW(ROL);            break;
		case 0x40:  RTI();                  break; // RTI
		case 0x41:  IND_X(EOR);             break;
		case 0x45:  ZP_R(EOR);              break;
		case 0x46:  ZP_RW(LSR);             break;
		case 0x48:  PUSH(_regs.a);          break; // PHA
		case 0x49:  IMM(EOR);               break;
		case 0x4a:  IMP_A(LSR);             break;
		case 0x4c:  JMP();                  break; // JMP
		case 0x4d:  ABS_R(EOR);             break;
		case 0x4e:  ABS_RW(LSR);            break;
		case 0x50:  BRA_0(FLAG_V);          break;
		case 0x51:  IND_Y(EOR);             break;
		case 0x55:  ZPX_R(EOR);             break;
		case 0x56:  ZPX_RW(LSR);            break;
		case 0x58:  CL_F(FLAG_I);           break; // CLI
		case 0x59:  ABY_R(EOR);             break;
		case 0x5d:  ABX_R(EOR);             break;
		case 0x5e:  ABX_RW(LSR);            break;
		case 0x60:  RTS();                  break; // RTS
		case 0x61:  IND_X(ADC);             break;
		case 0x65:  ZP_R(ADC);              break;
		case 0x66:  ZP_RW(ROR);             break;
		case 0x68:  POP_A;                  break; // PLA
		case 0x69:  IMM(ADC);               break;
		case 0x6a:  IMP_A(ROR);             break;
		case 0x6c:  JMP16();                break; // JMP ()
		case 0x6d:  ABS_R(ADC);             break;
		case 0x6e:  ABS_RW(ROR);            break;
		case 0x70:  BRA_1(FLAG_V);          break;
		case 0x71:  IND_Y(ADC);             break;
		case 0x75:  ZPX_R(ADC);             break;
		case 0x76:  ZPX_RW(ROR);            break;
		case 0x78:  SE_F(FLAG_I);           break; // SEI
		case 0x79:  ABY_R(ADC);             break;
		case 0x7d:  ABX_R(ADC);             break;
		case 0x7e:  ABX_RW(ROR);            break;
		case 0x81:  INDW_X(STA);            break;
		case 0x84:  ZP_W(STY);              break;
		case 0x85:  ZP_W(STA);              break;
		case 0x86:  ZP_W(STX);              break;
		case 0x88:  IMP_Y(DEC);             break; // DEY
		case 0x8a:  IMP_X(LDA);             break; // TXA
		case 0x8c:  ABS_W(STY);             break;
		case 0x8d:  ABS_W(STA);             break;
		case 0x8e:  ABS_W(STX);             break;
		case 0x90:  BRA_0(FLAG_C);          break; // BCC
		case 0x91:  INDW_Y(STA);            break;
		case 0x94:  ZPX_W(STY);             break;
		case 0x95:  ZPX_W(STA);             break;
		case 0x96:  ZPY_W(STX);             break;
		case 0x98:  IMP_Y(LDA);             break; // TYA
		case 0x99:  ABY_W(STA);             break;
		case 0x9a:  TXS();                  break; // TXS
		case 0x9d:  ABX_W(STA);             break;
		case 0xa0:  IMM(LDY);               break;
		case 0xa1:  IND_X(LDA);             break;
		case 0xa2:  IMM(LDX);               break;
		case 0xa4:  ZP_R(LDY);              break;
		case 0xa5:  ZP_R(LDA);              break;
		case 0xa6:  ZP_R(LDX);              break;
		case 0xa8:  IMP_A(LDY);             break; // TAY
		case 0xa9:  IMM(LDA);               break;
		case 0xaa:  IMP_A(LDX);             break; // TAX
		case 0xac:  ABS_R(LDY);             break;
		case 0xad:  ABS_R(LDA);             break;
		case 0xae:  ABS_R(LDX);             break;
		case 0xb0:  BRA_1(FLAG_C);          break; // BCS
		case 0xb1:  IND_Y(LDA);             break;
		case 0xb4:  ZPX_R(LDY);             break;
		case 0xb5:  ZPX_R(LDA);             break;
		case 0xb6:  ZPY_R(LDX);             break;
		case 0xb8:  CL_F(FLAG_V);           break; // CLV
		case 0xb9:  ABY_R(LDA);             break;
		case 0xba:  IMP_X(val = _regs.s);   break; // TSX
		case 0xbc:  ABX_R(LDY);             break;
		case 0xbd:  ABX_R(LDA);             break;
		case 0xbe:  ABY_R(LDX);             break;
		case 0xc0:  IMM(CPY);               break;
		case 0xc1:  IND_X(CMP);             break;
		case 0xc4:  ZP_R(CPY);              break;
		case 0xc5:  ZP_R(CMP);              break;
		case 0xc6:  ZP_RW(DEC);             break;
		case 0xc8:  IMP_Y(INC);             break; // INY
		case 0xc9:  IMM(CMP);               break;
		case 0xca:  IMP_X(DEC);             break; // DEX
		case 0xcc:  ABS_R(CPY);             break;
		case 0xcd:  ABS_R(CMP);             break;
		case 0xce:  ABS_RW(DEC);            break;
		case 0xd0:  BRA_0(FLAG_Z);          break; // BNE
		case 0xd1:  IND_Y(CMP);             break;
		case 0xd5:  ZPX_R(CMP);             break;
		case 0xd6:  ZPX_RW(DEC);            break;
		case 0xd8:  CL_F(FLAG_D);           break; // CLD
		case 0xd9:  ABY_R(CMP);             break;
		case 0xdd:  ABX_R(CMP);             break;
		case 0xde:  ABX_RW(DEC);            break;
		case 0xe0:  IMM(CPX);               break;
		case 0xe1:  IND_X(SBC);             break;
		case 0xe4:  ZP_R(CPX);              break;
		case 0xe5:  ZP_R(SBC);              break;
		case 0xe6:  ZP_RW(INC);             break;
		case 0xe8:  IMP_X(INC);             break; // INX
		case 0xe9:  IMM(SBC);               break;
		case 0xea:  _cycles += 2;           break; // NOP
		case 0xec:  ABS_R(CPX);             break;
		case 0xed:  ABS_R(SBC);             break;
		case 0xee:  ABS_RW(INC);            break;
		case 0xf0:  BRA_1(FLAG_Z);          break; // BEQ
		case 0xf1:  IND_Y(SBC);             break;
		case 0xf5:  ZPX_R(SBC);             break;
		case 0xf6:  ZPX_RW(INC);            break;
		case 0xf8:  SE_F(FLAG_D);           break; // SED
		case 0xf9:  ABY_R(SBC);             break;
		case 0xfd:  ABX_R(SBC);             break;
		case 0xfe:  ABX_RW(INC);            break;
		default:    setError(E_INVALID_INSN, _regs.pc - 1);
		}

	/*************************************************************************\
	|* Update profile information
	\*************************************************************************/
	if (_doProfiling)
		{
		uint32_t cyc = _cycles - old_cycles;
		_profileData.instructions ++;
		_profileData.cycles[old_regs.pc & 0xFFFF] += cyc;
		if ((_regs.a == old_regs.a)							&&
			(_regs.x == old_regs.x)							&&
			(_regs.y == old_regs.y)							&&
			(_regs.p == old_regs.p)							&&
			(_regs.s == old_regs.s)							&&
			(_regs.pc == old_regs.pc + _insnLength[insn])	&&
			(!_writeMem))
			{
			_profileData.mflag[old_regs.pc] += cyc;
			}
		}
	}



#pragma mark -- Trace printing functions



/*****************************************************************************\
|* Trace printing: routines for hex conversion
\*****************************************************************************/
char * Simulator::_hex2(char *c, uint8_t x)
	{
	c[0] = _hexDigits[x >> 4];
	c[1] = _hexDigits[x & 15];
	return c + 2;
	}

char * Simulator::_hex4(char *c, uint16_t x)
	{
	c[0] = _hexDigits[x >> 12];
	c[1] = _hexDigits[(x >> 8) & 15];
	c[2] = _hexDigits[(x >> 4) & 15];
	c[3] = _hexDigits[x & 15];
	return c + 4;
	}

char * Simulator::_hex8(char *c, uint32_t x)
	{
	c[0] = _hexDigits[(x >> 28) & 15];
	c[1] = _hexDigits[(x >> 24) & 15];
	c[2] = _hexDigits[(x >> 20) & 15];
	c[3] = _hexDigits[(x >> 16) & 15];
	c[4] = _hexDigits[(x >> 12) & 15];
	c[5] = _hexDigits[(x >> 8) & 15];
	c[6] = _hexDigits[(x >> 4) & 15];
	c[7] = _hexDigits[x & 15];
	return c + 8;
	}

/*****************************************************************************\
|* Trace printing: trace the execution
\*****************************************************************************/
void Simulator::_traceRegs(void)
	{
	char buffer[256];
	char *buf = buffer;
	buf = _hex8(buf, _cycles);
	PSTR(": A=");
	PHX2(_regs.a);
	PSTR(" X=");
	PHX2(_regs.x);
	PSTR(" Y=");
	PHX2(_regs.y);
	PSTR(" P=");
	PHX2(_regs.p);
	PSTR(" S=");
	PHX2(_regs.s);
	PSTR(" PC=");
	PHX4(_regs.pc);
	*buf++ = ' ';
	_printCurrentInsn(_regs.pc, buf, 1);
	fputs(buffer, _traceFile);
	putc('\n', _traceFile);
	}

/*****************************************************************************\
|* Trace printing: print the current instruction
\*****************************************************************************/
void Simulator::_printCurrentInsn(uint16_t pc, char *buf, int hint)
	{
	uint32_t insn = _mem[pc & 0xFFFF];
	uint32_t data = (_insnLength[insn] == 2)
				  ? _mem[(1 + pc) & 0xFFFF]
				  : (_insnLength[insn] == 3)
				  ? _mem[(1 + pc) & 0xFFFF] + (_mem[(2 + pc) & 0xFFFF] << 8)
				  : 0;

	if (_labels.size())
		{
		char *ebuf		= buf + 19;
		const String&l	= _getLabel(pc);
		buf = _printLabelMax(buf, l, 16);

		if (l.length())
			*buf++ = ':';

		while (buf < ebuf)
			*buf++ = ' ';
		}
	else
		{
		*buf++ = ':';
		*buf++ = ' ';
		}

	int ln = (_labels.size()) ? 31 : 21;
	memset(buf, ' ', ln + 9);
	buf[ln] = ';';

	uint32_t c = _insnLength[insn];
	_printMemCount(buf + ln + 2, pc, c);

	buf[ln + 2 + c * 4] = 0;

	switch (insn)
		{
		case 0x00: INSPRT_IMP("BRK"); break;
		case 0x01: INSPRT_IDX("ORA"); break;
		case 0x02: INSPRT_IMP("kil"); break;
		case 0x03: INSPRT_IDYW("slo"); break;
		case 0x04: INSPRT_ZPG("dop"); break;
		case 0x05: INSPRT_ZPG("ORA"); break;
		case 0x06: INSPRT_ZPG("ASL"); break;
		case 0x07: INSPRT_ZPG("slo"); break;
		case 0x08: INSPRT_IMP("PHP"); break;
		case 0x09: INSPRT_IMM("ORA"); break;
		case 0x0A: INSPRT_ACC("ASL"); break;
		case 0x0B: INSPRT_IMM("aac"); break;
		case 0x0C: INSPRT_ABS("top"); break;
		case 0x0D: INSPRT_ABS("ORA"); break;
		case 0x0E: INSPRT_ABS("ASL"); break;
		case 0x0F: INSPRT_ABS("slo"); break;
		case 0x10: INSPRT_BRA("BPL"); break;
		case 0x11: INSPRT_IDY("ORA"); break;
		case 0x12: INSPRT_IMP("kil"); break;
		case 0x13: INSPRT_IDX("slo"); break;
		case 0x14: INSPRT_ZPX("dop"); break;
		case 0x15: INSPRT_ZPX("ORA"); break;
		case 0x16: INSPRT_ZPX("ASL"); break;
		case 0x17: INSPRT_ZPX("slo"); break;
		case 0x18: INSPRT_IMP("CLC"); break;
		case 0x19: INSPRT_ABY("ORA"); break;
		case 0x1A: INSPRT_IMP("nop"); break;
		case 0x1B: INSPRT_ABY("slo"); break;
		case 0x1C: INSPRT_ABX("top"); break;
		case 0x1D: INSPRT_ABX("ORA"); break;
		case 0x1E: INSPRT_ABXW("ASL"); break;
		case 0x1F: INSPRT_ABXW("slo"); break;
		case 0x20: INSPRT_ABS("JSR"); break;
		case 0x21: INSPRT_IDX("AND"); break;
		case 0x22: INSPRT_IMP("kil"); break;
		case 0x23: INSPRT_IDX("rla"); break;
		case 0x24: INSPRT_ZPG("BIT"); break;
		case 0x25: INSPRT_ZPG("AND"); break;
		case 0x26: INSPRT_ZPG("ROL"); break;
		case 0x27: INSPRT_ZPG("rla"); break;
		case 0x28: INSPRT_IMP("PLP"); break;
		case 0x29: INSPRT_IMM("AND"); break;
		case 0x2A: INSPRT_ACC("ROL"); break;
		case 0x2B: INSPRT_IMM("aac"); break;
		case 0x2C: INSPRT_ABS("BIT"); break;
		case 0x2D: INSPRT_ABS("AND"); break;
		case 0x2E: INSPRT_ABS("ROL"); break;
		case 0x2F: INSPRT_ABS("rla"); break;
		case 0x30: INSPRT_BRA("BMI"); break;
		case 0x31: INSPRT_IDY("AND"); break;
		case 0x32: INSPRT_IMP("kil"); break;
		case 0x33: INSPRT_IDYW("rla"); break;
		case 0x34: INSPRT_ZPX("dop"); break;
		case 0x35: INSPRT_ZPX("AND"); break;
		case 0x36: INSPRT_ZPX("ROL"); break;
		case 0x37: INSPRT_ZPX("rla"); break;
		case 0x38: INSPRT_IMP("SEC"); break;
		case 0x39: INSPRT_ABY("AND"); break;
		case 0x3A: INSPRT_IMP("nop"); break;
		case 0x3B: INSPRT_ABYW("rla"); break;
		case 0x3C: INSPRT_ABX("top"); break;
		case 0x3D: INSPRT_ABX("AND"); break;
		case 0x3E: INSPRT_ABXW("ROL"); break;
		case 0x3F: INSPRT_ABXW("rla"); break;
		case 0x40: INSPRT_IMP("RTI"); break;
		case 0x41: INSPRT_IDX("EOR"); break;
		case 0x42: INSPRT_IMP("kil"); break;
		case 0x43: INSPRT_IDX("sre"); break;
		case 0x44: INSPRT_ZPG("dop"); break;
		case 0x45: INSPRT_ZPG("EOR"); break;
		case 0x46: INSPRT_ZPG("LSR"); break;
		case 0x47: INSPRT_ZPG("sre"); break;
		case 0x48: INSPRT_IMP("PHA"); break;
		case 0x49: INSPRT_IMM("EOR"); break;
		case 0x4A: INSPRT_ACC("LSR"); break;
		case 0x4B: INSPRT_IMM("asr"); break;
		case 0x4C: INSPRT_ABS("JMP"); break;
		case 0x4D: INSPRT_ABS("EOR"); break;
		case 0x4E: INSPRT_ABS("LSR"); break;
		case 0x4F: INSPRT_ABS("sre"); break;
		case 0x50: INSPRT_BRA("BVC"); break;
		case 0x51: INSPRT_IDY("EOR"); break;
		case 0x52: INSPRT_IMP("kil"); break;
		case 0x53: INSPRT_IDYW("sre"); break;
		case 0x54: INSPRT_ZPX("dop"); break;
		case 0x55: INSPRT_ZPX("EOR"); break;
		case 0x56: INSPRT_ZPX("LSR"); break;
		case 0x57: INSPRT_ZPX("sre"); break;
		case 0x58: INSPRT_IMP("CLI"); break;
		case 0x59: INSPRT_ABY("EOR"); break;
		case 0x5A: INSPRT_IMP("nop"); break;
		case 0x5B: INSPRT_ABYW("sre"); break;
		case 0x5C: INSPRT_ABX("top"); break;
		case 0x5D: INSPRT_ABX("EOR"); break;
		case 0x5E: INSPRT_ABXW("LSR"); break;
		case 0x5F: INSPRT_ABXW("sre"); break;
		case 0x60: INSPRT_IMP("RTS"); break;
		case 0x61: INSPRT_IDX("ADC"); break;
		case 0x62: INSPRT_IMP("kil"); break;
		case 0x63: INSPRT_IDX("rra"); break;
		case 0x64: INSPRT_ZPG("dop"); break;
		case 0x65: INSPRT_ZPG("ADC"); break;
		case 0x66: INSPRT_ZPG("ROR"); break;
		case 0x67: INSPRT_ZPG("rra"); break;
		case 0x68: INSPRT_IMP("PLA"); break;
		case 0x69: INSPRT_IMM("ADC"); break;
		case 0x6A: INSPRT_ACC("ROR"); break;
		case 0x6B: INSPRT_IMM("arr"); break;
		case 0x6C: INSPRT_IND("JMP"); break;
		case 0x6D: INSPRT_ABS("ADC"); break;
		case 0x6E: INSPRT_ABS("ROR"); break;
		case 0x6F: INSPRT_ABS("rra"); break;
		case 0x70: INSPRT_BRA("BVS"); break;
		case 0x71: INSPRT_IDY("ADC"); break;
		case 0x72: INSPRT_IMP("kil"); break;
		case 0x73: INSPRT_IDYW("rra"); break;
		case 0x74: INSPRT_ZPX("dop"); break;
		case 0x75: INSPRT_ZPX("ADC"); break;
		case 0x76: INSPRT_ZPX("ROR"); break;
		case 0x77: INSPRT_ZPX("rra"); break;
		case 0x78: INSPRT_IMP("SEI"); break;
		case 0x79: INSPRT_ABY("ADC"); break;
		case 0x7A: INSPRT_IMP("nop"); break;
		case 0x7B: INSPRT_ABYW("rra"); break;
		case 0x7C: INSPRT_ABX("top"); break;
		case 0x7D: INSPRT_ABX("ADC"); break;
		case 0x7E: INSPRT_ABXW("ROR"); break;
		case 0x7F: INSPRT_ABXW("rra"); break;
		case 0x80: INSPRT_IMM("dop"); break;
		case 0x81: INSPRT_IDX("STA"); break;
		case 0x82: INSPRT_IMM("dop"); break;
		case 0x83: INSPRT_IDX("aax"); break;
		case 0x84: INSPRT_ZPG("STY"); break;
		case 0x85: INSPRT_ZPG("STA"); break;
		case 0x86: INSPRT_ZPG("STX"); break;
		case 0x87: INSPRT_ZPG("aax"); break;
		case 0x88: INSPRT_IMP("DEY"); break;
		case 0x89: INSPRT_IMM("dop"); break;
		case 0x8A: INSPRT_IMP("TXA"); break;
		case 0x8B: INSPRT_IMM("xaa"); break;
		case 0x8C: INSPRT_ABS("STY"); break;
		case 0x8D: INSPRT_ABS("STA"); break;
		case 0x8E: INSPRT_ABS("STX"); break;
		case 0x8F: INSPRT_ABS("aax"); break;
		case 0x90: INSPRT_BRA("BCC"); break;
		case 0x91: INSPRT_IDYW("STA"); break;
		case 0x92: INSPRT_IMP("kil"); break;
		case 0x93: INSPRT_IDYW("axa"); break;
		case 0x94: INSPRT_ZPX("STY"); break;
		case 0x95: INSPRT_ZPX("STA"); break;
		case 0x96: INSPRT_ZPY("STX"); break;
		case 0x97: INSPRT_ZPX("aax"); break;
		case 0x98: INSPRT_IMP("TYA"); break;
		case 0x99: INSPRT_ABYW("STA"); break;
		case 0x9A: INSPRT_IMP("TXS"); break;
		case 0x9B: INSPRT_ABYW("xas"); break;
		case 0x9C: INSPRT_ABXW("sya"); break;
		case 0x9D: INSPRT_ABXW("STA"); break;
		case 0x9E: INSPRT_ABYW("sxa"); break;
		case 0x9F: INSPRT_ABYW("axa"); break;
		case 0xA0: INSPRT_IMM("LDY"); break;
		case 0xA1: INSPRT_IDX("LDA"); break;
		case 0xA2: INSPRT_IMM("LDX"); break;
		case 0xA3: INSPRT_IDX("lax"); break;
		case 0xA4: INSPRT_ZPG("LDY"); break;
		case 0xA5: INSPRT_ZPG("LDA"); break;
		case 0xA6: INSPRT_ZPG("LDX"); break;
		case 0xA7: INSPRT_ZPG("lax"); break;
		case 0xA8: INSPRT_IMP("TAY"); break;
		case 0xA9: INSPRT_IMM("LDA"); break;
		case 0xAA: INSPRT_IMP("TAX"); break;
		case 0xAB: INSPRT_IMP("atx"); break;
		case 0xAC: INSPRT_ABS("LDY"); break;
		case 0xAD: INSPRT_ABS("LDA"); break;
		case 0xAE: INSPRT_ABS("LDX"); break;
		case 0xAF: INSPRT_ABS("lax"); break;
		case 0xB0: INSPRT_BRA("BCS"); break;
		case 0xB1: INSPRT_IDY("LDA"); break;
		case 0xB2: INSPRT_IMP("kil"); break;
		case 0xB3: INSPRT_IDY("lax"); break;
		case 0xB4: INSPRT_ZPX("LDY"); break;
		case 0xB5: INSPRT_ZPX("LDA"); break;
		case 0xB6: INSPRT_ZPY("LDX"); break;
		case 0xB7: INSPRT_ZPX("lax"); break;
		case 0xB8: INSPRT_IMP("CLV"); break;
		case 0xB9: INSPRT_ABY("LDA"); break;
		case 0xBA: INSPRT_IMP("TSX"); break;
		case 0xBB: INSPRT_ABY("lar"); break;
		case 0xBC: INSPRT_ABX("LDY"); break;
		case 0xBD: INSPRT_ABX("LDA"); break;
		case 0xBE: INSPRT_ABY("LDX"); break;
		case 0xBF: INSPRT_ABY("lax"); break;
		case 0xC0: INSPRT_IMM("CPY"); break;
		case 0xC1: INSPRT_IDX("CMP"); break;
		case 0xC2: INSPRT_IMM("dop"); break;
		case 0xC3: INSPRT_IDX("dcp"); break;
		case 0xC4: INSPRT_ZPG("CPY"); break;
		case 0xC5: INSPRT_ZPG("CMP"); break;
		case 0xC6: INSPRT_ZPG("DEC"); break;
		case 0xC7: INSPRT_ZPG("dcp"); break;
		case 0xC8: INSPRT_IMP("INY"); break;
		case 0xC9: INSPRT_IMM("CMP"); break;
		case 0xCA: INSPRT_IMP("DEX"); break;
		case 0xCB: INSPRT_IMM("axs"); break;
		case 0xCC: INSPRT_ABS("CPY"); break;
		case 0xCD: INSPRT_ABS("CMP"); break;
		case 0xCE: INSPRT_ABS("DEC"); break;
		case 0xCF: INSPRT_ABS("dcp"); break;
		case 0xD0: INSPRT_BRA("BNE"); break;
		case 0xD1: INSPRT_IDY("CMP"); break;
		case 0xD2: INSPRT_IMP("kil"); break;
		case 0xD3: INSPRT_IDYW("dcp"); break;
		case 0xD4: INSPRT_ZPX("dop"); break;
		case 0xD5: INSPRT_ZPX("CMP"); break;
		case 0xD6: INSPRT_ZPX("DEC"); break;
		case 0xD7: INSPRT_ZPX("dcp"); break;
		case 0xD8: INSPRT_IMP("CLD"); break;
		case 0xD9: INSPRT_ABY("CMP"); break;
		case 0xDA: INSPRT_IMP("nop"); break;
		case 0xDB: INSPRT_ABYW("dcp"); break;
		case 0xDC: INSPRT_ABX("top"); break;
		case 0xDD: INSPRT_ABX("CMP"); break;
		case 0xDE: INSPRT_ABXW("DEC"); break;
		case 0xDF: INSPRT_ABXW("dcp"); break;
		case 0xE0: INSPRT_IMM("CPX"); break;
		case 0xE1: INSPRT_IDX("SBC"); break;
		case 0xE2: INSPRT_IMM("dop"); break;
		case 0xE3: INSPRT_IDX("isc"); break;
		case 0xE4: INSPRT_ZPG("CPX"); break;
		case 0xE5: INSPRT_ZPG("SBC"); break;
		case 0xE6: INSPRT_ZPG("INC"); break;
		case 0xE7: INSPRT_ZPG("isc"); break;
		case 0xE8: INSPRT_IMP("INX"); break;
		case 0xE9: INSPRT_IMM("SBC"); break;
		case 0xEA: INSPRT_IMP("NOP"); break;
		case 0xEB: INSPRT_IMM("sbc"); break;
		case 0xEC: INSPRT_ABS("CPX"); break;
		case 0xED: INSPRT_ABS("SBC"); break;
		case 0xEE: INSPRT_ABS("INC"); break;
		case 0xEF: INSPRT_ABS("isc"); break;
		case 0xF0: INSPRT_BRA("BEQ"); break;
		case 0xF1: INSPRT_IDY("SBC"); break;
		case 0xF2: INSPRT_IMP("kil"); break;
		case 0xF3: INSPRT_IDYW("isc"); break;
		case 0xF4: INSPRT_ZPX("dop"); break;
		case 0xF5: INSPRT_ZPX("SBC"); break;
		case 0xF6: INSPRT_ZPX("INC"); break;
		case 0xF7: INSPRT_ZPX("isc"); break;
		case 0xF8: INSPRT_IMP("SED"); break;
		case 0xF9: INSPRT_ABY("SBC"); break;
		case 0xFA: INSPRT_IMP("nop"); break;
		case 0xFB: INSPRT_ABYW("isc"); break;
		case 0xFC: INSPRT_ABX("top"); break;
		case 0xFD: INSPRT_ABX("SBC"); break;
		case 0xFE: INSPRT_ABXW("INC"); break;
		case 0xFF: INSPRT_ABXW("isc"); break;
		}
	}

/*****************************************************************************\
|* Trace printing: show a label to a maximum length
\*****************************************************************************/
char * Simulator::_printLabelMax(char *buf, const String& label, int max)
	{
	int ln			= (int) label.length();
	const char *lbl	= label.c_str();

	if (ln > max)
		{
		*buf++ = '?';
		memcpy(buf, lbl + ln - max + 1, max - 1);
		buf += max - 1;
		}
	else
		{
		memcpy(buf, lbl, ln);
		buf += ln;
		}

	return buf;
	}

/*****************************************************************************\
|* Trace printing: print a zero-page label
\*****************************************************************************/
char * Simulator::_printZpLabel(char *buf, uint16_t address, char idx)
	{
	const String& label = _getLabel(address);
	const char *l		= label.c_str();

	if (l && *l)
		buf = _printLabelMax(buf, label, 24);
	else
		{
		*buf++ = '$';
		buf = _hex2(buf, address);
		}
	if (idx)
		{
		*buf++ = ',';
		*buf++ = idx;
		}
	return buf;
	}

/*****************************************************************************\
|* Trace printing: print an absolute label
\*****************************************************************************/
char * Simulator::_printAbsLabel(char *buf, uint16_t address, char idx)
	{
	const String& label = _getLabel(address);
	const char *l		= label.c_str();

	if (l && *l)
		buf = _printLabelMax(buf, label, 24);
	else
		{
		*buf++ = '$';
		buf = _hex4(buf, address);
		}
	if (idx)
		{
		*buf++ = ',';
		*buf++ = idx;
		}
	return buf;
	}

/*****************************************************************************\
|* Trace printing: print an indirect label
\*****************************************************************************/
char * Simulator::_printIndLabel(char *buf, uint16_t address, char idx, int hint)
	{
	const String& label = _getLabel(address);
	const char *l		= label.c_str();

	*buf++ = '(';
	if (l && *l)
		buf = _printLabelMax(buf, label, 14);
	else
		{
		*buf++ = '$';
		if (idx)
			buf = _hex2(buf, address);
		else
			buf = _hex4(buf, address);
		}

	if (idx == 'Y')
		*buf++ = ')';

	if (idx)
		{
		*buf++ = ',';
		*buf++ = idx;
		}

	if (idx != 'Y')
		*buf++ = ')';

	if (hint)
		{
		*buf++ = ' ';
		*buf++ = '[';
		*buf++ = '$';

		if (idx == 'X')
			buf = _hex4(buf, _readWord(0xFF & (address + _regs.x)));
		else if (idx == 'Y')
			buf = _hex4(buf, _readWord(address) + _regs.y);
		else
			buf = _hex4(buf, _readWord(address));
		*buf++ = ']';
		}

	return buf;
	}

/*****************************************************************************\
|* Trace printing: show N bytes of memory
\*****************************************************************************/
void Simulator::_printMemCount(char *buf, uint32_t address, unsigned len)
	{
	unsigned i;
	for (i = 0; i < len; i++)
		_printMem(buf + i * 4, address + i);
	}

/*****************************************************************************\
|* Trace printing: show memory
\*****************************************************************************/
void Simulator::_printMem(char *buf, uint32_t address)
	{
	address &= 0xFFFF;
	if (!(_memState[address] & MS_INVALID))
		{
		if (!(_memState[address] & MS_ROM))
			{
			buf[0] = '[';
			_hex2(buf + 1, _mem[address]);
			buf[3] = ']';
			buf[4] = 0;
			}
		else
			{
			buf[0] = '{';
			_hex2(buf + 1, _mem[address]);
			buf[3] = '}';
			buf[4] = 0;
			}
		}
	else if (_memState[address] & MS_UNDEFINED)
		memcpy(buf, "[UU]", 4);
	else
		memcpy(buf, "[NN]", 4);
	}
