#ifndef ASMTEXTEDIT_H
#define ASMTEXTEDIT_H

#include <QObject>
#include <QTextEdit>

#include "NotifyCenter.h"
#include "sim/simulator.h"

class Atari;

class AsmTextEdit : public QTextEdit
	{
	Q_OBJECT

		/*************************************************************************\
		|* Make the list-of-instructions-stream easier on the fingers
		\*************************************************************************/
		typedef std::vector<Simulator::InstructionInfo> InfoList;

		/*************************************************************************\
		|* Hold both the rendered text and a link to the entry in _infoList
		\*************************************************************************/
		typedef struct
			{
			uint32_t	infoIdx;		// Index of data in instruction stream

			}InsnText;

		/*************************************************************************\
		|* Properties
		\*************************************************************************/
		GET(Atari*, hw);				// Hardware being simulated
		GET(uint32_t, org);				// Start of program
		GET(InfoList, infoList);		// Instruction stream

	private:
		/*********************************************************************\
		|* Notification: a binary was just loaded
		\*********************************************************************/
		void _binaryLoaded(NotifyData &nd);

		/*********************************************************************\
		|* Notification: the simulator has initialises
		\*********************************************************************/
		void _simulatorReady(NotifyData &nd);

	public:
		explicit AsmTextEdit(QWidget *parent = nullptr);


	signals:

	};

#endif // ASMTEXTEDIT_H
