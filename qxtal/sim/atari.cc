
#include "atari.h"
#include "display.h"
#include "io.h"
#include "simulator.h"


/*****************************************************************************\
|* Atari-specific screen modes
\*****************************************************************************/
static const int _sx[] = {   40,  20,  20,  40,  80,  80,  160, 160,
							 320, 80,  80,  80,  40,  40,  160, 160 };
static const int _sy[] = {   24,  24,  12,  24,  48,  48,  96,  96,
							 192, 192, 192, 192, 24,  12,  192, 192 };
static const int _numc[] = { 256, 256, 256, 4,   2,   4,   2,   4,
							 2,   16,  16,  16,  256, 256, 2,   4 };


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
			// TODO: emulate line fill
			_sim.warn("SCREEN: special (fillto) @(%d, %d) color: %d  fcolor:%d",
						  x, y, data & 0xFF, data >> 8);
			_io.printf("SCREEN: fill to %d,%d  color %d, fill color %d\n",
					x, y, (data & 0xFF) % _dpy->colours(),
					   (data >>8) % _dpy->colours() );
			break;
		}
	regs->y = _dpy->inBounds(x, y) ? 0 : 1;
	}
