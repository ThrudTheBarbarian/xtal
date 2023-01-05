#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QObject>
#include <QWidget>

class MemoryWidget : public QWidget
	{
	Q_OBJECT

	public:
		MemoryWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);
	};

#endif // MEMORYWIDGET_H
