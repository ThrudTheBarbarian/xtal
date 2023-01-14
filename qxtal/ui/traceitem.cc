#include "traceitem.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
TraceItem::TraceItem(const QString& text,
					 Simulator::Registers& regs,
					 MemoryOp& op0,
					 MemoryOp& op1,
					 QListWidget *parent)
		  :QListWidgetItem(text, parent)
		  ,_regs(regs)
		  ,_op0(op0)
		  ,_op1(op1)
	{
	}

