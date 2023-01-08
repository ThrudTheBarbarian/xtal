
#include <sys/stat.h>
#include <sys/time.h>

#include "atari.h"
#include "atarihw.h"
#include "display.h"
#include "io.h"
#include "mathpack.h"
#include "simulator.h"

#include "notifications.h"

#include "NotifyCenter.h"
#include "Stringutils.h"

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
|* LOAD/EXEC defs
\*****************************************************************************/
#define RUNADDR		0x2E0		// Binary-load global-run address
#define INITADDR	0x2E2		// Binary-load block-init address

enum
	{
	READ_HDR1	= 0,
	READ_HDR2,
	READ_START1,
	READ_START2,
	READ_END1,
	READ_END2,
	READ_DATA,
	READ_NEXT1,
	READ_NEXT2
	};
typedef int LoadState;

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
static uint8_t hatab_default[] =
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
static uint8_t devhand_tables[] =
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
static uint8_t devhand_emudos[] =
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

#define CB_ARGS	Simulator *sim,				\
	Simulator::Registers *regs,				\
	uint32_t address,							\
	int data

static uint8_t iocv_empty[16] =
	{
	0xFF, 0, 0, 0, // HID, DNO, COM, STA
	0, 0, LO(CIOERR - 1), HI(CIOERR - 1), // BAL, BAH, PTL, PTH
	0, 0, 0, 0, // BLL, BLH, AX1, AX2
	0, 0, 0, 0 // AX3, AX4, AX5, SPR
	};

/*****************************************************************************\
|* RAM locations
\*****************************************************************************/
#define MAX_RAM  (0xD000)       // RAM up to 0xD000 (52k)
#define APP_RAM  (0xC000)       // Usable to applications 0xC000 (48k)
#define LOW_RAM  (0x0700)       // Reserved to the OS up to 0x0700 (1.75k)
#define VID_RAM  (0xC000)       // Video RAM from 0xC000) (4k for video)

/*****************************************************************************\
|* State, filled in by init() or constructor
\*****************************************************************************/
static Atari *			_a8;			// Instance

/*****************************************************************************\
|* Callback declarations
\*****************************************************************************/
Simulator::ErrorCode _cio(CB_ARGS)
	{ return _a8->cioCB(regs, address, data); }

Simulator::ErrorCode _editor(CB_ARGS)
	{ return _a8->editorCB(regs, address, data); }

Simulator::ErrorCode _keyboard(CB_ARGS)
	{ return _a8->keyboardCB(regs, address, data); }

Simulator::ErrorCode _cioErr(CB_ARGS)
	{ return _a8->cioErrCB(regs, address, data); }

Simulator::ErrorCode _printer(CB_ARGS)
	{ return _a8->printerCB(regs, address, data); }

Simulator::ErrorCode _cassette(CB_ARGS)
	{ return _a8->cassetteCB(regs, address, data); }

Simulator::ErrorCode _disk(CB_ARGS)
	{ return _a8->diskCB(regs, address, data); }

Simulator::ErrorCode _screen(CB_ARGS)
	{ return _a8->screenOpenCB(regs, address, data); }

Simulator::ErrorCode _screenOpen(CB_ARGS)
	{ return _a8->screenOpenCB(regs, address, data); }

Simulator::ErrorCode _screenPlot(CB_ARGS)
	{ return _a8->screenPlotCB(regs, address, data); }

Simulator::ErrorCode _screenDraw(CB_ARGS)
	{ return _a8->screenDrawCB(regs, address, data); }

Simulator::ErrorCode _screenLocate(CB_ARGS)
	{ return _a8->screenLocateCB(regs, address, data); }

Simulator::ErrorCode _rtc(CB_ARGS)
	{ return _a8->rtcCB(regs, address, data); }

Simulator::ErrorCode _keyCode(CB_ARGS)
	{ return _a8->keyCodeCB(regs, address, data); }


/*****************************************************************************\
|* Labels for "well known" locations in the ROM
\*****************************************************************************/
static const struct
	{
	const char *lbl;
	unsigned addr;
	} atari_labels[] =
	{
		{ "WARMST",   0x08 }, // WARM START FLAG
		{ "BOOTQ",    0x09 }, // SUCCESSFUL BOOT FLAG
		{ "DOSVEC",   0x0A }, // DISK SOFTWARE START VECTOR
		{ "DOSINI",   0x0C }, // DISK SOFTWARE INIT ADDRESS
		{ "APPMHI",   0x0E }, // APPLICATIONS MEMORY HI LIMIT

		{ "POKMSK",   0x10 }, // SYSTEM MASK FOR POKEY IRQ ENABLE (shadow of IRQEN)
		{ "BRKKEY",   0x11 }, // BREAK KEY FLAG
		{ "RTCLOK",   0x12 }, // REAL TIME CLOCK (IN 16 MSEC UNITS>
		{ "BUFADR",   0x15 }, // INDIRECT BUFFER ADDRESS REGISTER
		{ "ICCOMT",   0x17 }, // COMMAND FOR VECTOR
		{ "DSKFMS",   0x18 }, // DISK FILE MANAGER POINTER
		{ "DSKUTL",   0x1A }, // DISK UTILITIES POINTER
		{ "ABUFPT",   0x1C }, // ##1200xl## 4-byte ACMI buffer pointer area

		{ "IOCBAS",   0x20 }, // 16-byte page zero IOCB
		{ "ICHIDZ",   0x20 }, // HANDLER INDEX NUMBER (FF = IOCB FREE)
		{ "ICDNOZ",   0x21 }, // DEVICE NUMBER (DRIVE NUMBER)
		{ "ICCOMZ",   0x22 }, // COMMAND CODE
		{ "ICSTAZ",   0x23 }, // STATUS OF LAST IOCB ACTION
		{ "ICBALZ",   0x24 }, // BUFFER ADDRESS LOW BYTE
		{ "ICBAHZ",   0x25 }, // 1-byte high buffer address
		{ "ICPTLZ",   0x26 }, // PUT BYTE ROUTINE ADDRESS -1
		{ "ICPTHZ",   0x27 }, // 1-byte high PUT-BYTE routine address
		{ "ICBLLZ",   0x28 }, // BUFFER LENGTH LOW BYTE
		{ "ICBLHZ",   0x29 }, // 1-byte high buffer length
		{ "ICAX1Z",   0x2A }, // AUXILIARY INFORMATION FIRST BYTE
		{ "ICAX2Z",   0x2B }, // 1-byte second auxiliary information
		{ "ICSPRZ",   0x2C }, // 4-byte spares
		{ "ICIDNO",   0x2E }, // IOCB NUMBER X 16
		{ "CIOCHR",   0x2F }, // CHARACTER BYTE FOR CURRENT OPERATION

		{ "CRITIC",   0x42 }, // DEFINES CRITICAL SECTION (CRITICAL IF NON-Z)

		{ "ATRACT",   0x4D }, // ATRACT FLAG
		{ "DRKMSK",   0x4E }, // DARK ATRACT MASK
		{ "COLRSH",   0x4F }, // ATRACT COLOR SHIFTER (EOR'ED WITH PLAYFIELD

		{ "LMARGN",   0x52 }, // left margin (normally 2, cc65 C startup code sets it to 0)
		{ "RMARGN",   0x53 }, // right margin (normally 39 if no XEP80 is used)
		{ "ROWCRS",   0x54 }, // 1-byte cursor row
		{ "COLCRS",   0x55 }, // 2-byte cursor column
		{ "DINDEX",   0x57 }, // 1-byte display mode
		{ "SAVMSC",   0x58 }, // 2-byte saved memory scan counter
		{ "OLDROW",   0x5A }, // 1-byte prior row
		{ "OLDCOL",   0x5B }, // 2-byte prior column
		{ "OLDCHR",   0x5D }, // DATA UNDER CURSOR
		{ "OLDADR",   0x5E }, // 2-byte saved cursor memory address

		{ "RAMTOP",   0x6A }, // RAM SIZE DEFINED BY POWER ON LOGIC

		{ "FR0",      0xD4 }, // 6-byte register 0
		{ "FR0+1",    0xD5 }, // 6-byte register 0
		{ "FR0+2",    0xD6 }, // 6-byte register 0
		{ "FR0+3",    0xD7 }, // 6-byte register 0
		{ "FR0+4",    0xD8 }, // 6-byte register 0
		{ "FR0+5",    0xD9 }, // 6-byte register 0
		{ "FRE",      0xDA }, // 6-byte (internal) register E

		{ "FR1",      0xE0 }, // FP REG1
		{ "FR1+1",    0xE1 }, // FP REG1
		{ "FR1+2",    0xE2 }, // FP REG1
		{ "FR1+3",    0xE3 }, // FP REG1
		{ "FR1+4",    0xE4 }, // FP REG1
		{ "FR1+5",    0xE5 }, // FP REG1

		{ "FR2",      0xE6 }, // 6-byte (internal) register 2

		{ "CIX",      0xF2 }, // CURRENT INPUT INDEX
		{ "INBUFF",   0xF3 }, // POINTS TO USER'S LINE INPUT BUFFER
		{ "INBUFF+1", 0xF4 }, // POINTS TO USER'S LINE INPUT BUFFER

		// Most of the following are not used in the simulator,
		// but included anyway for debugging.

		{ "VDSLST", 0x0200 }, // DISPLAY LIST NMI VECTOR
		{ "VPRCED", 0x0202 }, // PROCEED LINE IRQ VECTOR
		{ "VINTER", 0x0204 }, // INTERRUPT LINE IRQ VECTOR
		{ "VBREAK", 0x0206 }, // SOFTWARE BREAK (00) INSTRUCTION IRQ VECTOR
		{ "VKEYBD", 0x0208 }, // POKEY KEYBOARD IRQ VECTOR
		{ "VSERIN", 0x020A }, // POKEY SERIAL INPUT READY IRQ
		{ "VSEROR", 0x020C }, // POKEY SERIAL OUTPUT READY IRQ
		{ "VSEROC", 0x020E }, // POKEY SERIAL OUTPUT COMPLETE IRQ
		{ "VTIMR1", 0x0210 }, // POKEY TIMER 1 IRQ
		{ "VTIMR2", 0x0212 }, // POKEY TIMER 2 IRQ
		{ "VTIMR4", 0x0214 }, // POKEY TIMER 4 IRQ
		{ "VIMIRQ", 0x0216 }, // IMMEDIATE IRQ VECTOR
		{ "CDTMV1", 0x0218 }, // COUNT DOWN TIMER 1
		{ "CDTMV2", 0x021A }, // COUNT DOWN TIMER 2
		{ "CDTMV3", 0x021C }, // COUNT DOWN TIMER 3
		{ "CDTMV4", 0x021E }, // COUNT DOWN TIMER 4
		{ "CDTMV5", 0x0220 }, // COUNT DOWN TIMER 5
		{ "VVBLKI", 0x0222 }, // IMMEDIATE VERTICAL BLANK NMI VECTOR
		{ "VVBLKD", 0x0224 }, // DEFERRED VERTICAL BLANK NMI VECTOR
		{ "CDTMA1", 0x0226 }, // COUNT DOWN TIMER 1 JSR ADDRESS
		{ "CDTMA2", 0x0228 }, // COUNT DOWN TIMER 2 JSR ADDRESS
		{ "CDTMF3", 0x022A }, // COUNT DOWN TIMER 3 FLAG
		{ "SRTIMR", 0x022B }, // SOFTWARE REPEAT TIMER
		{ "CDTMF4", 0x022C }, // COUNT DOWN TIMER 4 FLAG

		{ "SDMCTL", 0x022F }, // SAVE DMACTL REGISTER
		{ "SDLSTL", 0x0230 }, // SAVE DISPLAY LIST LOW BYTE
		{ "SDLSTH", 0x0231 }, // SAVE DISPLAY LIST HI BYTE
		{ "SSKCTL", 0x0232 }, // SKCTL REGISTER RAM
		{ "LPENH",  0x0234 }, // LIGHT PEN HORIZONTAL VALUE
		{ "LPENV",  0x0235 }, // LIGHT PEN VERTICAL VALUE
		{ "BRKKY",  0x0236 }, // BREAK KEY VECTOR

		{ "PADDL0", 0x0270 }, // 1-byte potentiometer 0
		{ "PADDL1", 0x0271 }, // 1-byte potentiometer 1
		{ "PADDL2", 0x0272 }, // 1-byte potentiometer 2
		{ "PADDL3", 0x0273 }, // 1-byte potentiometer 3
		{ "PADDL4", 0x0274 }, // 1-byte potentiometer 4
		{ "PADDL5", 0x0275 }, // 1-byte potentiometer 5
		{ "PADDL6", 0x0276 }, // 1-byte potentiometer 6
		{ "PADDL7", 0x0277 }, // 1-byte potentiometer 7

		{ "STICK0", 0x0278 }, // 1-byte joystick 0
		{ "STICK1", 0x0279 }, // 1-byte joystick 1
		{ "STICK2", 0x027A }, // 1-byte joystick 2
		{ "STICK3", 0x027B }, // 1-byte joystick 3

		{ "PTRIG0", 0x027C }, // 1-byte paddle trigger 0
		{ "PTRIG1", 0x027D }, // 1-byte paddle trigger 1
		{ "PTRIG2", 0x027E }, // 1-byte paddle trigger 2
		{ "PTRIG3", 0x027F }, // 1-byte paddle trigger 3
		{ "PTRIG4", 0x0280 }, // 1-byte paddle trigger 4
		{ "PTRIG5", 0x0281 }, // 1-byte paddle trigger 5
		{ "PTRIG6", 0x0281 }, // 1-byte paddle trigger 6
		{ "PTRIG7", 0x0283 }, // 1-byte paddle trigger 7

		{ "STRIG0", 0x0284 }, // 1-byte joystick trigger 0
		{ "STRIG1", 0x0285 }, // 1-byte joystick trigger 1
		{ "STRIG2", 0x0286 }, // 1-byte joystick trigger 2
		{ "STRIG3", 0x0287 }, // 1-byte joystick trigger 3

		// Text window
		{ "TXTROW", 0x0290 }, // TEXT ROWCRS
		{ "TXTCOL", 0x0291 }, // TEXT COLCRS
		{ "TINDEX", 0x0293 }, // TEXT INDEX
		{ "TXTMSC", 0x0294 }, // FOOLS CONVRT INTO NEW MSC
		{ "TXTOLD", 0x0296 }, // OLDROW & OLDCOL FOR TEXT (AND THEN SOME)

		// Color registers
		{ "PCOLR0", 0x02C0 }, // 1-byte player-missile 0 color/luminance
		{ "PCOLR1", 0x02C1 }, // 1-byte player-missile 1 color/luminance
		{ "PCOLR2", 0x02C2 }, // 1-byte player-missile 2 color/luminance
		{ "PCOLR3", 0x02C3 }, // 1-byte player-missile 3 color/luminance
		{ "COLOR0", 0x02C4 }, // 1-byte playfield 0 color/luminance
		{ "COLOR1", 0x02C5 }, // 1-byte playfield 1 color/luminance
		{ "COLOR2", 0x02C6 }, // 1-byte playfield 2 color/luminance
		{ "COLOR3", 0x02C7 }, // 1-byte playfield 3 color/luminance
		{ "COLOR4", 0x02C8 }, // 1-byte background color/luminance

		{ "RUNAD",    0x02E0 }, // Binary load RUN ADDRESS
		{ "INITAD",   0x02E2 }, // Binary load INIT ADDRESS
		{ "RAMSIZ",   0x02E4 }, // RAM SIZE (HI BYTE ONLY)
		{ "MEMTOP",   0x02E5 }, // TOP OF AVAILABLE USER MEMORY
		{ "MEMTOP+1", 0x02E6 },
		{ "MEMLO",    0x02E7 }, // BOTTOM OF AVAILABLE USER MEMORY
		{ "MEMLO+1",  0x02E8 },

		{ "CHAR",     0x02FA }, // 1-byte internal character
		{ "ATACHR",   0x02FB }, // ATASCII CHARACTER FOR DRAW/FILL BORDER
		{ "CH",       0x02FC }, // GLOBAL VARIABLE FOR KEYBOARD
		{ "FILDAT",   0x02FD }, // RIGHT FILL COLOR
		{ "DSPFLG",   0x02FE }, // DISPLAY FLAG   DISPLAY CNTLS IF NON-ZERO
		{ "SSFLAG",   0x02FF }, // START/STOP FLAG FOR PAGING (CNTL 1). CLEARE

		{ "HATABS", 0x031A }, // 35-byte handler address table (was 38 bytes)


		{ "IOCB",  0x0340 }, // I/O CONTROL BLOCKS
		{ "ICHID", 0x0340 }, // HANDLER INDEX NUMBER (FF=IOCB FREE)
		{ "ICDNO", 0x0341 }, // DEVICE NUMBER (DRIVE NUMBER)
		{ "ICCOM", 0x0342 }, // COMMAND CODE
		{ "ICSTA", 0x0343 }, // STATUS OF LAST IOCB ACTION
		{ "ICBAL", 0x0344 }, // 1-byte low buffer address
		{ "ICBAH", 0x0345 }, // 1-byte high buffer address
		{ "ICPTL", 0x0346 }, // 1-byte low PUT-BYTE routine address - 1
		{ "ICPTH", 0x0347 }, // 1-byte high PUT-BYTE routine address - 1
		{ "ICBLL", 0x0348 }, // 1-byte low buffer length
		{ "ICBLH", 0x0349 }, // 1-byte high buffer length
		{ "ICAX1", 0x034A }, // 1-byte first auxiliary information
		{ "ICAX2", 0x034B }, // 1-byte second auxiliary information
		{ "ICAX3", 0x034C }, // 1-byte third auxiliary information
		{ "ICAX4", 0x034D }, // 1-byte fourth auxiliary information
		{ "ICAX5", 0x034E }, // 1-byte fifth auxiliary information
		{ "ICSPR", 0x034F }, // SPARE BYTE

		{ "LBPR1", 0x057E }, // LBUFF PREFIX 1
		{ "LBPR2", 0x057F }, // LBUFF PREFIX 2
		{ "LBUFF", 0x0580 }, // 128-byte line buffer

		// Floating Point package
		{ "AFP",    0xD800 }, // convert ASCII to floating point
		{ "FASC",   0xD8E6 }, // convert floating point to ASCII
		{ "IFP",    0xD9AA }, // convert integer to floating point
		{ "FPI",    0xD9D2 }, // convert floating point to integer
		{ "ZFR0",   0xDA44 }, // zero FR0
		{ "ZF1",    0xDA46 }, // zero floating point number
		{ "FSUB",   0xDA60 }, // subtract floating point numbers
		{ "FADD",   0xDA66 }, // add floating point numbers
		{ "FMUL",   0xDADB }, // multiply floating point numbers
		{ "FDIV",   0xDB28 }, // divide floating point numbers
		{ "PLYEVL", 0xDD40 }, // evaluate floating point polynomial
		{ "FLD0R",  0xDD89 }, // load floating point number
		{ "FLD0P",  0xDD8D }, // load floating point number
		{ "FLD1R",  0xDD98 }, // load floating point number
		{ "PLD1P",  0xDD9C }, // load floating point number
		{ "FST0R",  0xDDA7 }, // store floating point number
		{ "FST0P",  0xDDAB }, // store floating point number
		{ "FMOVE",  0xDDB6 }, // move floating point number
		{ "LOG",    0xDECD }, // calculate floating point logarithm
		{ "LOG10",  0xDED1 }, // calculate floating point base 10 logarithm
		{ "EXP",    0xDDC0 }, // calculate floating point exponential
		{ "EXP10",  0xDDCC }, // calculate floating point base 10 exponential

		{ "EDITRV", 0xE400 }, // editor handler vector table
		{ "SCRENV", 0xE410 }, // screen handler vector table
		{ "KEYBDV", 0xE420 }, // keyboard handler vector table
		{ "PRINTV", 0xE430 }, // printer handler vector table
		{ "CASETV", 0xE440 }, // cassette handler vector table

		{ "DISKIV", 0xE450 }, // vector to initialize DIO
		{ "DSKINV", 0xE453 }, // vector to DIO
		{ "CIOV",   0xE456 }, // vector to CIO
		{ "SIOV",   0xE459 }, // vector to SIO
		{ "SETVBV", 0xE45C }, // vector to set VBLANK parameters
		{ "SYSVBV", 0xE45F }, // vector to process immediate VBLANK
		{ "XITVBV", 0xE462 }, // vector to process deferred VBLANK
		{ "SIOINV", 0xE465 }, // vector to initialize SIO
		{ "SENDEV", 0xE468 }, // vector to enable SEND
		{ "INTINV", 0xE46B }, // vector to initialize interrupt handler
		{ "CIOINV", 0xE46E }, // vector to initialize CIO
		{ "BLKBDV", 0xE471 }, // vector to power-up display
		{ "WARMSV", 0xE474 }, // vector to warmstart
		{ "COLDSV", 0xE477 }, // vector to coldstart
		{ "RBLOKV", 0xE47A }, // vector to read cassette block
		{ "CSOPIV", 0xE47D }, // vector to open cassette for input

		{ 0, 0 }
	};



/*****************************************************************************\
|* Constructor - call the static init() method to create the shared instance
\*****************************************************************************/
Atari::Atari(Simulator* sim, IO *io, bool loadLabels)
	  :_io(io)
	  ,_sim(sim)
	  ,_lastRow(0)
	  ,_lastCol(0)
	{
	/*************************************************************************\
	|* Create the display
	\*************************************************************************/
	_dpy = new Display();
	_dpy->init();

	/*************************************************************************\
	|* Adds 52K bytes of RAM at $0
	\*************************************************************************/
	_sim->addRAM(0, MAX_RAM);

	/*************************************************************************\
	|* Initialise the hardware
	\*************************************************************************/
	AtariHW::init(_sim);

	/*************************************************************************\
	|* Adds 32 bytes of zeroed RAM at $80
	\*************************************************************************/
	_sim->addRAM(0x80, 0x20, true);

	/*************************************************************************\
	|* Adds a ROM at 0xE000, to support reads to ROM start
	\*************************************************************************/
	_sim->addROM(0xE000, (unsigned char*)"\x60", 1);

	/*************************************************************************\
	|* CIOV
	\*************************************************************************/
	_addRtsCallback(CIOV, 1, _cio);

	/*************************************************************************\
	|* Init empty CIOV blocks
	\*************************************************************************/
	for (int i = 0; i < 8; i++)
		_sim->addRAM(ICHID + i * 16, iocv_empty, 16);

	/*************************************************************************\
	|* Copy HTAB table
	\*************************************************************************/
	_sim->addRAM(HATABS, hatab_default, sizeof(hatab_default));

	/*************************************************************************\
	|* Copy device handlers table
	\*************************************************************************/
	_sim->addROM(EDITRV, devhand_tables, sizeof(devhand_tables));
	_sim->addROM(DISKDV, devhand_emudos, sizeof(devhand_emudos));

	/*************************************************************************\
	|* Init IOCV 0, editor
	\*************************************************************************/
	_poke(ICHID, EDITOR_OFFSET);
	_poke(ICAX1, 0x0C);
	_dpoke(ICPTL, _dpeek(EDITRV + 6));

	/*************************************************************************\
	|* Install device handlers callbacks
	\*************************************************************************/
	_addRtsCallback(EDITR_BASE, 8, _editor);
	_addRtsCallback(SCREN_BASE, 8, _screen);
	_addRtsCallback(KEYBD_BASE, 8, _keyboard);
	_addRtsCallback(PRINT_BASE, 8, _printer);
	_addRtsCallback(CASET_BASE, 8, _cassette);
	_addRtsCallback(DISKD_BASE, 8, _disk);
	_addRtsCallback(CIOERR, 1, _cioErr);

	/*************************************************************************\
	|* Simulate direct calls to XL OS ROM
	\*************************************************************************/
	_addRtsCallback(0xEF9C, 1, _screenOpen);
	_addRtsCallback(0xF18F, 1, _screenLocate);
	_addRtsCallback(0xF1D8, 1, _screenPlot);
	_addRtsCallback(0xF9C2, 1, _screenDraw);

	/*************************************************************************\
	|* Math Package
	\*************************************************************************/
	MathPack::load(_sim);

	/*************************************************************************\
	|* Simulate keyboard character "CH"
	\*************************************************************************/
	_sim->addCallback(_keyCode, 0x2FC, 1, Simulator::CB_READ);
	_sim->addCallback(_keyCode, 0x2FC, 1, Simulator::CB_WRITE);

	/*************************************************************************\
	|* Simulate real-time clock
	\*************************************************************************/
	_sim->addCallback(_rtc, 0x12, 3, Simulator::CB_READ);
	_sim->addCallback(_rtc, 0x12, 3, Simulator::CB_WRITE);

	/*************************************************************************\
	|* Random OS addresses
	\*************************************************************************/
	_poke(8, 0);				// WARM START
	_poke(17, 0x80);			// BREAK key not pressed
	_dpoke(14, 0x800);			// APPHI, lowest usable RAM area
	_dpoke(0x2e5, APP_RAM);		// MEMTOP
	_dpoke(0x2e7, LOW_RAM);		// MEMLO
	_poke(0x2FC, 0xFF);			// CH
	_poke(0x2F2, 0xFF);			// CH1
	_poke(LMARGN, 2);			// LMARGIN
	_poke(RMARGN, 39);			// RMARGIN
	_poke(ROWCRS, 0);			// ROWCRS
	_dpoke(COLCRS, 2);			// COLCRS
	_poke(0x57, 0);				// DINDEX
	_dpoke(0x58, VID_RAM);		// Simulated screen pointer
	_poke(0x5D, 0);				// OLDCH
	_dpoke(0x5E, VID_RAM);		// Store an invalid value in OLDADR, to catch
								// programs writing to the screen directly.
	_poke(0x6A, APP_RAM/256);	// RAMTOP
	_poke(0x2be, 64);			// SHFLOK
	_dpoke(10, 0xFFFF);			// DOSVEC, go to DOS vector,
								// use $FFFF as simulation return
	}

/*****************************************************************************\
|* Create a shared, initialised instance
\*****************************************************************************/
Atari * Atari::instance(Simulator* sim, IO *io, bool loadLabels)
	{
	if (_a8 == nullptr)
		_a8 = new Atari(sim, io, loadLabels);
	return _a8;
	}

/*****************************************************************************\
|* Load a binary file. This (slightly ab)uses the Atari binary load format to
|* optionally provide embedded symbol tables as well as binary data.
|*
|* Atari binary format is:
|*
|* $FF $FF		- First two bytes indicate it's a binary file
|* $lo $lo		- Next two bytes are the start address to load data at
|* $hi $hi		- Next two bytes are the last address to fill with data
|*
|* There then follows sufficient data to fill $hi - $lo + 1 bytes (note the
|* +1, this is an *inclusive* range)
|*
|* A series of these blocks can be catenated together, and will load data in
|* sequence to different ranges of addresses
|*
|* If a block covers $02E0,$02E1, then it is considered the execution address
|* for the entire file once loaded, and the computer will JSR to that location
|* once the file is completely loaded.
|*
|* If a block covers $02E2,$02E3, then it is considered a mid-load call, and
|* once the block has finished loading, the computer will immediately JSR to
|* the address in $02E2,$02E3.
|*
|* For both of these locations, the routine should end with RTS
|*
|*
|* qxtal slightly abuses this to provide an embedded symbol table. The
|* idea is that the assembler will create a block (or multiple ones if the
|* block length is longer than the code) at the start of the binary
|* file which is just data, loaded at the same offset as the actual binary
|* code (so it will be over-written by the code). That said, if the code
|* is < 256 bytes and the symbol table is more than 256 bytes, then the
|* block-size for the symbol table will be 256 bytes
|*
|* The symbol-table data within each block follows the form:
|*
|* $60			- the hex code for RTS, in case this is ever executed (!)
|* $53			- the hex code for 'S'
|* $59			- the hex code for 'Y'
|* $4D			- the hex code for 'M'
|*
|* Then a series of chunks:
|*
|*  $lo, $hi	- two bytes to represent the address
|*  $len		- one byte to represent how long the string is
|*  <len bytes>	- the string that forms the label name
|*  ...         - repeated until all the symbols are defined
|*
|* Followed by two more bytes which are a checksum of the entire block apart
|* from the checksum itself. The checksum is simply the sum of all bytes in
|* the block, modulo 16 bits.
|*
|* $lo, $hi		- checksum
|*
|* If the symbol table is ever left in the binary, it ought to be loaded and
|* discarded, then overwritten (finally) by the binary, and the binary can
|* still be executed.
|*
|* qxtal however, can load and parse the symbols, and use them in the UI.
\*****************************************************************************/

Simulator::ErrorCode Atari::load(const String& filename)
	{
	Simulator::ErrorCode e = Simulator::E_NONE;	// What to return

	LoadState state = READ_HDR1;	// State-machine for block-loading
	uint16_t sAddr = 0;				// Address of memory to load first byte
	uint16_t eAddr = 0;				// Address of memory to load last byte
	uint16_t runAddr = 0;			// Address to call into

	uint16_t vec;					// General purpose address vector

	std::vector<uint8_t> chunk;		// Actual data within block

	/*************************************************************************\
	|* Open the file and begin reading
	\*************************************************************************/
	FILE *fp = fopen(filename.c_str(), "rb");
	if (fp == NULL)
		e = Simulator::E_USER;
	else
		{
		/*********************************************************************\
		|* Initialise both vectors to 0
		\*********************************************************************/
		_dpoke(RUNADDR, 0);
		_dpoke(INITADDR, 0);

		while (e == Simulator::E_NONE)
			{
			uint8_t data;
			int c = fgetc(fp);
			if (c == EOF)
				{
				vec = _dpeek(RUNADDR);
				if (vec != 0)
					{
					_sim->warn("call RUN vector at $%04x", vec);
					runAddr = vec;
					}
				else
					{
					_sim->warn("call XEX load vector at $%04x", sAddr);
					runAddr = sAddr;
					}

				auto nc = NotifyCenter::defaultNotifyCenter();
				nc->notify(NTFY_BINARY_LOADED, runAddr);
				//e = _sim->call(runAddr);
				break;
				}

			/*****************************************************************\
			|* Run through the state machine, loading the block
			\*****************************************************************/
			switch (state)
				{
				case READ_HDR1:
				case READ_HDR2:
					if (c != 0xFF)
						e = Simulator::E_USER;
					break;

				case READ_START1:
					sAddr = c;
					break;

				case READ_START2:
					sAddr |= (c<<8);
					vec = sAddr;
					break;

				case READ_END1:
					eAddr = c;
					break;

				case READ_END2:
					eAddr |= (c<<8);
					chunk.clear();
					break;

				case READ_DATA:
					chunk.push_back(c & 0xFF);
					if (vec != eAddr)
						{
						vec ++;
						continue;
						}

					/*********************************************************\
					|* We've loaded the memory, check to see if it's a symbol
					|* table, and if so parse, otherwise load to RAM
					\*********************************************************/
					if (!_isSymbolTable(chunk))
						{
						_sim->addRAM(sAddr,chunk.data(), chunk.size());
						_sim->warn("Loading block at $%04x", sAddr);
						}

					/*********************************************************\
					|* Since we're at the end of a data section, check the init
					|* address and see if we should call it
					\*********************************************************/
					vec = _dpeek(INITADDR);
					if (vec != 0)
						{
						_sim->warn("call INIT vector at $%04x", vec);
						e = _sim->call(vec);
						_dpoke(INITADDR, 0);
						}
					break;

				case READ_NEXT1:
					sAddr = c;
					break;

				case READ_NEXT2:
					sAddr |= (c<<8);
					if (sAddr == 0xFFFF)
						state = READ_START1;
					else
						state = READ_END1;
					continue;
				}

			state ++;
			}

		fclose(fp);
		}

	return e;
	}


/*****************************************************************************\
|* Check if a block is a symbol table, and if so, load it
\*****************************************************************************/
bool Atari::_isSymbolTable(const std::vector<uint8_t> &data)
	{
	bool isSymTab = false;

	/*************************************************************************\
	|* First check the signature bytes
	\*************************************************************************/
	if (data.size() > 5)	// Signature + checksum
		{
		Simulator::AddressMap labels;

		uint16_t cksum = 0x60 + 'S' + 'Y' + 'M';

		const uint8_t *bytes = data.data();
		int idx = 4;
		if ((bytes[0] == 0x60) /*RTS*/ &&
			(bytes[1] == 'S') &&
			(bytes[2] == 'Y') &&
			(bytes[3] == 'M'))
			{
			while (idx + 3 < data.size() - 2)
				{
				int addr = bytes[idx+1];
				addr	 = (addr << 8) + bytes[idx];
				int size = bytes[idx+2];
				cksum   += bytes[idx] + bytes[idx+1] + bytes[idx+2];

				String label = "";
				for (int i=0; i<size; i++)
					{
					label += bytes[idx+3+i];
					cksum += bytes[idx+3+i];
					}
				labels[addr] = label;
				idx += size + 3;
				}
			}

		/*********************************************************************\
		|* If the checksum matches, then we store the labels
		\*********************************************************************/
		uint16_t blocksum	= bytes[idx+1];
		blocksum			= (blocksum << 8) | bytes[idx];
		if (blocksum == cksum)
			{
			isSymTab = true;
			for (Elements<uint32_t, String> kv : labels)
				_sim->addLabel(kv.key, kv.value);
			_sim->warn("Parsing, found %d symbols", labels.size());
			}
		}
	return isSymTab;
	}


#pragma mark -- screen handling

/*****************************************************************************\
|* Handle a screen command
\*****************************************************************************/
void Atari::_screenOp(ScreenCmd cmd,
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

/*****************************************************************************\
|* Open the screen
\*****************************************************************************/
int Atari::screenOpenCB(Simulator::Registers *regs, uint32_t  addr, int data)
	{
	_screenOp(SCR_GRAPHICS, 0, 0, regs->a, regs);
	return 0;
	}

/*****************************************************************************\
|* Plot to the screen
\*****************************************************************************/
int Atari::screenPlotCB(Simulator::Registers *regs, uint32_t  addr, int data)
	{
	_screenOp(SCR_PLOT, _dpeek(COLCRS), _peek(ROWCRS), _peek(ATACHR), regs);
	return 0;
	}

/*****************************************************************************\
|* Draw or fill to the screen
\*****************************************************************************/
int Atari::screenDrawCB(Simulator::Registers *regs, uint32_t  addr, int data)
	{
	_screenOp(_peek(FILFLG) ? SCR_FILLTO : SCR_DRAWTO,
			  _dpeek(COLCRS),
			  _peek(ROWCRS),
			  _peek(ATACHR) | (_peek(FILDAT)<<8),
			  regs);
	return 0;
	}

/*****************************************************************************\
|* Move to position on the screen
\*****************************************************************************/
int Atari::screenLocateCB(Simulator::Registers *regs, uint32_t  addr, int data)
	{
	_screenOp(SCR_LOCATE, _dpeek(COLCRS), _peek(ROWCRS), 0, regs);
	return 0;
	}

/*****************************************************************************\
|* Move to position on the screen
\*****************************************************************************/
int Atari::screenCB(Simulator::Registers *regs, uint32_t  addr, int data)
	{
	unsigned cmd  = GET_IC(COM);
	switch (addr & 7)
	{
		case DEVR_OPEN:
			regs->a = (GET_IC(AX2) & 0x0F) | (GET_IC(AX1) & 0xF0);
			return screenOpenCB(regs, addr, data);

		case DEVR_CLOSE:
			_sim->warn("SCREEN: close");
			return 0; // OK

		case DEVR_GET:
			return screenLocateCB(regs, addr, data);

		case DEVR_PUT:
			_poke(ATACHR, regs->a);
			return screenPlotCB(regs, addr, data);

		case DEVR_STATUS:
			_sim->warn("SCREEN: cmd STATUS");
			return 0;

		case DEVR_SPECIAL:
			if (cmd == 17 || cmd == 18)
				{
				_poke(FILFLG, (cmd == 18));
				return screenDrawCB(regs, addr, data);
				}
			else
				_sim->warn("SCREEN: special (unknown=%d)", cmd);
			return 0;

		case DEVR_INIT:
			return 0;

		default:
			return _cbError(addr);
		}
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
int Atari::cioCB(Simulator::Registers *regs, uint32_t address, int data)
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
int Atari::cioErrCB(Simulator::Registers *regs, uint32_t addr, int data)
	{
	_sim->warn("CIO error, IOCB not open");
	regs->y = 0x83;
	_sim->setFlags(Simulator::FLAG_N, Simulator::FLAG_N);
	return 0;
	}



#pragma mark -- Editor

int Atari::editorCB(Simulator::Registers *regs, uint32_t addr, int data)
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
			if (row != _lastRow || col != _lastCol)
				{
				if (row != _lastRow)
					_io->putchar(0x9B);

				_sim->warn("EDITOR position from (%d,%d) to (%d,%d)",
							  _lastRow, _lastCol, row, col);
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
			_lastRow = row;
			_lastCol = col;
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


/*****************************************************************************\
|* Editor (ish): keyboard
\*****************************************************************************/
int Atari::keyboardCB(Simulator::Registers *regs, uint32_t addr, int data)
	{
	switch (addr & 7)
		{
		case DEVR_OPEN:
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
			regs->y = 135;
			return 0; // Nothing

		case DEVR_STATUS:
			return 0;

		case DEVR_SPECIAL:
			return 0; // Nothing

		case DEVR_INIT:
			return 0;

		default:
			return _cbError(addr);
		}
	}





#pragma mark -- Peripheral i/o

/*****************************************************************************\
|* PIO: Printer
\*****************************************************************************/
int Atari::printerCB(Simulator::Registers *regs, uint32_t addr, int data)
	{
	switch (addr & 7)
		{
		case DEVR_OPEN:
			return 0;

		case DEVR_CLOSE:
			return 0;

		case DEVR_GET:
			return 0; // Nothing

		case DEVR_PUT:
			return 0;

		case DEVR_STATUS:
			return 0;

		case DEVR_SPECIAL:
			return 0; // Nothing

		case DEVR_INIT:
			return 0;

		default:
			return _cbError(addr);
		}
	}


/*****************************************************************************\
|* PIO : cassette
\*****************************************************************************/
int Atari::cassetteCB(Simulator::Registers *regs, uint32_t addr, int data)
	{
	switch (addr & 7)
		{
		case DEVR_OPEN:
			return 0;

		case DEVR_CLOSE:
			return 0;

		case DEVR_GET:
			return 0; // Nothing

		case DEVR_PUT:
			return 0;

		case DEVR_STATUS:
			return 0;

		case DEVR_SPECIAL:
			return 0; // Nothing

		case DEVR_INIT:
			return 0;

		default:
			return _cbError(addr);
		}
	}

/*****************************************************************************\
|* PIO: Disk filesystem
\*****************************************************************************/
FILE * Atari::_fOpen(const char *name, const char *mode)
	{
	// Easy, check if file already exists
	struct stat st;
	if (0 == stat(name, &st))
		return fopen(name, mode);

	// Not, check if file with lower case exists:
	String lcName = lcase(name);
	const char *lname = lcName.c_str();

	if (0 == stat(lname, &st))
		return fopen(lname, mode);

	return nullptr;
	}

/*****************************************************************************\
|* PIO: Disk
\*****************************************************************************/
int Atari::diskCB(Simulator::Registers *regs, uint32_t addr, int data)
	{
	static FILE *fhand[16];		// One per CIO channel

	// We need IOCB data
	unsigned chn  = ((regs->x) >> 4);
	unsigned badr = GET_IC(BAL) | (GET_IC(BAH) << 8);
	unsigned dno  = GET_IC(DNO);
	unsigned ax1  = GET_IC(AX1);
	unsigned ax2  = GET_IC(AX2);

	switch (addr & 7)
		{
		case DEVR_OPEN:
			{
			// Decode file name
			char fname[256];
			int i;

			// Skip 'D#:'
			badr++;
			if (dno == (_peek(badr) - '0'))
				badr++;

			if (_peek(badr) == ':')
				badr++;

			// Translate rest of filename
			for (i = 0; i < 250; i++)
				{
				int c = _peek(badr++);
				if (!c || c == 0x9b)
					break;
				fname[i] = c;
				}

			fname[i] = 0;
			_sim->warn("DISK OPEN #%d, %d, %d, '%s'", chn, ax1, ax2, fname);

			// Test if not already open
			if (fhand[chn])
				{
				_sim->warn("DISK: Internal error, %d already open.", chn);
				fclose(fhand[chn]);
				fhand[chn] = 0;
				}

			// Open Flag:
			const char *flags = nullptr;
			switch (ax1)
				{
				case 4: // Open for read
					flags = "rb";
					break;

				case 8: // Open for write
					flags = "wb";
					break;

				case 9: // Open for append
					flags = "ab";
					break;

				case 12: // Open for update
					flags = "r+b";
					break;

				case 6:
					// TODO: Directory read

				default:
					regs->y = 0xA8;
					return 0;
				}

			fhand[chn] = _fOpen(fname, flags);
			if (!fhand[chn])
				{
				_sim->warn("DISK OPEN: error %s", strerror(errno));
				if (errno == ENOENT)
					regs->y = 170;
				else if (errno == ENOSPC)
					regs->y = 162;
				else if (errno == EACCES)
					regs->y = 167;
				else
					regs->y = 139;
				}
			else
				regs->y = 1;
			return 0;
			}

		case DEVR_CLOSE:
			if (fhand[chn])
				{
				fclose(fhand[chn]);
				fhand[chn] = 0;
				}
			regs->y = 1;
			return 0;

		case DEVR_GET:
			if (!fhand[chn])
				{
				_sim->warn("DISK GET: Internal error, %d closed.", chn);
				regs->y = 133;
				}
			else
				{
				int c   = fgetc(fhand[chn]);
				regs->y = 1;
				if (c == EOF)
					regs->y = 136;
				else
					regs->a = c;
				}
			return 0;

		case DEVR_PUT:
			if (!fhand[chn])
				{
				_sim->warn("DISK PUT: Internal error, %d closed.", chn);
				regs->y = 133;
				}
			else
				{
				fputc(regs->a, fhand[chn]);
				regs->y = 1;
				}
			return 0;

		case DEVR_STATUS:
			return 0;

		case DEVR_SPECIAL:
			return 0; // Nothing

		case DEVR_INIT:
			return 0;

		default:
			return _cbError(addr);
		}
	}





#pragma mark -- system devices

/*****************************************************************************\
|* System: real-time clock
\*****************************************************************************/
int Atari::rtcCB(Simulator::Registers *regs, uint32_t addr, int data)
	{
	static int startTime;

	// Get current time
	struct timeval tv;
	gettimeofday(&tv, 0);
	int curTime   = (int)(fmod(tv.tv_sec * 60 + tv.tv_usec * 0.00006, 16777216.));
	int atariTime = curTime - startTime;

	if (data == Simulator::CB_READ)
		{
		if (addr == 0x12)
			return 0xFF & (atariTime >> 16);
		else if (addr == 0x13)
			return 0xFF & (atariTime >> 8);
		else
			return 0xFF & atariTime;
		}
	else
		{
		if (addr == 0x12)
			atariTime = (atariTime & 0x00FFFF) | (data << 16);
		else if (addr == 0x13)
			atariTime = (atariTime & 0xFF00FF) | (data << 8);
		else
			atariTime = (atariTime & 0xFFFF00) | data;
		startTime = curTime - atariTime;
		}
	return 0;
	}



/*****************************************************************************\
|* System: keyboard scan
\*****************************************************************************/
int Atari::keyCodeCB(Simulator::Registers *regs, uint32_t addr, int data)
	{
	static int ch = 0xFF;
	static uint8_t kcodes[128] = {
// ;    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
0xA0,0xBF,0x95,0x92,0xBA,0xAA,0xB8,0xBD,0xB9,0x8d,0x81,0x85,0x80,0xA5,0xA3,0x88,
// P    Q    R    S    T    U    V    W    X    Y    Z  ESC    ^    v   <-   ->    _
0x8A,0xAF,0xA8,0xBE,0xAD,0x8B,0x90,0xAE,0x96,0xAB,0x97,0x1C,0x8E,0x8F,0x86,0x87,
//      !    "    #    $    %    &    '    (    )    *    +    ,    -    .    /
0x21,0x5F,0x5E,0x5A,0x58,0x5D,0x5B,0x73,0x70,0x72,0x07,0x06,0x20,0x0E,0x22,0x26,
// 0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ?
0x32,0x1F,0x1E,0x1A,0x18,0x1D,0x1B,0x33,0x35,0x30,0x42,0x02,0x36,0x0F,0x37,0x66,
// @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
0x75,0x3F,0x15,0x12,0x3A,0x2A,0x38,0x3D,0x39,0x0D,0x01,0x05,0x00,0x25,0x23,0x08,
// P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _
0x0A,0x2F,0x28,0x3E,0x2D,0x0B,0x10,0x2E,0x16,0x2B,0x17,0x60,0x46,0x64,0x47,0x4E,
// `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o
0xA2,0x7F,0x55,0x52,0x7A,0x6A,0x78,0x7D,0x79,0x4D,0x41,0x45,0x40,0x65,0x63,0x48,
// p    q    r    s    t    u    v    w    x    y    z  C-;    |  CLS   BS  TAB
0x4A,0x6F,0x68,0x7E,0x6D,0x4B,0x50,0x6E,0x56,0x6B,0x57,0x82,0x4F,0x76,0x34,0x2C
	};

	if (data == Simulator::CB_READ)
		{
		// Return value if we have one
		if (ch != 0xFF)
			return ch;

		// Else, see if we have a character available
		int c = _io->peekchar();
		if (c == EOF)
			return 0xFF;
		else
			{
			// Translate to key-code
			if (c == 0x9B)
				ch = 0x0C;
			else
				ch = kcodes[c & 0x7F];
			}
		return ch;
		}
	else
		{
		// Simply write over our internal value
		ch = data;
		}
	return 0;
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
uint16_t Atari::_peek(uint32_t address)
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
