#include "fontmgr.h"
#include "notifications.h"
#include "statewidget.h"
#include "StringUtils.h"
#include "traceitem.h"

#include <QPainter>
#include <QColor>

/*****************************************************************************\
|* Helpers
\*****************************************************************************/
template<typename T>
static std::string toBinaryString(const T& x)
	{
	std::stringstream ss;
	ss << std::bitset<sizeof(T) * 8>(x);
	return ss.str();
	}


/*****************************************************************************\
|* Constructor
\*****************************************************************************/
StateWidget::StateWidget(QWidget *parent)
	: QWidget{parent}
	{
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_stateChanged(nd);}, NTFY_TRACE_SEL_CHG);

	_regs = {0,0,0,0,0,0,0};
	}

/*****************************************************************************\
|* Paint the widget
|*
|* PCPCPC  	FlagsFlags
|* PCPCPC  	SP:sp
|*
|* A:aa X:xx Y:yy
\*****************************************************************************/
void StateWidget::paintEvent(QPaintEvent *)
	{
	int x = 8;
	int y = 30;

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

	/*************************************************************************\
	|* Show PC
	\*************************************************************************/
	QFont mono = FontMgr::monospacedFont();
	mono.setPointSize(20);

	painter.setFont(mono);
	String txt = toHexString(_regs.pc,"PC:$", 4);
	painter.drawText(x, y, QString::fromStdString(txt));

	/*************************************************************************\
	|* Show A,X,Y,SP
	\*************************************************************************/
	painter.drawText(x, y+35, QString::fromStdString(toHexString(_regs.a,"A:$")));
	painter.drawText(x+80, y+35, QString::fromStdString(toHexString(_regs.x,"X:$")));
	painter.drawText(x+160, y+35, QString::fromStdString(toHexString(_regs.y,"Y:$")));
	painter.drawText(x+240, y+35, QString::fromStdString(toHexString(_regs.s,"SP:$")));

	/*************************************************************************\
	|* Show flags
	\*************************************************************************/
	painter.drawText(x+155, y, "Flags:");
	mono.setPointSize(14);
	painter.setFont(mono);
	painter.drawText(x+245, y-10, "NV-BDIZC");
	painter.drawText(x+245, y+6, QString::fromStdString(toBinaryString(_regs.p)));

	}


/*****************************************************************************\
|* Our state changed
\*****************************************************************************/
void StateWidget::_stateChanged(NotifyData &nd)
	{
	TraceItem *item = static_cast<TraceItem *>(nd.voidValue());
	_regs = item->regs();
	repaint();
	}
