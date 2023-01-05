#ifndef VCRWIDGET_H
#define VCRWIDGET_H

#include <QObject>
#include <QWidget>

class VcrWidget : public QWidget
	{
	Q_OBJECT

	public:
		explicit VcrWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);

	signals:

	};

#endif // VCRWIDGET_H
