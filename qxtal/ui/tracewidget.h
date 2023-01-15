#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QObject>
#include <QListWidget>

#include "sim/atari.h"
#include "NotifyCenter.h"

class TraceItem;
class TraceWidget : public QListWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Make the instruction collections easier on the fingers
	\*************************************************************************/
	typedef std::map<int,std::vector<TraceItem*>> ItemMap;
	typedef std::vector<TraceItem *> SelectionList;

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);				// Hardware being simulated
	GET(QFont, font);				// Monospaced font
	GET(bool, propagateSelection);	// Whether to send selection messages
	GET(ItemMap, itemMap);			// Map of address to item
	GET(SelectionList, selected);	// List of currently selected items
	GET(int, previousRow);			// Previously selected row

	private:
		/*********************************************************************\
		|* Listen for the simulator to become ready
		\*********************************************************************/
		void _simulatorReady(NotifyData &nd);

		/*********************************************************************\
		|* A selection was made in the assembly widget
		\*********************************************************************/
		void _asmSelectionChanged(NotifyData &nd);

		/*********************************************************************\
		|* Handle selection
		\*********************************************************************/
		void _handleSelectionChanged(QListWidgetItem *current,
									 QListWidgetItem *previous);

		/*********************************************************************\
		|* Clear the current selection before selecting any more
		\*********************************************************************/
		void _clearCurrentSelection(void);

	public:
		/*********************************************************************\
		|* Constructor
		\*********************************************************************/
		explicit TraceWidget(QWidget *parent = nullptr);

	signals:
		void updateMemory(std::vector<MemoryOp>& ops,
						  bool forwards);

	public slots:
		void addTraceItem(const QString& text,
						  Simulator::Registers regs,
						  MemoryOp op0,
						  MemoryOp op1,
						  MemoryOp op2);
	};

#endif // TRACEWIDGET_H
