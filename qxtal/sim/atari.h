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

		private:
			/*********************************************************************\
			|* Screen: Handle a screen command
			\*********************************************************************/
			void _screen(ScreenCmd cmd,
						 int x,
						 int y,
						 int data,
						 Simulator::Registers *regs);


			/*********************************************************************\
			|* CIO: exit
			\*********************************************************************/
			static int _cioExit(Simulator::Registers *regs);

			/*********************************************************************\
			|* CIO: error and exit
			\*********************************************************************/
			static int _cioError(Simulator::Registers *regs,
						  const char *err,
						  uint32_t value);

			/*********************************************************************\
			|* CIO: ok and exit
			\*********************************************************************/
			static int _cioOk(Simulator::Registers *regs, uint32_t acc);

			/*********************************************************************\
			|* CIO: call via the device table
			\*********************************************************************/
			static void _cioCallDevTab(Simulator::Registers *regs,
								uint16_t devtab,
								int fn);

			/*********************************************************************\
			|* CIO: Handle CIO
			\*********************************************************************/
			static int _cio(Simulator::Registers *regs, uint32_t address, int data);
			static int _cioErr(Simulator::Registers *regs, uint32_t addr, int data);


			/*********************************************************************\
			|* CIO: Handle Editor
			\*********************************************************************/
			static int _editor(Simulator::Registers *regs, uint32_t addr, int data);



			/*********************************************************************\
			|* Utility: error callback
			\*********************************************************************/
			static int _cbError(uint32_t address);

			/*********************************************************************\
			|* Utility: Poke a byte (and create the RAM)
			\*********************************************************************/
			static void _poke(uint32_t address, uint8_t val);

			/*********************************************************************\
			|* Utility: Poke a word (and create the RAM)
			\*********************************************************************/
			static void _dpoke(uint32_t address, uint32_t val);

			/*********************************************************************\
			|* Utility: Peek at a byte
			\*********************************************************************/
			static uint8_t _peek(uint32_t address);

			/*********************************************************************\
			|* Utility: Peek at a word
			\*********************************************************************/
			static uint16_t _dpeek(uint32_t  address);

			/*********************************************************************\
			|* Utility: fill region with RTS
			\*********************************************************************/
			static void _addRtsCallback(uint32_t address, int len, Simulator::SIM_CB cb);

		public:
			/*********************************************************************\
			|* Constructor
			\*********************************************************************/
			explicit Atari(Simulator* sim, IO* io, QObject *parent = nullptr);

			static void init(Simulator* sim, IO* io);


		signals:

		};

#endif // ATARI_H
