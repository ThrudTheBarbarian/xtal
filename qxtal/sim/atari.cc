
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
#define GET_IC(a) _sim.getByte(IC(a))
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
|* Constructor
\*****************************************************************************/
Atari::Atari(Simulator& sim, IO &io, QObject *parent)
	  :QObject{parent}
	  ,_io(io)
	  ,_sim(sim)
	{
	_dpy = new Display(this);
	_dpy->init();
	}



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
			_sim.warn("SCREEN: open mode %d", 0x10 ^ data);
			_io.printf("SCREEN: set graphics %d%s%s\n", data & 15,
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
			_sim.warn("SCREEN: get (locate) @(%d, %d)", x, y);
			_io.printf("SCREEN: locate %d,%d\n", x, y);
			if (_dpy->inBounds(x, y))
				regs->a = _dpy->colourAt(x, y);
			break;

		case SCR_PLOT:
			_sim.warn("SCREEN: put (plot) @(%d, %d) color: %d", x, y, data);
			_io.printf("SCREEN: plot %d,%d  color %d\n", x, y, colIdx);
			if (_dpy->inBounds(x, y))
				_dpy->plot(x, y, colIdx);
			break;

		case SCR_DRAWTO:
			data &= 0xFF;
			_sim.warn("SCREEN: line (drawto) @(%d, %d) color: %d", x, y, data);
			_io.printf("SCREEN: draw to %d,%d  color %d\n", x, y, colIdx);
			_dpy->drawTo(x, y, colIdx);
			break;

		case SCR_FILLTO:
			// FIXME: emulate line fill
			_sim.warn("SCREEN: special (fillto) @(%d, %d) color: %d  fcolor:%d",
						  x, y, data & 0xFF, data >> 8);
			_io.printf("SCREEN: fill to %d,%d  color %d, fill color %d\n",
					x, y, (data & 0xFF) % _dpy->colours(),
					   (data >>8) % _dpy->colours() );
			break;
		}
	regs->y = _dpy->inBounds(x, y) ? 0 : 1;
	}




#pragma mark -- Utility methods

/*****************************************************************************\
|* Utility: error callback
\*****************************************************************************/
int Atari::_cbError(uint32_t address)
	{
	_sim.error("invalid access to cb address $%04x", address);
	return 0;
	}

/*****************************************************************************\
|* Utility: Poke a byte (and create the RAM)
\*****************************************************************************/
void Atari::_poke(uint32_t address, uint8_t val)
	{
	_sim.addRAM(address, &val, 1);
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
	return _sim.getByte(address);
	}

/*****************************************************************************\
|* Utility: Peek at a word
\*****************************************************************************/
uint16_t Atari::_dpeek(uint32_t  address)
	{
	return _sim.getByte(address) + (((int)(_sim.getByte(address + 1))) << 8);
	}

/*****************************************************************************\
|* Utility: fill region with RTS
\*****************************************************************************/
void Atari::_addRtsCallback(uint32_t address, int len, Simulator::SIM_CB cb)
	{
	_sim.addCallback(cb, address, len, Simulator::CB_EXEC);

	uint8_t rts = 0x60;
	for (; len > 0; address++, len--)
		_sim.addROM(address, &rts, 1);
	}
