#ifndef COMMANDS_H
#define COMMANDS_H

/*****************************************************************************\
|* These are commands that can be sent to the worker thread for it to perform
|* a single one of the below until completion or interruption, on a background
|* thread
\*****************************************************************************/
typedef enum
	{
	CMD_NONE		= -1,
	CMD_PLAY_BACK	= 0,
	CMD_STEP_BACK,
	CMD_STOP,
	CMD_STEP_FORWARD,
	CMD_PLAY_FORWARD,
	CMD_RESET
	} Command;

#endif // COMMANDS_H
