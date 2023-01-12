#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QObject>
#include <QListWidget>

#include "sim/atari.h"
#include "NotifyCenter.h"

class TraceWidget : public QListWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);				// Hardware being simulated
	GET(QFont, font);				// Monospaced font

	private:
		/*********************************************************************\
		|* Listen for the simulator to become ready
		\*********************************************************************/
		void _simulatorReady(NotifyData &nd);

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
