#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

#include "commands.h"

class Atari;
class Worker : public QThread
	{
	Q_OBJECT

	private:
		QMutex				_sync;			// Synchronisation between threads
		QWaitCondition		_pauseCond;		// Allow thread to wait efficiently
		bool				_active;		// Currently busy
		QVector<Command>	_queue;			// List of things to do
		Atari *				_hw;			// Hardware weak reference

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
		void _playForward(void);

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
		bool schedule(Command cmd);


	};

#endif // WORKER_H
