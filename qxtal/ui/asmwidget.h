#ifndef ASMWIDGET_H
#define ASMWIDGET_H

#include <QListWidget>
#include <QObject>

#include "sim/atari.h"
#include "NotifyCenter.h"

class AsmItem;
class AsmWidget : public QListWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Make the instruction collections easier on the fingers
	\*************************************************************************/
	typedef std::vector<Simulator::InstructionInfo> InfoList;
	typedef std::vector<int> LineList;
	typedef std::map<int,AsmItem*> ItemMap;
	typedef std::map<InsnType, String> InsnMap;

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);				// Hardware being simulated
	GET(QFont, font);				// Monospaced font
	GET(uint32_t, org);				// Start of program
	GET(InfoList, infoList);		// Instruction stream
	GET(LineList, lines);			// Look up insn info by text line
	GET(InsnMap, insnMap);			// Map of instruction to mnemonic
	GET(ItemMap, itemMap);			// Map of address to item
	GET(bool, upperCase);			// Use uppercase
	GET(bool, propagateSelection);	// Whether to send selection messages

	private:
		/*********************************************************************\
		|* Notification: Listen for the simulator to become ready
		\*********************************************************************/
		void _simulatorReady(NotifyData &nd);

		/*********************************************************************\
		|* Notification: a binary was just loaded
		\*********************************************************************/
		void _binaryLoaded(NotifyData &nd);

		/*********************************************************************\
		|* Notification: a selection was made in the trace window
		\*********************************************************************/
		void _traceSelection(NotifyData &nd);

		/*********************************************************************\
		|* PRivate method: Toggle breakpoint at an instruction
		\*********************************************************************/
		void _toggleBreakpoint(AsmItem *item, int x);


		/*********************************************************************\
		|* Handle selection
		\*********************************************************************/
		void _handleSelectionChanged(QListWidgetItem *current,
									 QListWidgetItem *previous);


		QIcon _blank;
		QIcon _redDot;

	protected:
		/*********************************************************************\
		|* Event: check if we want to toggle a breakpoint
		\*********************************************************************/
		void mousePressEvent(QMouseEvent *event);

	public:
		/*********************************************************************\
		|* Constructor
		\*********************************************************************/
		explicit AsmWidget(QWidget *parent = nullptr);

	public slots:
	};

#endif // ASMWIDGET_H
