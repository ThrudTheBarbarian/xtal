#include "memorywidget.h"

#include <QPainter>
#include <QColor>

#include "notifications.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
MemoryWidget::MemoryWidget(QWidget *parent)
			 :QWidget(parent)
			 ,_page(0)
	{
	memset(_mem, 0x00, sizeof(_mem));
	memset(_written, 0x00, sizeof(_written));
	memset(_last, 0x00, sizeof(_last));
	_font = FontMgr::monospacedFont();
	_font.setPointSize(12);
	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void MemoryWidget::paintEvent(QPaintEvent *)
	{
	int ox = 30;
	int oy = 40;
	int dx = 18;
	int dy = 15;

	/*************************************************************************\
	|* Get the drawing area
	\*************************************************************************/
	QRect bounds = rect();
	bounds.adjust(0,0,-1,-1);

	/*************************************************************************\
	|* Paint the background
	\*************************************************************************/
	QPainter painter(this);
	painter.setBrush(QColor(255,255,255,255));
	painter.fillRect(bounds, painter.brush());
	painter.setPen(QColor(0,0,0,255));
	painter.drawRect(bounds);
	painter.setFont(_font);

	/*************************************************************************\
	|* Draw axes
	\*************************************************************************/
	painter.setPen(QColor(50,50,50,255));
	painter.drawLine(ox-5,oy-dy,ox+16*dx,oy-dy);
	painter.drawLine(ox-3,oy-dy-2,ox-3,oy+dy*15);

	/*************************************************************************\
	|* Draw values for the current page
	\*************************************************************************/
	painter.setPen(QColor(0,0,0,255));
	int offset = _page * 256;
	for (int i=0; i<16; i++)
		{
		int x = ox;
		int y = oy + i * dy;

		QString txt = QString("%1").arg(i*16, 2, 16,QLatin1Char('0'));
		painter.drawText(ox-dx-6, y, txt.toUpper());

		txt = QString("%1").arg(i, 2, 16,QLatin1Char('0'));
		painter.drawText(ox+i*dx, oy-dy-5, txt.toUpper());

		for (int j=0; j<16; j++)
			{
			if (_written[offset] == 0)
				txt = "..";
			else
				txt = QString("%1").arg(_mem[offset], 2, 16,QLatin1Char('0'));

			if (_last[offset] == _mem[offset])
				painter.setPen(QColor(0,0,0,255));
			else
				painter.setPen(QColor(240,0,0,255));
			_last[offset] = _mem[offset];

			painter.drawText(x, y, txt.toUpper());
			x += dx;
			offset ++;
			}
		}
	}


/*****************************************************************************\
|* Handle an update incoming
\*****************************************************************************/
void MemoryWidget::updateState(std::vector<MemoryOp>& ops, bool forwards)
	{
	fprintf(stderr, "Got %ld %s ops\n", ops.size(),
			(forwards ? ("forwards") : ("backwards")));

	if (forwards)
		for (MemoryOp& op : ops)
			{
			_mem[op.address] = op.newVal;
			_written[op.address] = 1;
			}
	else
		for (MemoryOp& op: ops)
			{
			_mem[op.address] = op.oldVal;
			_written[op.address] = 1;
			}

	repaint();
	}


