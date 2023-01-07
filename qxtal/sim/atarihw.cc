#include "atarihw.h"
#include "simulator.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
AtariHW::AtariHW(QObject *parent)
	: QObject{parent}
	{}

/*****************************************************************************\
|* Can't execute code in the IO area
\*****************************************************************************/
static Simulator::ErrorCode _execError(Simulator *sim,
									   Simulator::Registers *regs,
									   uint32_t address,
									   int data)
	{
	sim->error("invalid execution address $%04x", address);
	return Simulator::E_EX_UNDEF;
	}

/*****************************************************************************\
|* Provide GTIA simulation
\*****************************************************************************/
static Simulator::ErrorCode _gtia(Simulator *sim,
								  Simulator::Registers *regs,
								  uint32_t address,
								  int data)
	{
	if (data == Simulator::CB_READ)
		{
		switch (address - 0xD000)
			{
			case 0x1f:
				return 7; // CONSOL, no key pressed.

			default:
				sim->warn("GTIA read $%04x", address);
				return 0;
			}
		}
	else
		sim->warn("GTIA write $%04x <- $%02x", address, data);
	return 0;
	}


/*****************************************************************************\
|* Helper function
\*****************************************************************************/
static int rand32(void)
	{
	static uint32_t a, b, c, d, seed = 0;

	if (!seed)
		a = 0xf1ea5eed, b = c = d = seed = 123;

	uint32_t e;
	e = a - ((b << 27) | (b >> 5));
	a = b ^ ((c << 17) | (c >> 15));
	b = c + d;
	c = d + e;
	d = e + a;
	return d;
	}

/*****************************************************************************\
|* Provide POKEY simulation
\*****************************************************************************/
static Simulator::ErrorCode _pokey(Simulator *sim,
								   Simulator::Registers *regs,
								   uint32_t address,
								   int data)
	{
	if (data == Simulator::CB_READ)
		{
		if (address == 0xD20A)
			return 0xFF & rand32();

		sim->warn("POKEY read $%04x", address);
		}
	else
		{
		// Don't log zero writes
		if (data != 0)
			sim->warn("POKEY write $%04x <- $%02x", address, data);
		}
	return 0;
	}


/*****************************************************************************\
|* Provide PIA simulation
\*****************************************************************************/
static Simulator::ErrorCode _pia(Simulator *sim,
								 Simulator::Registers *regs,
								 uint32_t address,
								 int data)
	{
	if (data == Simulator::CB_READ)
		sim->warn("PIA read $%04x", address);
	else
		sim->warn("PIA write $%04x <- $%02x", address, data);
	return 0;
	}


/*****************************************************************************\
|* Provide ANTIC simulation
\*****************************************************************************/
static Simulator::ErrorCode _antic(Simulator *sim,
								   Simulator::Registers *regs,
								   uint32_t address,
								   int data)
	{
	if (data == Simulator::CB_READ)
		sim->warn("ANTIC read $%04x", address);
	else
		sim->warn("ANTIC write $%04x <- $%02x", address, data);
	return 0;
	}


/*****************************************************************************\
|* Initialise the atari hardware
\*****************************************************************************/
void AtariHW::init(Simulator *sim)
	{
	// HW registers
	sim->addCallback(_gtia,		0xD000, 0x100, Simulator::CB_READ);
	sim->addCallback(_pokey,	0xD200, 0x100, Simulator::CB_READ);
	sim->addCallback(_pia,		0xD300, 0x100, Simulator::CB_READ);
	sim->addCallback(_antic,	0xD400, 0x100, Simulator::CB_READ);

	sim->addCallback(_gtia,		0xD000, 0x100, Simulator::CB_WRITE);
	sim->addCallback(_pokey,	0xD200, 0x100, Simulator::CB_WRITE);
	sim->addCallback(_pia,		0xD300, 0x100, Simulator::CB_WRITE);
	sim->addCallback(_antic,	0xD400, 0x100, Simulator::CB_WRITE);

	sim->addCallback(_execError,0xD000, 0x7FF, Simulator::CB_EXEC);
	}
