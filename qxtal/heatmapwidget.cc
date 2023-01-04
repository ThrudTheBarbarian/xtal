#include "heatmapwidget.h"

#include <QPainter>
#include <QColor>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
HeatMapWidget::HeatMapWidget(QWidget *parent)
			  :QWidget{parent}
	{

	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void HeatMapWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);
	painter.setBrush(QColor(0, 0, 255, 127));
	painter.fillRect(rect(), painter.brush());
	}
