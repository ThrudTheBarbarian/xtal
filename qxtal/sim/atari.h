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



	public:
		/*********************************************************************\
		|* Constructor
		\*********************************************************************/
		explicit Atari(Simulator& sim, IO& io, QObject *parent = nullptr);


	signals:

	};

#endif // ATARI_H
