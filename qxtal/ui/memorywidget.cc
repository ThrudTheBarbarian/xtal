
#include <QColor>
#include <QPainter>

#include "memorywidget.h"
#include "notifications.h"

typedef std::vector<Simulator::InstructionInfo> InfoList;

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
MemoryWidget::MemoryWidget(QWidget *parent)
			 :QWidget(parent)
			 ,_offset(0)
	{
	/*************************************************************************\
	|* Initialise the arrays
	\*************************************************************************/
	memset(_written, 0x00, sizeof(_written));
	memset(_last, 0x00, sizeof(_last));

	memset(_wr, 0x00, sizeof(_wr));
	memset(_pc, 0x00, sizeof(_pc));
	memset(_rd, 0x00, sizeof(_rd));
	memset(_pg, 0x00, sizeof(_pg));

	/*************************************************************************\
	|* Fetch the font
	\*************************************************************************/
	_font = FontMgr::monospacedFont();
	_font.setPointSize(12);

	/*************************************************************************\
	|* Set up the colour-maps
	\*************************************************************************/
	_black = QColor(0,0,0,255);
	_white = QColor(255,255,255,255);
	for (int i=0; i<4; i++)
		{
		int v = 127 + i*32;
		_red[i] = QColor(v,0,0,255);
		_grn[i] = QColor(0,v,0,255);
		_blu[i] = QColor(0,0,v,255);
		}

	/*************************************************************************\
	|* Announce to who cares what the object pointers are
	\*************************************************************************/
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_simulatorReady(nd);}, NTFY_SIM_AVAILABLE);
	nc->addObserver([=](NotifyData &nd){_assemblyComplete(nd);}, NTFY_ASM_DONE);
	}

/*****************************************************************************\
|* Set the editable box
\*****************************************************************************/
void MemoryWidget::setMemStartEditor(QLineEdit *editor)
	{
	_memStart = editor;
	_memStart->setFont(_font);
	_memStart->setText("0000");

	QObject::connect(_memStart, &QLineEdit::textChanged,
					 this,		&MemoryWidget::memStartChanged);
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

	int offset = _offset;

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

	_heatmap(painter, _wr, 340);
	_heatmap(painter, _pg, 650);
	}


/*****************************************************************************\
|* Paint a heatmap
\*****************************************************************************/
void MemoryWidget::_heatmap(QPainter &painter, uint32_t *data, int y)
	{
	int w = 256;
	int h = 256;
	int x = (rect().width() - w) / 2;

	painter.setPen(QColor(128,128,128,255));
	painter.drawRect(x,y,w+1,h+1);
	x++;
	y++;

	QPixmap pix(256,256);
	pix.fill();
	QPainter P(&pix);

	const int dx	= 16;
	const int dw	= dx - 1;
	int idx			= _offset;

	for (int i=0; i<16; i++)
		{
		for (int j=0; j<16; j++)
			{
			int val = data[idx++];

			if (val > 0)
				{
				if (val <= 4)
					P.fillRect(j*dx,i*dx,dw,dw,_grn[val -1]);

				else if (val <= 16)
					P.fillRect(j*dx,i*dx,dw,dw,_blu[val/4 -1]);

				else if (val <= 64)
					P.fillRect(j*dx,i*dx,dw,dw,_red[val/16 -1]);

				else
					{
					P.fillRect(j*dx,i*dx,dw,dw,_black);
					P.fillRect(j*dx+dw/4,i*dx+dw/4,dw/2,dw/2,_white);
					}
				}
			}
		}

	painter.drawPixmap(x, y, pix);
	}



#pragma mark -- slots


/*****************************************************************************\
|* Handle an update incoming
\*****************************************************************************/
void MemoryWidget::updateState(std::vector<MemoryOp>& ops, bool forwards)
	{
	//fprintf(stderr, "Got %ld %s ops\n", ops.size(),
	//		(forwards ? ("forwards") : ("backwards")));

	if (forwards)
		for (MemoryOp& op : ops)
			{
			_mem[op.address] = op.newVal;
			_written[op.address] = 1;
			if (op.isRead)
				_rd[op.address] ++;
			else
				_wr[op.address] ++;
			}
	else
		for (MemoryOp& op: ops)
			{
			_mem[op.address] = op.oldVal;
			_written[op.address] = 1;
			if (op.isRead)
				_rd[op.address] --;
			else
				_wr[op.address] --;
			}

	repaint();
	}

/*****************************************************************************\
|* User changed the offset
\*****************************************************************************/
void MemoryWidget::memStartChanged(const QString& text)
	{
	uint32_t val;
	sscanf(text.toStdString().c_str(), "%x", &val);
	_offset = val & 0xFFFF;
	repaint();
	}





#pragma mark -- notifications


/*****************************************************************************\
|* Notification: Listen for the simulator to become ready
\*****************************************************************************/
void MemoryWidget::_simulatorReady(NotifyData &nd)
	{
	_hw		= static_cast<Atari *>(nd.voidValue());
	_mem	= _hw->sim()->mem();
	}


/*****************************************************************************\
|* Notification: Listen for the simulator to become ready
\*****************************************************************************/
void MemoryWidget::_assemblyComplete(NotifyData &nd)
	{
	InfoList * items = static_cast<InfoList *>(nd.voidValue());
	for (Simulator::InstructionInfo& info : *items)
		for (int i=0; i<info.bytes; i++)
			{
			_written[info.addr+i] = 1;
			_last[info.addr+i] = _mem[info.addr+i];
			}
	}


