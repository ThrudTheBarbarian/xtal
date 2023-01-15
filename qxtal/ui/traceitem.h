#ifndef TRACEITEM_H
#define TRACEITEM_H

#include <QListWidgetItem>
#include <QObject>

#include "sim/simulator.h"

/*****************************************************************************\
|* Class definition
\*****************************************************************************/
class TraceItem : public QObject, public QListWidgetItem
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Simulator::Registers, regs);		// Processor state
	GET(MemoryOp, op0);						// First memory op, if needed
	GET(MemoryOp, op1);						// Second memory op, if needed
	GET(MemoryOp, op2);						// Second memory op, if needed

	public:
		TraceItem(const QString& text,
				  Simulator::Registers& regs,
				  MemoryOp &op0,
				  MemoryOp &op1,
				  MemoryOp &op2,
				  QListWidget *parent = nullptr);

	};

#endif // TRACEITEM_H
