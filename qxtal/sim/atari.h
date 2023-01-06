#ifndef ATARI_H
#define ATARI_H

#include <QObject>

#include "simulator.h"

class Display;
class IO;
class Atari : public QObject
	{
	Q_OBJECT


	public:
		/*********************************************************************\
		|* Screen routine callbacks
		\*********************************************************************/
		typedef enum
			{
			SCR_GRAPHICS,
			SCR_LOCATE,
			SCR_PLOT,
			SCR_DRAWTO,
			SCR_FILLTO
			} ScreenCmd;

	protected:
		IO&				_io;			// Input/Output channel
		Simulator&		_sim;			// Simulator engine
		Display *		_dpy;			// Simulated screen

	private:
		/*********************************************************************\
		|* Handle a screen command
		\*********************************************************************/
		void _screen(ScreenCmd cmd,
					 int x,
					 int y,
					 int data,
					 Simulator::Registers *regs);


		/*********************************************************************\
		|* Utility: error callback
		\*********************************************************************/
		int _cbError(uint32_t address);

		/*********************************************************************\
		|* Utility: Poke a byte (and create the RAM)
		\*********************************************************************/
		void _poke(uint32_t address, uint8_t val);

		/*********************************************************************\
		|* Utility: Poke a word (and create the RAM)
		\*********************************************************************/
		void _dpoke(uint32_t address, uint32_t val);

		/*********************************************************************\
		|* Utility: Peek at a byte
		\*********************************************************************/
		uint8_t _peek(uint32_t address);

		/*********************************************************************\
		|* Utility: Peek at a word
		\*********************************************************************/
		uint16_t _dpeek(uint32_t  address);

		/*********************************************************************\
		|* Utility: fill region with RTS
		\*********************************************************************/
		void _addRtsCallback(uint32_t address, int len, Simulator::SIM_CB cb);

	public:
		/*********************************************************************\
		|* Constructor
		\*********************************************************************/
		explicit Atari(Simulator& sim, IO& io, QObject *parent = nullptr);


	signals:

	};

#endif // ATARI_H
