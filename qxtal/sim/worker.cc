#include <QDebug>

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
bool Worker::schedule(Command cmd)
	{
	_sync.lock();
	bool wasActive = _active;

	/*************************************************************************\
	|* Set the state (active)
	\*************************************************************************/
	_active = true;
	_queue.push_back(cmd);

	/*************************************************************************\
	|* Release the kraken
	\*************************************************************************/
	_pauseCond.wakeAll();

	_sync.unlock();
	return wasActive;
	}

/*****************************************************************************\
|* Protected method: operate on the queue
\*****************************************************************************/
void Worker::run(void)
	{
	forever
		{
		Command cmd = CMD_NONE;

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
			cmd		= _queue.takeLast();
			_active	= true;
			}

		_sync.unlock();


		switch (cmd)
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
				_playForward();
				break;

			default:
				qDebug() << "Unknown command received by worker: " << cmd;
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
void Worker::_playForward(void)
	{
	qDebug() << "Play forward";
	QThread::sleep(2);
	}






