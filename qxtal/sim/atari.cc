
#include "atari.h"
#include "display.h"
#include "io.h"
#include "simulator.h"


/*****************************************************************************\
|* screen mode settings
\*****************************************************************************/
static const int _sx[] = {   40,  20,  20,  40,  80,  80,  160, 160,
							 320, 80,  80,  80,  40,  40,  160, 160 };
static const int _sy[] = {   24,  24,  12,  24,  48,  48,  96,  96,
							 192, 192, 192, 192, 24,  12,  192, 192 };
static const int _numc[] = { 256, 256, 256, 4,   2,   4,   2,   4,
							 2,   16,  16,  16,  256, 256, 2,   4 };

/*****************************************************************************\
|* EDITOR defs
\*****************************************************************************/
#define LMARGN (0x52)			// left margin
#define RMARGN (0x53)			// right margin
#define ROWCRS (0x54)			// cursor row
#define COLCRS (0x55)			// cursor column (2 byte)

/*****************************************************************************\
|* SCREEN defs
\*****************************************************************************/
#define ATACHR (0x2FB)
#define FILDAT (0x2FD)
#define FILFLG (0x2B7)

#define LO(a) ((a)&0xFF)
#define HI(a) ((a) >> 8)

/*****************************************************************************\
|* HATAB defs
\*****************************************************************************/
#define HATABS (0x031A)			// Device handlers table
#define EDITRV (0xE400)
#define SCRENV (0xE410)
#define KEYBDV (0xE420)
#define PRINTV (0xE430)
#define CASETV (0xE440)
#define DISKDV (0xE3F0)			// Emulated DOS, not there in real hardware!
#define EDITOR_OFFSET (6)		// NOTE: Editor must be entry at offset "6"

#define HATABS_ENTRY(a, b) (a), LO(b), HI(b)
static const unsigned char hatab_default[] =
	{
	HATABS_ENTRY('P', PRINTV),
	HATABS_ENTRY('C', CASETV),
	HATABS_ENTRY('E', EDITRV), // NOTE: Editor must be entry at offset "6"
	HATABS_ENTRY('S', SCRENV),
	HATABS_ENTRY('K', KEYBDV),
	HATABS_ENTRY('D', DISKDV),
	HATABS_ENTRY(0, 0),
	HATABS_ENTRY(0, 0),
	HATABS_ENTRY(0, 0),
	HATABS_ENTRY(0, 0),
	HATABS_ENTRY(0, 0)
	};

/*****************************************************************************\
|* Fake routines addresses: Bases
\*****************************************************************************/
#define EDITR_BASE 0xE500
#define SCREN_BASE 0xE508
#define KEYBD_BASE 0xE510
#define PRINT_BASE 0xE518
#define CASET_BASE 0xE520
#define DISKD_BASE 0xE528

/*****************************************************************************\
|* Fake routines addresses: Offsets
\*****************************************************************************/
#define DEVR_OPEN (0)           // 3
#define DEVR_CLOSE (1)          // 12
#define DEVR_GET (2)            // 4,5,6,7
#define DEVR_PUT (3)            // 8.9,10,11
#define DEVR_STATUS (4)         // 13
#define DEVR_SPECIAL (5)        // 14 and up
#define DEVR_INIT (6)

#define DEVH_E(a) LO(a - 1), HI(a - 1)
#define DEVH_TAB(a)                      \
	DEVH_E(a##_BASE + DEVR_OPEN),        \
		DEVH_E(a##_BASE + DEVR_CLOSE),   \
		DEVH_E(a##_BASE + DEVR_GET),     \
		DEVH_E(a##_BASE + DEVR_PUT),     \
		DEVH_E(a##_BASE + DEVR_STATUS),  \
		DEVH_E(a##_BASE + DEVR_SPECIAL), \
		0x20, DEVH_E(a##_BASE + DEVR_INIT + 1), 0x00

/*****************************************************************************\
|* Standard Handlers
\*****************************************************************************/
static const unsigned char devhand_tables[] =
	{
	DEVH_TAB(EDITR),
	DEVH_TAB(SCREN),
	DEVH_TAB(KEYBD),
	DEVH_TAB(PRINT),
	DEVH_TAB(CASET)
	};

/*****************************************************************************\
|* Emulated DOS handler
\*****************************************************************************/
static const unsigned char devhand_emudos[] =
	{
	DEVH_TAB(DISKD)
	};

/*****************************************************************************\
|* IOCB defs
\*****************************************************************************/
#define CIOV (0xE456)
#define CIOERR (0xE530)			// FIXME: CIO error return routine
#define IC(a) (regs->x + IC##a) // IOCB address
#define GET_IC(a) _sim->getByte(IC(a))
#define CIOCHR (0x2F)  //         ;CHARACTER BYTE FOR CURRENT OPERATION
#define ICHID (0x0340) //         ;HANDLER INDEX NUMBER (FF=IOCB FREE)
#define ICDNO (0x0341) //         ;DEVICE NUMBER (DRIVE NUMBER)
#define ICCOM (0x0342) //         ;COMMAND CODE
#define ICSTA (0x0343) //         ;STATUS OF LAST IOCB ACTION
#define ICBAL (0x0344) //         ;1-byte low buffer address
#define ICBAH (0x0345) //         ;1-byte high buffer address
#define ICPTL (0x0346) //         ;1-byte low PUT-BYTE routine address - 1
#define ICPTH (0x0347) //         ;1-byte high PUT-BYTE routine address - 1
#define ICBLL (0x0348) //         ;1-byte low buffer length
#define ICBLH (0x0349) //         ;1-byte high buffer length
#define ICAX1 (0x034A) //         ;1-byte first auxiliary information
#define ICAX2 (0x034B) //         ;1-byte second auxiliary information
#define ICAX3 (0x034C) //         ;1-byte third auxiliary information
#define ICAX4 (0x034D) //         ;1-byte fourth auxiliary information
#define ICAX5 (0x034E) //         ;1-byte fifth auxiliary information
#define ICSPR (0x034F) //         ;SPARE BYTE

static const unsigned char iocv_empty[16] =
	{
	0xFF, 0, 0, 0, // HID, DNO, COM, STA
	0, 0, LO(CIOERR - 1), HI(CIOERR - 1), // BAL, BAH, PTL, PTH
	0, 0, 0, 0, // BLL, BLH, AX1, AX2
	0, 0, 0, 0 // AX3, AX4, AX5, SPR
	};

/*****************************************************************************\
|* State, filled in by init() or constructor
\*****************************************************************************/
static IO*				_io;			// Input/Output channel
static Simulator*		_sim;			// Simulator engine
static Display *		_dpy;			// Simulated screen

/*****************************************************************************\
|* Generated state
\*****************************************************************************/
static uint32_t editr_last_row = 0;
static uint32_t editr_last_col = 0;


/*****************************************************************************\
|* Constructor - call this or the init() method
\*****************************************************************************/
Atari::Atari(Simulator* sim, IO *io, QObject *parent)
	  :QObject{parent}
	{
	_io = io;
	_sim = sim;
	_dpy = new Display(this);
	_dpy->init();
	}

void init(Simulator* sim, IO *io)
	{
	_io = io;
	_sim = sim;
	_dpy = new Display();
	_dpy->init();
	}

#pragma mark -- screen handling



/*****************************************************************************\
|* Handle a screen command
\*****************************************************************************/
void Atari::_screen(ScreenCmd cmd,
					int x,
					int y,
					int data,
					Simulator::Registers *regs)
	{
	int colIdx = data % _dpy->colours();

	switch (cmd)
		{
		case SCR_GRAPHICS:
			_sim->warn("SCREEN: open mode %d", 0x10 ^ data);
			_io->printf("SCREEN: set graphics %d%s%s\n", data & 15,
						 data & 16 ? " with text window": "",
						 data & 32 ? " don't clear" : "" );

			_dpy->setW(_sx[data & 15]);
			_dpy->setH(_sy[data & 15]);
			_dpy->setColours(_numc[data & 15]);
			_dpy->init();
			_dpy->fill(((data & 32) == 0) ? 0 : 16);
			regs->y = 0;
			return;

		case SCR_LOCATE:
			_sim->warn("SCREEN: get (locate) @(%d, %d)", x, y);
			_io->printf("SCREEN: locate %d,%d\n", x, y);
			if (_dpy->inBounds(x, y))
				regs->a = _dpy->colourAt(x, y);
			break;

		case SCR_PLOT:
			_sim->warn("SCREEN: put (plot) @(%d, %d) color: %d", x, y, data);
			_io->printf("SCREEN: plot %d,%d  color %d\n", x, y, colIdx);
			if (_dpy->inBounds(x, y))
				_dpy->plot(x, y, colIdx);
			break;

		case SCR_DRAWTO:
			data &= 0xFF;
			_sim->warn("SCREEN: line (drawto) @(%d, %d) color: %d", x, y, data);
			_io->printf("SCREEN: draw to %d,%d  color %d\n", x, y, colIdx);
			_dpy->drawTo(x, y, colIdx);
			break;

		case SCR_FILLTO:
			// FIXME: emulate line fill
			_sim->warn("SCREEN: special (fillto) @(%d, %d) color: %d  fcolor:%d",
						  x, y, data & 0xFF, data >> 8);
			_io->printf("SCREEN: fill to %d,%d  color %d, fill color %d\n",
					x, y, (data & 0xFF) % _dpy->colours(),
					   (data >>8) % _dpy->colours() );
			break;
		}
	regs->y = _dpy->inBounds(x, y) ? 0 : 1;
	}


#pragma mark -- CIO

/*****************************************************************************\
|* CIO: exit
\*****************************************************************************/
int Atari::_cioExit(Simulator::Registers *regs)
	{
	_poke(IC(STA), regs->y);
	if (regs->y & 0x80)
		_sim->setFlags(Simulator::FLAG_N, Simulator::FLAG_N);
	else
		_sim->setFlags(Simulator::FLAG_N, 0);
	return 0;
	}

/*****************************************************************************\
|* CIO: error and exit
\*****************************************************************************/
int Atari::_cioError(Simulator::Registers *regs,
					 const char *err,
					 uint32_t value)
	{
	regs->y = value & 0xFF;
	_sim->warn("CIO: %s", err);
	return _cioExit(regs);
	}

/*****************************************************************************\
|* CIO: ok
\*****************************************************************************/
int Atari::_cioOk(Simulator::Registers *regs, uint32_t acc)
	{
	regs->a = acc & 0xFF;
	regs->y = 1;
	return _cioExit(regs);
	}

/*****************************************************************************\
|* CIO: call through device table
\*****************************************************************************/
void Atari::_cioCallDevTab(Simulator::Registers *regs, uint16_t devtab, int fn)
	{
	if (fn == DEVR_PUT)
		_poke(CIOCHR, regs->a);

	_sim->call(1 + _dpeek(devtab + 2 * fn), regs);

	if (fn == DEVR_GET)
		_poke(CIOCHR, regs->a);
	}

/*****************************************************************************\
|* CIO: handler
\*****************************************************************************/
int Atari::_cio(Simulator::Registers *regs, uint32_t address, int data)
	{
	if (regs->x & 0x0F || regs->x >= 0x80)
		return _cioError(regs, "invalid value of X register", 134);

	unsigned hid  = GET_IC(HID);
	unsigned badr = GET_IC(BAL) | (GET_IC(BAH) << 8);
	unsigned blen = GET_IC(BLL) | (GET_IC(BLH) << 8);
	unsigned com  = GET_IC(COM);
	unsigned ax1  = GET_IC(AX1);

	unsigned devtab = _dpeek(1 + hid + HATABS);

	if (hid == 0xFF && com != 3 && com < 12)
		return _cioError(regs, "channel not open", 133);

	if (com < 3)
		{
		_sim->warn("CIO CMD = %d", com);
		return _cioError(regs, "invalid command", 132);
		}

	if ((com >= 4) && (com < 8) && !(ax1 & 0x4))
		return _cioError(regs, "write only", 131);

	if ((com >= 8) && (com < 12) && !(ax1 & 0x8))
		return _cioError(regs, "read only", 135);

	/*************************************************************************\
	|* CIO Command: Open
	\*************************************************************************/
	if (com == 3)
		{
		_sim->warn("CIO open %c%c", _peek(badr), _peek(badr + 1));

		// OPEN (command 0)
		if (GET_IC(HID) != 0xFF)
			return _cioError(regs, "channel already opened", 129);

		// Search handle and call open routine.
		unsigned dev = _peek(badr);
		unsigned num = _peek(badr + 1) - '0';
		if (num > 9)
			num = 0;

		// Store DeviceNumber from filename
		_poke(IC(DNO), num);

		// Search HATAB
		int i;
		for (i = 0; i < 32; i += 3)
			{
			if (_peek(HATABS + i) == dev)
				{
				// Copy data
				unsigned devtab = _dpeek(1 + i + HATABS);
				_poke(IC(HID), i);
				_dpoke(IC(PTL), _dpeek(devtab + 6));

				// Found, call open
				_cioCallDevTab(regs, devtab, DEVR_OPEN);
				return _cioExit(regs);
				}
			}

		// Return error
		regs->y = 0x82;
		return _cioExit(regs);
		}

	/*************************************************************************\
	|* CIO Command: Get record
	\*************************************************************************/
	else if (com == 4 || com == 5)
		{
		// GET RECORD
		for (;;)
			{
			// Get single
			_cioCallDevTab(regs, devtab, DEVR_GET);
			if (regs->y & 0x80)
				break;

			if (blen)
				{
				_poke(badr, regs->a);
				badr++;
				blen--;
				}

			if (regs->a == 0x9B)
				break;
			}
		_dpoke(IC(BLL), _dpeek(IC(BLL)) - blen);
		return _cioExit(regs);
		}

	/*************************************************************************\
	|* CIO Command: Get chars
	\*************************************************************************/
	else if (com == 6 || com == 7)
		{
		// GET CHARS
		if (!blen)
			{
			// Get single
			_cioCallDevTab(regs, devtab, DEVR_GET);
			return _cioExit(regs);
			}
		else
			{
			while (blen)
				{
				// get
				_cioCallDevTab(regs, devtab, DEVR_GET);
				if (regs->y & 0x80)
					break;
				_poke(badr, regs->a);
				badr++;
				blen--;
				}
			// Must return number of bytes transfered
			_sim->warn("ICBL ends at %04x, transfered %d bytes",
						  blen, _dpeek(IC(BLL)) - blen);
			_dpoke(IC(BLL), _dpeek(IC(BLL)) - blen);
			return _cioExit(regs);
			}
		}

	/*************************************************************************\
	|* CIO Command: Put record
	\*************************************************************************/
	else if (com == 8 || com == 9)
		{
		// PUT RECORD
		if (!blen)
			{
			// Put single
			regs->a = 0x9B;
			_cioCallDevTab(regs, devtab, DEVR_PUT);
			return _cioExit(regs);
			}
		else
			{
			while (blen)
				{
				regs->a = _peek(badr);
				_cioCallDevTab(regs, devtab, DEVR_PUT);
				if (regs->y & 0x80)
					break;

				badr++;
				blen--;
				if (_peek(CIOCHR) == 0x9B)
					break;
				}

			// Must return number of bytes transfered
			_dpoke(IC(BLL), _dpeek(IC(BLL)) - blen);
			return _cioExit(regs);
			}
		}

	/*************************************************************************\
	|* CIO Command: Put chars
	\*************************************************************************/
	else if (com == 10 || com == 11)
		{
		// PUT CHARS
		if (!blen)
			{
			// Put single
			_cioCallDevTab(regs, devtab, DEVR_PUT);
			return _cioExit(regs);
			}
		else
			{
			while (blen)
				{
				regs->a = _peek(badr);
				_cioCallDevTab(regs, devtab, DEVR_PUT);
				if (regs->y & 0x80)
					break;
				badr++;
				blen--;
				}

			// Must return number of bytes transfered
			_dpoke(IC(BLL), _dpeek(IC(BLL)) - blen);
			return _cioExit(regs);
			}
		}

	/*************************************************************************\
	|* CIO Command: Close
	\*************************************************************************/
	else if (com == 12)
		{
		// CLOSE
		regs->y = 1;
		if (GET_IC(HID) != 0xFF)
			{
			// Call close handler
			_cioCallDevTab(regs, devtab, DEVR_CLOSE);
			}
		_poke(IC(HID), 0xFF);
		_dpoke(IC(PTL), CIOERR - 1);
		return _cioExit(regs);
		}

	/*************************************************************************\
	|* CIO Command: Status
	\*************************************************************************/
	else if (com == 13)
		{
		// GET STATUS
		}


	/*************************************************************************\
	|* CIO Command: Special
	\*************************************************************************/
	else if (com >= 14)
		{
		// SPECIAL
		if (com == 37)
			{
			unsigned ax3 = GET_IC(AX3);
			unsigned ax4 = GET_IC(AX4);
			unsigned ax5 = GET_IC(AX5);
			_sim->warn("POINT %d/%d/%d", ax3, ax4, ax5);
			return _cioOk(regs, 0);
			}

		// Call close handler
		_cioCallDevTab(regs, devtab, DEVR_SPECIAL);
		return _cioExit(regs);
		}

	return 0;
	}


/*****************************************************************************\
|* CIO: Error handler
\*****************************************************************************/
int Atari::_cioErr(Simulator::Registers *regs, uint32_t addr, int data)
	{
	_sim->warn("CIO error, IOCB not open");
	regs->y = 0x83;
	_sim->setFlags(Simulator::FLAG_N, Simulator::FLAG_N);
	return 0;
	}



#pragma mark -- Editor

int Atari::_editor(Simulator::Registers *regs, uint32_t addr, int data)
	{
	switch (addr & 7)
		{
		case DEVR_OPEN:
			_sim->warn("EDITR cmd OPEN");
			return 0;

		case DEVR_CLOSE:
			return 0;

		case DEVR_GET:
			{
			int c   = _io->getchar();
			regs->y = 1;
			if (c == EOF)
				regs->y = 136;
			else
				regs->a = c;
			return 0;
			}

		case DEVR_PUT:
			{
			unsigned row = _peek(ROWCRS);
			unsigned col = _dpeek(COLCRS);

			// Detect POS changes
			if (row != editr_last_row || col != editr_last_col)
				{
				if (row != editr_last_row)
					_io->putchar(0x9B);

				_sim->warn("EDITOR position from (%d,%d) to (%d,%d)",
							  editr_last_row, editr_last_col, row, col);
				}
			if (regs->a == 0x9B || col == _dpeek(RMARGN))
				{
				col = _peek(LMARGN) - 1;
				if (row < 24)
					row ++;
				}

			_io->putchar(regs->a);
			col ++;
			_dpoke(COLCRS, col);
			_poke(ROWCRS, row);
			editr_last_row = row;
			editr_last_col = col;
			regs->y = 1;
			return 0;
			}

		case DEVR_STATUS:
			_sim->warn("EDITOR cmd STATUS");
			return 0;

		case DEVR_SPECIAL:
			_sim->warn("EDITOR cmd SPECIAL");
			return 0; // Nothing

		case DEVR_INIT:
			_sim->warn("EDITR cmd INIT");
			return 0;

		default:
			return _cbError(addr);
		}
	}



#pragma mark -- Utility methods

/*****************************************************************************\
|* Utility: error callback
\*****************************************************************************/
int Atari::_cbError(uint32_t address)
	{
	_sim->error("invalid access to cb address $%04x", address);
	return 0;
	}

/*****************************************************************************\
|* Utility: Poke a byte (and create the RAM)
\*****************************************************************************/
void Atari::_poke(uint32_t address, uint8_t val)
	{
	_sim->addRAM(address, &val, 1);
	}

/*****************************************************************************\
|* Utility: Poke a word (and create the RAM)
\*****************************************************************************/
void Atari::_dpoke(uint32_t address, uint32_t val)
	{
	_poke(address,     val & 0xFF);
	_poke(address + 1, val >> 8);
	}

/*****************************************************************************\
|* Utility: Peek at a byte
\*****************************************************************************/
uint8_t Atari::_peek(uint32_t address)
	{
	return _sim->getByte(address);
	}

/*****************************************************************************\
|* Utility: Peek at a word
\*****************************************************************************/
uint16_t Atari::_dpeek(uint32_t  address)
	{
	return _sim->getByte(address) + (((int)(_sim->getByte(address + 1))) << 8);
	}

/*****************************************************************************\
|* Utility: fill region with RTS
\*****************************************************************************/
void Atari::_addRtsCallback(uint32_t address, int len, Simulator::SIM_CB cb)
	{
	_sim->addCallback(cb, address, len, Simulator::CB_EXEC);

	uint8_t rts = 0x60;
	for (; len > 0; address++, len--)
		_sim->addROM(address, &rts, 1);
	}
