#include "asmwidget.h"

#include "notifications.h"
#include "sim/simulator.h"

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
	uint32_t addr = nd.integerValue();

	StringList lines;
	std::vector<Simulator::InstructionInfo> info;
/*
	bool done = false;
	while (!done)
		{
		Simulator::InstructionInfo info =
		}
*/

	}


/*****************************************************************************\
|* A binary was loaded
\*****************************************************************************/
void AsmWidget::_simulatorReady(NotifyData& nd)
	{
	_hw = static_cast<Atari *>(nd.voidValue());
	}

