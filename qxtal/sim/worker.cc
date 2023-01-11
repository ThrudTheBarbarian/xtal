#include <QDebug>

#include "atari.h"
#include "notifications.h"
#include "NotifyCenter.h"
#include "worker.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Worker::Worker(Atari *hw)
	   :QThread()
	   ,_active(false)
	   ,_hw(hw)
	{}

/*****************************************************************************\
|* Public method: schedule a task. Return if we were previously active
\*****************************************************************************/
bool Worker::schedule(Command cmd, uint32_t arg)
	{
	_sync.lock();
	bool wasActive = _active;

	/*************************************************************************\
	|* Set the state (active)
	\*************************************************************************/
	_active = true;

	WorkItem wi;
	wi.arg = arg;
	wi.cmd = cmd;

	_queue.push_back(wi);

	/*************************************************************************\
	|* Release the kraken
	\*************************************************************************/
	_pauseCond.wakeAll();

	_sync.unlock();
	return wasActive;
	}

/*****************************************************************************\
|* Public method: clear the queue stop any in-progress work
\*****************************************************************************/
void Worker::stop(void)
	{
	_sync.lock();
	_queue.clear();
	_sync.unlock();
	requestInterruption();
	}

/*****************************************************************************\
|* Protected method: operate on the queue
\*****************************************************************************/
void Worker::run(void)
	{
	forever
		{
		WorkItem wi = {CMD_NONE,0};

		/*********************************************************************\
		|* See if we're active, and if not, wait on the condition to be woken
		\*********************************************************************/
		_sync.lock();
		if (!_active)
			_pauseCond.wait(&_sync);

		/*********************************************************************\
		|* Check to see if there is a command in the queue, if so, pull it out
		\*********************************************************************/
		if (_queue.empty())
			_active = false;
		else
			{
			wi		= _queue.takeLast();
			_active	= true;
			}

		_sync.unlock();


		switch (wi.cmd)
			{
			case CMD_NONE:
				break;

			case CMD_PLAY_BACK:
				_playBack();
				break;

			case CMD_STEP_BACK:
				_stepBack();
				break;

			case CMD_STOP:
				break;

			case CMD_STEP_FORWARD:
				_stepForward();
				break;

			case CMD_PLAY_FORWARD:
				_playForward(wi.arg);
				break;

			default:
				qDebug() << "Unknown command received by worker: " << wi.cmd;
			}
		}
	}



#pragma mark -- Private methods



/*****************************************************************************\
|* Private method: play backwards
\*****************************************************************************/
void Worker::_playBack(void)
	{
	qDebug() << "Play backwards";
	}

/*****************************************************************************\
|* Private method: step backwards
\*****************************************************************************/
void Worker::_stepBack(void)
	{
	qDebug() << "Step backwards";
	}

/*****************************************************************************\
|* Private method: step forwards
\*****************************************************************************/
void Worker::_stepForward(void)
	{
	qDebug() << "Step forward";
	}


/*****************************************************************************\
|* Private method: play forwards
\*****************************************************************************/
void Worker::_playForward(uint32_t address)
	{
	qDebug() << "Play forward";

	//auto nc = NotifyCenter::defaultNotifyCenter();
	//nc->notify(NTFY_WRK_PLAY_FORWARD);

	Simulator *sim = _hw->sim();

	sim->setError(Simulator::E_NONE, 0);
	sim->regs().pc = _address = address;


	while (!sim->shouldExit())
		{
		if (isInterruptionRequested())
			break;
		sim->next();
		_address = sim->regs().pc;
		fprintf(stderr, "address: $%04x\n", _address);
		}

	_address = sim->regs().pc;
	}






