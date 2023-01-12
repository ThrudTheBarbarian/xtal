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

	public:
		TraceItem(const QString& text,
				  Simulator::Registers regs,
				  QListWidget *parent = nullptr);

	};

#endif // TRACEITEM_H
