#include "vcrwidget.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QColor>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
VcrWidget::VcrWidget(QWidget *parent)
		  :QWidget{parent}
		  ,_clicked(-1)
	{
	_icons.push_back(QIcon(":/icon/rsrc/icons/play-back.png"));
	_icons.push_back(QIcon(":/icon/rsrc/icons/step-back.png"));
	_icons.push_back(QIcon(":/icon/rsrc/icons/stop.png"));
	_icons.push_back(QIcon(":/icon/rsrc/icons/step-forward.png"));
	_icons.push_back(QIcon(":/icon/rsrc/icons/play-forward.png"));
	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void VcrWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);

	int x = 5;
	int idx = 0;
	for (QIcon& icon : _icons)
		{
		_drawIcon(painter, icon, x, idx);
		x += 50;
		idx ++;
		}
	}

void VcrWidget::_drawIcon(QPainter &painter, QIcon &icon, int x, int idx)
	{
	QPixmap pixmap = icon.pixmap(QSize(48, 48),
								(idx == _clicked) ? QIcon::Selected
												: QIcon::Normal,
								QIcon::On);
	painter.drawPixmap(x, 0, pixmap);
	}


void VcrWidget::mousePressEvent(QMouseEvent *event)
	{
	_clicked = (event->pos().x() - 5) / 50;
	repaint();
	}

void VcrWidget::mouseReleaseEvent(QMouseEvent *event)
	{
	_clicked = -1;
	repaint();
	}
