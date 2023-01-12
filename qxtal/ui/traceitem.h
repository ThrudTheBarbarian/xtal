#ifndef TRACEITEM_H
#define TRACEITEM_H

#include <QListWidgetItem>
#include <QObject>

/*****************************************************************************\
|* Class definition
\*****************************************************************************/
class TraceItem : public QObject, public QListWidgetItem
	{
	Q_OBJECT

	public:
		TraceItem(const QString& text,QListWidget *parent = nullptr);

	};

#endif // TRACEITEM_H
