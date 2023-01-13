#include <QFontDatabase>

#include "tracewidget.h"
#include "traceitem.h"

#include "notifications.h"
#include "sim/worker.h"
#include "ui/fontmgr.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
TraceWidget::TraceWidget(QWidget *parent)
			:QListWidget{parent}
			,_propagateSelection(true)
	{
	_font = FontMgr::monospacedFont();

	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_simulatorReady(nd);}, NTFY_SIM_AVAILABLE);
	nc->addObserver([=](NotifyData &nd){_asmSelectionChanged(nd);}, NTFY_ASM_SEL_CHG);

	QObject::connect(this, &TraceWidget::currentItemChanged,
					 this, &TraceWidget::_handleSelectionChanged);
	}

/*****************************************************************************\
|* Public slot - add an item
\*****************************************************************************/
void TraceWidget::addTraceItem(const QString& text, Simulator::Registers regs)
	{
	TraceItem *item = new TraceItem("  "+text, regs);
	item->setData(Qt::FontRole, _font);
	_itemMap[regs.pc] = item;

	addItem(item);
	}



/*****************************************************************************\
|* Notification: Listen for the simulator to become ready
\*****************************************************************************/
void TraceWidget::_simulatorReady(NotifyData &nd)
	{
	_hw = static_cast<Atari *>(nd.voidValue());
	QObject::connect(_hw->worker(), &Worker::simulationStep,
					 this, &TraceWidget::addTraceItem);
	}


/*****************************************************************************\
|* Notification: Listen for the simulator to become ready
\*****************************************************************************/
void TraceWidget::_asmSelectionChanged(NotifyData &nd)
	{
	int address = nd.integerValue();
	if (_itemMap.find(address) != _itemMap.end())
		{
		_propagateSelection = false;
		setCurrentItem(_itemMap[address]);
		}
	}

/*****************************************************************************\
|* Signal handler: our selection changed
\*****************************************************************************/
void TraceWidget::_handleSelectionChanged(QListWidgetItem *current,
										  QListWidgetItem *previous)
	{
	if (_propagateSelection)
		{
		TraceItem *item = static_cast<TraceItem *>(current);

		auto nc = NotifyCenter::defaultNotifyCenter();
		nc->notify(NTFY_TRACE_SEL_CHG, item);

		fprintf(stderr, "trce: $%04x\n", item->regs().pc);
		}
	else
		_propagateSelection = true;
	}
