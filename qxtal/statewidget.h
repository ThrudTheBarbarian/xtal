#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QObject>
#include <QWidget>

class StateWidget : public QWidget
	{
	Q_OBJECT

	public:
		explicit StateWidget(QWidget *parent = nullptr);


		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);

	};

#endif // STATEWIDGET_H
