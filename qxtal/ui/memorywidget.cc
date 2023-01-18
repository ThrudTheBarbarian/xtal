
#include <QColor>
#include <QPainter>

#include "mainwindow.h"
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
	_reset();

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
	nc->addObserver([=](NotifyData &nd){_reload(nd);}, NTFY_XEX_CHANGED);


	/*************************************************************************\
	|* Set up the heatmap rects
	\*************************************************************************/
	int w = 256;
	int h = 256;
	int x = (330 - w) / 2;
	_hm1Rect = QRect(x, 355, w, h);
	_hm2Rect = QRect(x, 650, w, h);

	_hm1Str = "";
	_hm2Str = "";

	/*************************************************************************\
	|* We want mouse tracking without having to press the button
	\*************************************************************************/
	setMouseTracking(true);

	/*************************************************************************\
	|* Set up for writes to start off with
	\*************************************************************************/
	_counts = _wr;
	_page	= _pgWr;
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
|* Paint a heatmap
\*****************************************************************************/
void MemoryWidget::_heatmap(QPainter &painter,
							uint32_t *data,
							QRect& r,
							int offset)
	{
	int x = r.x();
	int y = r.y();
	int w = r.width();
	int h = r.height();

	painter.setPen(QColor(128,128,128,255));
	painter.drawRect(x,y,w+1,h+1);
	x++;
	y++;

	QPixmap pix(256,256);
	pix.fill();
	QPainter P(&pix);

	const int dx	= 16;
	const int dw	= dx - 1;
	int idx			= offset;

	for (int i=0; i<16; i++)
		{
		for (int j=0; j<16; j++)
			{
			int val = data[idx++];

			if (val > 0)
				{
				if (val <= 4)
					P.fillRect(j*dx,i*dx,dw,dw,_grn[val -1]);

				else if (val < 4 + 16)
					P.fillRect(j*dx,i*dx,dw,dw,_blu[(val-4)/4]);

				else if (val < 4 + 16 + 64)
					P.fillRect(j*dx,i*dx,dw,dw,_red[(val-16-4)/16]);

				else if (val <= 4 + 16 + 64 + 256)
					{
					P.fillRect(j*dx,i*dx,dw,dw,_black);
					int col = (val - 64 - 16 - 4) / 64;
					P.fillRect(j*dx+dw/4+1,i*dx+dw/4+1,dw/2,dw/2,_grn[col]);
					}

				else if (val <= 4 + 16 + 64 + 256 + 1024)
					{
					P.fillRect(j*dx,i*dx,dw,dw,_black);
					int col = (val - 256 - 64 - 16 - 4) / 256;
					P.fillRect(j*dx+dw/4+1,i*dx+dw/4+1,dw/2,dw/2,_blu[col]);
					}

				else if (val <= 4 + 16 + 64 + 256 + 1024 + 4096)
					{
					P.fillRect(j*dx,i*dx,dw,dw,_black);
					int col = (val - 1024 - 256 - 64 - 16 - 4) / 1024;
					P.fillRect(j*dx+dw/4+1,i*dx+dw/4+1,dw/2,dw/2,_red[col]);
					}

				else
					P.fillRect(j*dx,i*dx,dw,dw,_black);
				}
			}
		}

	painter.drawPixmap(x, y, pix);
	}


#pragma mark -- Private methods


/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void MemoryWidget::paintEvent(QPaintEvent *)
	{
	int ox = 30;
	int oy = 70;
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

	/*************************************************************************\
	|* Draw the heatmap rects
	\*************************************************************************/
	_heatmap(painter, _counts, _hm1Rect, _offset);
	painter.drawText(_hm1Rect.x(),
					 _hm1Rect.y() + _hm1Rect.height() + 20,
					 _hm1Str);
	_heatmap(painter, _page, _hm2Rect, 0);
	painter.drawText(_hm2Rect.x(),
					 _hm2Rect.y() + _hm2Rect.height() + 20,
					 _hm2Str);
	}


/*****************************************************************************\
|* Reset the widget
\*****************************************************************************/
void MemoryWidget::_reset(void)
	{
	memset(_written, 0x00, sizeof(_written));
	memset(_last, 0x00, sizeof(_last));

	memset(_wr, 0x00, sizeof(_wr));
	memset(_pc, 0x00, sizeof(_pc));
	memset(_rd, 0x00, sizeof(_rd));

	memset(_pgWr, 0x00, sizeof(_pgWr));
	memset(_pgRd, 0x00, sizeof(_pgRd));
	memset(_pgPc, 0x00, sizeof(_pgPc));
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
			uint16_t addr	= op.address & 0xFFFF;
			uint8_t pgAddr	= addr >> 8;

			switch (op.type)
				{
				case OP_READ:
					_rd[addr] ++;
					_pgRd[pgAddr] ++;
					break;

				case OP_WRITE:
					_wr[addr] ++;
					_pgWr[pgAddr] ++;
					_mem[addr] = op.newVal;
					_written[addr] = 1;
					break;

				case OP_INSN:
					_pc[addr] ++;
					_pgPc[pgAddr] ++;
					break;
				}
			}
	else
		for (MemoryOp& op: ops)
			{
			uint16_t addr	= op.address & 0xFFFF;
			uint8_t pgAddr	= addr >> 8;

			switch (op.type)
				{
				case OP_READ:
					_rd[addr] --;
					_pgRd[pgAddr] --;
					break;

				case OP_WRITE:
					_wr[addr] --;
					_pgWr[pgAddr] --;
					_mem[op.address] = op.oldVal;
					_written[op.address] = 1;
					break;

				case OP_INSN:
					_pc[addr] --;
					_pgPc[pgAddr] --;
					break;
				}
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
void MemoryWidget::_reload(NotifyData &nd)
	{
	_reset();
	}

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



#pragma mark -- Combo box for count-types

/*****************************************************************************\
|* Pulldown menu - The type-of-counts-to-display changed
\*****************************************************************************/
void MemoryWidget::_countTypeChanged(int idx)
	{
	switch (idx)
		{
		case 0:
			_counts = _wr;
			_page   = _pgWr;
			break;

		case 1:
			_counts = _rd;
			_page   = _pgRd;
			break;

		case 2:
			_counts = _pc;
			_page   = _pgPc;
			break;

		default:
			fprintf(stderr, "unknown count-type received: %d\n", idx);
			break;
		}
	repaint();
	}



#pragma mark -- Events


/*****************************************************************************\
|* Event: We got a mouse-move (without needing a button). Check to see if we
|* want to update the memory region text
\*****************************************************************************/
void MemoryWidget:: mouseMoveEvent( QMouseEvent *e )
	{
	int x = e->pos().x();
	int y = e->pos().y();

	if (_hm1Rect.contains(x,y))
		{
		int X		= (x - _hm1Rect.x()) / 16;
		int Y		= (y - _hm1Rect.y()) / 16;
		int idx		= X + Y*16;
		_hm1Str		= QString("Count at $%1 is %2")
						.arg(_offset+idx, 2, 16,QLatin1Char('0'))
						.arg(_counts[_offset+idx]);
		}
	else
		_hm1Str = "";

	if (_hm2Rect.contains(x, y))
		{
		int X		= (x - _hm2Rect.x()) / 16;
		int Y		= (y - _hm2Rect.y()) / 16;
		int idx		= X + Y*16;
		_hm2Str		= QString("Count for page $%1 is %2")
						.arg(idx, 2, 16,QLatin1Char('0'))
						.arg(_page[idx]);

		}
	else
		_hm2Str = "";

	repaint();
	}


/*****************************************************************************\
|* Event: We got a mouse-move (without needing a button). Check to see if we
|* want to update the memory region text
\*****************************************************************************/
void MemoryWidget:: mousePressEvent( QMouseEvent *e )
	{
	int x = e->pos().x();
	int y = e->pos().y();

	if (_hm2Rect.contains(x, y))
		{
		int X		= (x - _hm2Rect.x()) / 16;
		int Y		= (y - _hm2Rect.y()) / 16;
		_offset		= 256 * (X + Y*16);

		QString txt = QString("%1").arg(_offset, 2, 16, QLatin1Char('0'));
		_memStart->setText(txt);
		}
	else
		_hm2Str = "";

	repaint();
	}

