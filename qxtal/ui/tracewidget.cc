#include <QFontDatabase>

#include "memorywidget.h"
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
			,_previousRow(0)
	{
	_font = FontMgr::monospacedFont();

	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_simulatorReady(nd);}, NTFY_SIM_AVAILABLE);
	nc->addObserver([=](NotifyData &nd){_asmSelectionChanged(nd);}, NTFY_ASM_SEL_CHG);
	nc->addObserver([=](NotifyData &nd){_prepareToSimulate(nd);}, NTFY_SIM_START);
	nc->addObserver([=](NotifyData &nd){_reload(nd);}, NTFY_XEX_CHANGED);

	QObject::connect(this, &TraceWidget::currentItemChanged,
					 this, &TraceWidget::_handleSelectionChanged);

	}

/*****************************************************************************\
|* Public slot - add an item
\*****************************************************************************/
void TraceWidget::addTraceItem(const QString& text,
							   Simulator::Registers regs,
							   Simulator::MemOpList ops)
	{
	TraceItem *item = new TraceItem(" "+text, regs, ops);
	//fprintf(stderr, "$%04x: %d %d\n", regs.pc, op0.isValid, op1.isValid);

	item->setData(Qt::FontRole, _font);
	_itemMap[regs.pc].push_back(item);
	addItem(item);

	_lastItem = item;
	}



/*****************************************************************************\
|* Public slot - simulation is complete
\*****************************************************************************/
void TraceWidget::simulationDone(uint32_t address)
	{
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->notify(NTFY_SIM_DONE, (int)address);

	setCurrentItem(_lastItem);
	}

#pragma mark -- Private Methods



/*****************************************************************************\
|* Private method: Clear any current selections
\*****************************************************************************/
void TraceWidget::_clearCurrentSelection(void)
	{
	for (TraceItem * item : _selected)
		item->setSelected(false);
	_selected.clear();
	}


#pragma mark -- Notifications


/*****************************************************************************\
|* Notification: Listen for the simulator to become ready
\*****************************************************************************/
void TraceWidget::_reload(NotifyData &nd)
	{
	_prepareToSimulate(nd);
	}



/*****************************************************************************\
|* Notification: Listen for the simulator to become ready
\*****************************************************************************/
void TraceWidget::_simulatorReady(NotifyData &nd)
	{
	_hw = static_cast<Atari *>(nd.voidValue());
	QObject::connect(_hw->worker(), &Worker::simulationStep,
					 this, &TraceWidget::addTraceItem);
	QObject::connect(_hw->worker(), &Worker::simulationDone,
					 this, &TraceWidget::simulationDone);
	}


/*****************************************************************************\
|* Notification: Selection changed in the Assembly widget
\*****************************************************************************/
void TraceWidget::_asmSelectionChanged(NotifyData &nd)
	{
	int address = nd.integerValue();
	if (_itemMap.find(address) != _itemMap.end())
		{
		/*********************************************************************\
		|* Clear any previous selection, do not propagate the selection-change
		|* to the AsmWidget (because the user didn't get here by selecting
		|* something in this widget), and allow for multiple selection
		\*********************************************************************/
		_clearCurrentSelection();
		_propagateSelection = false;
		setSelectionMode(QAbstractItemView::MultiSelection);

		/*********************************************************************\
		|* For each call to the address, set it to be selected so it can be
		|* seen
		\*********************************************************************/
		for (TraceItem * item : _itemMap[address])
			{
			item->setSelected(true);
			_selected.push_back(item);
			}


		/*********************************************************************\
		|* For now, zoom in on the first-selected item. Should probably check
		|* for any of the just-selected items being visible and not move if
		|* so. In fact: FIXME: do that
		\*********************************************************************/
		if (_selected.size())
			{
			scrollToItem(_selected[0], QAbstractItemView::EnsureVisible);
			setCurrentItem(_selected[0], QItemSelectionModel::Current);
			}
		}
	}


/*****************************************************************************\
|* The simulator is about to run
\*****************************************************************************/
void TraceWidget::_prepareToSimulate(NotifyData& nd)
	{
	clear();
	_selected.clear();
	_itemMap.clear();
	_previousRow = 0;
	}


#pragma mark -- Events




/*****************************************************************************\
|* Signal handler: our selection changed
\*****************************************************************************/
void TraceWidget::_handleSelectionChanged(QListWidgetItem *current,
										  QListWidgetItem *previous)
	{
	if (_propagateSelection)
		{
		/*********************************************************************\
		|* Clear any previous selection, and set single selection
		\*********************************************************************/
		_clearCurrentSelection();
		TraceItem *theItem = static_cast<TraceItem *>(current);
		setSelectionMode(QAbstractItemView::SingleSelection);

		/*********************************************************************\
		|* Add this to the selection list so it will be cleared later
		\*********************************************************************/
		_selected.push_back(theItem);

		/*********************************************************************\
		|* Tell the world that we have a selection
		\*********************************************************************/
		auto nc = NotifyCenter::defaultNotifyCenter();
		nc->notify(NTFY_TRACE_SEL_CHG, theItem);
		}
	else
		_propagateSelection = true;

	/*********************************************************************\
	|* Figure out the memory differences between the previous selection
	|* and the current one
	\*********************************************************************/
	Simulator::MemOpList ops;
	int thisRow		= currentRow();
	bool forwards	= true;

	if (_previousRow < thisRow)
		for (int i=_previousRow; i<thisRow; i++)
			{
			TraceItem *ti = static_cast<TraceItem *>(item(i));
			if (ti != nullptr)
				for (MemoryOp op : ti->ops())
					ops.push_back(op);
			}

	else if (_previousRow > thisRow)
		{
		for (int i=_previousRow-1; i>=thisRow; i--)
			{
			TraceItem *ti = static_cast<TraceItem *>(item(i));
			if (ti != nullptr)
				for (MemoryOp op : ti->ops())
					ops.push_back(op);
			}
		forwards = false;
		}

	if (ops.size() > 0)
		emit updateMemory(ops, forwards);
	_previousRow = thisRow;
	}
