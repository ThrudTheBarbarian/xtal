#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QObject>
#include <QWidget>

#include "sim/simulator.h"
#include "NotifyCenter.h"

class StateWidget : public QWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Simulator::Registers, regs);				// Current state

	private:
		/*********************************************************************\
		|* Notification: Listen for the simulator to become ready
		\*********************************************************************/
		void _stateChanged(NotifyData &nd);

	public:
		explicit StateWidget(QWidget *parent = nullptr);


		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);

	};

#endif // STATEWIDGET_H
