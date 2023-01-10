#ifndef ASMTEXTEDIT_H
#define ASMTEXTEDIT_H

#include <map>

#include <QObject>
#include <QTextEdit>

#include "NotifyCenter.h"
#include "sim/simulator.h"
#include "instructions.h"

class Atari;

class AsmTextEdit : public QTextEdit
	{
	Q_OBJECT

	/*************************************************************************\
	|* Make the instruction collections easier on the fingers
	\*************************************************************************/
	typedef std::vector<Simulator::InstructionInfo> InfoList;
	typedef std::vector<int> LineList;
	typedef std::map<InsnType, String> InsnMap;

	/*************************************************************************\
	|* Hold both the rendered text and a link to the entry in _infoList
	\*************************************************************************/
	typedef struct
		{
		uint32_t	infoIdx;		// Index of data in instruction stream
		} InsnText;

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);				// Hardware being simulated
	GET(uint32_t, org);				// Start of program
	GET(InfoList, infoList);		// Instruction stream
	GET(LineList, lines);			// Look up insn info by text line
	GET(InsnMap, insnMap);			// Map of instruction to mnemonic
	GET(bool, upperCase);			// Use uppercase
	GET(QColor, highlight);			// Colour to highlight a line with

	private:

		/*********************************************************************\
		|* Return the line number for the current cursor
		\*********************************************************************/
		int _lineNumber(void);

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


	public slots:

		/*********************************************************************\
		|* Detect the cursor being changed in any way
		\*********************************************************************/
		void cursorChanged(void);
	};

#endif // ASMTEXTEDIT_H
