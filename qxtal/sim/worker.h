#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

#include "commands.h"
#include "properties.h"
#include "preferences.h"
#include "sim/simulator.h"

#include "NotifyCenter.h"

class Atari;
class Worker : public QThread
	{
	Q_OBJECT

	/*************************************************************************\
	|* Typedefs
	\*************************************************************************/
	typedef struct
		{
		Command cmd;						// The command to execute
		uint32_t arg;						// Argument for the command
		} WorkItem;



	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GETSET(uint32_t, address, Address);		// Current address

	private:
		QMutex				_sync;			// Synchronisation between threads
		QWaitCondition		_pauseCond;		// Allow thread to wait efficiently
		bool				_active;		// Currently busy
		QVector<WorkItem>	_queue;			// List of things to do
		Atari *				_hw;			// Hardware weak reference
		Preferences			_prefs;			// Current prefs

		/*********************************************************************\
		|* Play backwards
		\*********************************************************************/
		void _playBack(void);

		/*********************************************************************\
		|* Step backwards
		\*********************************************************************/
		void _stepBack(void);

		/*********************************************************************\
		|* Step forwards
		\*********************************************************************/
		void _stepForward(void);

		/*********************************************************************\
		|* Play forwards
		\*********************************************************************/
		void _playForward(uint32_t address);

		/*********************************************************************\
		|* Reset the simulator
		\*********************************************************************/
		void _reset(uint32_t address);

		/*********************************************************************\
		|* Preferences changed
		\*********************************************************************/
		void _prefsChanged(NotifyData& nd);

	protected:
		/*********************************************************************\
		|* Main loop that performs the scheduled tasks in order
		\*********************************************************************/
		virtual void run(void);

	public:
		/*********************************************************************\
		|* Constructor
		\*********************************************************************/
		Worker(Atari *hw);

		/*********************************************************************\
		|* Schedule a task
		\*********************************************************************/
		bool schedule(Command cmd, uint32_t arg);

		/*********************************************************************\
		|* CLear the queue
		\*********************************************************************/
		void stop(void);

	signals:
		void simulationStep(const QString& description,
							Simulator::Registers regs,
							Simulator::MemOpList ops);


		void simulationDone(uint32_t pc);
	};

#endif // WORKER_H
