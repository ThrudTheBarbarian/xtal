#ifndef VCRWIDGET_H
#define VCRWIDGET_H

#include <QObject>
#include <QPushButton>
#include <QWidget>

#include "properties.h"

class VcrWidget : public QWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/


	private:
		std::vector<QIcon> _icons;
		int _clicked;

	public:
		explicit VcrWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);
		void _drawIcon(QPainter &painter, QIcon &_icon, int x, int idx);


		/*********************************************************************\
		|* Mouse events
		\*********************************************************************/
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);

	signals:

	};

#endif // VCRWIDGET_H
