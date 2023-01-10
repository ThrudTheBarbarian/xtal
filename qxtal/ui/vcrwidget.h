#ifndef VCRWIDGET_H
#define VCRWIDGET_H

#include <QObject>
#include <QPushButton>
#include <QWidget>

#include "NotifyCenter.h"
#include "properties.h"

class Atari;

class VcrWidget : public QWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(int, offset);				// Icon offset from left (=0)
	GET(Atari*, hw);				// Hardware being simulated

	private:
		std::vector<QIcon> _icons;
		std::vector<QIcon::Mode> _current;
		std::vector<QIcon::Mode> _normal;

		/*********************************************************************\
		|* Notification: a binary was just loaded
		\*********************************************************************/
		void _binaryLoaded(NotifyData &nd);

		/*********************************************************************\
		|* Notification: the simulator has initialises
		\*********************************************************************/
		void _simulatorReady(NotifyData &nd);

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
