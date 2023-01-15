#include "heatmapwidget.h"

#include <QPixmap>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
HeatMapWidget::HeatMapWidget(QWidget *parent)
			  :QWidget{parent}
	{
	memset(_wr, 0x00, sizeof(_wr));
	memset(_pc, 0x00, sizeof(_pc));
	memset(_rd, 0x00, sizeof(_rd));

	_black = QColor(0,0,0,255);
	_white = QColor(255,255,255,255);
	for (int i=0; i<4; i++)
		{
		int v = 127 + i*32;
		_red[i] = QColor(v,0,0,255);
		_grn[i] = QColor(0,v,0,255);
		_blu[i] = QColor(0,0,v,255);
		}
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

	P.setPen(QColor(128,128,128,255));
	P.drawRect(x,y,w+1,h+1);
	x++;
	y++;

	QImage img(256,256,QImage::Format_RGB32);
	img.fill(_white);

	int idx = 0;
	for (int i=0; i<256; i++)
		{
		for (int j=0; j<256; j++)
			{
			int val = data[idx++];

			if (val > 0)
				{
				if (val <= 4)
					img.setPixelColor(j, i, _grn[val-1]);

				else if (val <= 16)
					img.setPixelColor(j, i, _blu[val/4 -1]);

				else if (val <= 64)
					img.setPixelColor(j, i, _red[val/16 -1]);

				else
					img.setPixelColor(j, i, _black);
				}
			}
		}

	P.drawImage(QPoint(x,y),img);
	}



#pragma mark -- slots


/*****************************************************************************\
|* Handle an update incoming
\*****************************************************************************/
void HeatMapWidget::updateState(std::vector<MemoryOp>& ops, bool forwards)
	{
	if (forwards)
		for (MemoryOp& op : ops)
			{
			if (op.isRead)
				_rd[op.address] ++;
			else
				_wr[op.address] ++;
			}
	else
		for (MemoryOp& op: ops)
			{
			if (op.isRead)
				_rd[op.address] --;
			else
				_wr[op.address] --;
			}

	/*int idx = 0;
	for (int i=0; i<256; i++)
		{
		for (int j=0; j<256; j++)
			printf("%02x", _wr[idx++] & 0xff);
		printf("\n");
		}
*/
	repaint();
	}
