#include "memorywidget.h"

#include <QPainter>
#include <QColor>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
MemoryWidget::MemoryWidget(QWidget *parent)
			 :QWidget(parent)
	{

	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void MemoryWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);
	painter.setBrush(QColor(255, 0, 0, 127));
	painter.fillRect(rect(), painter.brush());
	}
