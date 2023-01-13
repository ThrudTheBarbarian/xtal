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
	typedef std::map<int,TraceItem*> ItemMap;

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);				// Hardware being simulated
	GET(QFont, font);				// Monospaced font
	GET(bool, propagateSelection);	// Whether to send selection messages
	GET(ItemMap, itemMap);			// Map of address to item

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

	public:
		/*********************************************************************\
		|* Constructor
		\*********************************************************************/
		explicit TraceWidget(QWidget *parent = nullptr);

	signals:

	public slots:
		void addTraceItem(const QString& text,
						  Simulator::Registers regs);
	};

#endif // TRACEWIDGET_H
