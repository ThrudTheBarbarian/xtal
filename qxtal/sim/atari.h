#ifndef ATARI_H
#define ATARI_H

#include <QObject>

#include "properties.h"
#include "simulator.h"

#define A8_CB_ARGS Simulator::Registers *regs, uint32_t address, int data

class Display;
class IO;
class Worker;

class Atari
	{
	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GETSET(IO*, io, Io);				// Input/Output channel
	GETSET(Simulator*, sim, Sim);		// Simulator engine
	GETSET(Display *, dpy, Dpy);		// Simulated screen
	GET(Worker*, worker);				// Worker thread

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
		uint32_t		_lastRow;		// Editor's last row
		uint32_t		_lastCol;		// Editor's last col

	private:
		/*********************************************************************\
		|* Screen: Actual implementation of screen commands
		\*********************************************************************/
		void _screenOp(ScreenCmd cmd,
					   int x,
					   int y,
					   int data,
					   Simulator::Registers *regs);




		/*********************************************************************\
		|* CIO: exit
		\*********************************************************************/
		int _cioExit(Simulator::Registers *regs);

		/*********************************************************************\
		|* CIO: error and exit
		\*********************************************************************/
		int _cioError(Simulator::Registers *regs,
					  const char *err,
					  uint32_t value);

		/*********************************************************************\
		|* CIO: ok and exit
		\*********************************************************************/
		int _cioOk(Simulator::Registers *regs, uint32_t acc);

		/*********************************************************************\
		|* CIO: call via the device table
		\*********************************************************************/
		void _cioCallDevTab(Simulator::Registers *regs,
							uint16_t devtab,
							int fn);


		/*********************************************************************\
		|* PIO : Open a file on disk
		\*********************************************************************/
		FILE *_fOpen(const char *name, const char *mode);


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
		uint16_t _peek(uint32_t address);

		/*********************************************************************\
		|* Utility: Peek at a word
		\*********************************************************************/
		uint16_t _dpeek(uint32_t  address);

		/*********************************************************************\
		|* Utility: fill region with RTS
		\*********************************************************************/
		void _addRtsCallback(uint32_t address, int len, Simulator::SIM_CB cb);

		/*********************************************************************\
		|* Determine if the block is actually a symbol table, if so, load it
		\*********************************************************************/
		bool _isSymbolTable(const std::vector<uint8_t> &data);

		/*********************************************************************\
		|* Constructor
		\*********************************************************************/
		Atari(Simulator* sim, IO* io, bool loadLabels);

	public:
		/*********************************************************************\
		|* Accessor. You have to supply the parameters on the first call
		\*********************************************************************/
		static Atari * instance(Simulator* sim = nullptr,
								IO* io = nullptr,
								bool loadLabels = true);


		/*********************************************************************\
		|* Load a binary
		\*********************************************************************/
		Simulator::ErrorCode load(const String& filename);

		/*********************************************************************\
		|* Callbacks: Handle CIO
		\*********************************************************************/
		Simulator::ErrorCode cioCB(A8_CB_ARGS);
		Simulator::ErrorCode cioErrCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: Handle Editor
		\*********************************************************************/
		Simulator::ErrorCode editorCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: Handle Keyboard
		\*********************************************************************/
		Simulator::ErrorCode keyboardCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback:  Handle Printer
		\*********************************************************************/
		Simulator::ErrorCode printerCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: Handle Cassette
		\*********************************************************************/
		Simulator::ErrorCode cassetteCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: Handle Disk
		\*********************************************************************/
		Simulator::ErrorCode diskCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: handler for driver
		\*********************************************************************/
		Simulator::ErrorCode screenCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: Open the screen
		\*********************************************************************/
		Simulator::ErrorCode screenOpenCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: Plot to the screen
		\*********************************************************************/
		Simulator::ErrorCode screenPlotCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: Draw or fill to the screen
		\*********************************************************************/
		Simulator::ErrorCode screenDrawCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback: Move to position on the screen
		\*********************************************************************/
		Simulator::ErrorCode screenLocateCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback:  Real-time clock
		\*********************************************************************/
		Simulator::ErrorCode rtcCB(A8_CB_ARGS);

		/*********************************************************************\
		|* Callback:  keyboard scan
		\*********************************************************************/
		Simulator::ErrorCode keyCodeCB(A8_CB_ARGS);
	};


#undef A8_CB_ARGS
#endif // ATARI_H
