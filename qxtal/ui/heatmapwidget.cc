#include "heatmapwidget.h"

#include <QColor>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
HeatMapWidget::HeatMapWidget(QWidget *parent)
			  :QWidget{parent}
	{
	memset(_wr, 0x00, sizeof(_wr));
	memset(_pc, 0x00, sizeof(_pc));
	memset(_rd, 0x00, sizeof(_rd));
	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void HeatMapWidget::paintEvent(QPaintEvent *)
	{
	/*************************************************************************\
	|* Paint the background
	\*************************************************************************/
	QRect bounds = rect();
	bounds.adjust(0,0,-1,-1);

	QPainter painter(this);
	painter.setBrush(QColor(255,255,255,255));
	painter.fillRect(bounds, painter.brush());
	painter.setPen(QColor(0,0,0,255));
	painter.drawRect(bounds);

	_heatmap(painter, _wr, 400);
	_heatmap(painter, _pc, 700);
	}

/*****************************************************************************\
|* Paint a heatmap
\*****************************************************************************/
void HeatMapWidget::_heatmap(QPainter &P, uint32_t *data, int y)
	{
	int w = 256;
	int h = 256;
	int x = (rect().width() - w) / 2;

	P.setPen(QColor(0,0,0,255));
	P.drawRect(x,y,w+2,h+2);
	x++;
	y++;


	}
