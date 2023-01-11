#include "tracewidget.h"

#include <QColor>
#include <QPainter>

TraceWidget::TraceWidget(QWidget *parent)
	: QWidget{parent}
	{
	}


/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void TraceWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);
	painter.setBrush(QColor(0, 255, 255, 127));
	painter.fillRect(rect(), painter.brush());
	}

/*****************************************************************************\
|* Override the size hint to help out the QScrollArea parent
\*****************************************************************************/
QSize TraceWidget::sizeHint(void) const
	{
	return QSize(width(), 3000);
	}
QSize TraceWidget::minimumSizeHint(void) const
	{
	return QSize(width(), 1000);
	}
