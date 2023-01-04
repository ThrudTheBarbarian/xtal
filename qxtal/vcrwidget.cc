#include "vcrwidget.h"

#include <QPainter>
#include <QColor>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
VcrWidget::VcrWidget(QWidget *parent)
	: QWidget{parent}
	{

	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void VcrWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);
	painter.setBrush(QColor(150, 75, 0, 127));
	painter.fillRect(rect(), painter.brush());
	}
