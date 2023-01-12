#include "traceitem.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
TraceItem::TraceItem(const QString& text,
					 Simulator::Registers regs,
					 QListWidget *parent)
		  :QListWidgetItem(text, parent)
		  ,_regs(regs)
	{
	}

