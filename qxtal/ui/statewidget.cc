#include "statewidget.h"

#include <QPainter>
#include <QColor>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
StateWidget::StateWidget(QWidget *parent)
	: QWidget{parent}
	{

	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void StateWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);
	painter.setBrush(QColor(0, 255, 0, 127));
	painter.fillRect(rect(), painter.brush());
	}
