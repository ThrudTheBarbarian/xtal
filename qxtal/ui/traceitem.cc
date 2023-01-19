#include "traceitem.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
TraceItem::TraceItem(const QString& text,
					 Simulator::Registers& regs,
					 Simulator::MemOpList ops,
					 QListWidget *parent)
		  :QListWidgetItem(text, parent)
		  ,_regs(regs)
		  ,_ops(ops)
	{
	}

