#include "asmwidget.h"

#include "notifications.h"
#include "sim/atari.h"

#include <QPainter>
#include <QColor>
#include <QCoreApplication>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
AsmWidget::AsmWidget(QWidget *parent)
	: QWidget{parent}
	{
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_binaryLoaded(nd);}, NTFY_BINARY_LOADED);
	nc->addObserver([=](NotifyData &nd){_simulatorReady(nd);}, NTFY_SIM_AVAILABLE);

	}

/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void AsmWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);
	painter.setBrush(QColor(111, 45, 168, 127));
	painter.fillRect(rect(), painter.brush());
	}


#pragma mark -- Private methods

/*****************************************************************************\
|* A binary was loaded
\*****************************************************************************/
void AsmWidget::_binaryLoaded(NotifyData& nd)
	{
	uint32_t addr	= _org = nd.integerValue();
	uint8_t badMem	= Simulator::MS_INVALID | Simulator::MS_UNDEFINED;

	StringList lines;

	bool done = false;
	while (!done)
		{
		Simulator::InstructionInfo info = _hw->sim()->insnInfo(addr);
		if ((info.state & badMem) == 0)
			{
			_infoList.push_back(info);
			addr += info.bytes;
			}
		else done = true;
		}


	}


/*****************************************************************************\
|* A binary was loaded
\*****************************************************************************/
void AsmWidget::_simulatorReady(NotifyData& nd)
	{
	_hw = static_cast<Atari *>(nd.voidValue());
	}

