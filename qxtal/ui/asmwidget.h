#ifndef ASMWIDGET_H
#define ASMWIDGET_H

#include <QObject>
#include <QWidget>

#include "NotifyCenter.h"

class Atari;

class AsmWidget : public QWidget
	{
	Q_OBJECT
		/*************************************************************************\
		|* Properties
		\*************************************************************************/
		GET(Atari*, hw);				// Hardweare being simulated

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
		explicit AsmWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);

	signals:

	};

#endif // ASMWIDGET_H
