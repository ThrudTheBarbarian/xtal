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
	GET(Simulator::MemOpList, ops);			// Memory changes/accesses

	public:
		TraceItem(const QString& text,
				  Simulator::Registers& regs,
				  Simulator::MemOpList ops,
				  QListWidget *parent = nullptr);

	};

#endif // TRACEITEM_H
