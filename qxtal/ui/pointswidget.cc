#include "pointswidget.h"

#include <QColor>
#include <QPainter>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
PointsWidget::PointsWidget(QWidget *parent)
	: QWidget{parent}
	{
	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void PointsWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);
	painter.setBrush(QColor(0, 0, 0, 127));
	painter.fillRect(rect(), painter.brush());
	}
