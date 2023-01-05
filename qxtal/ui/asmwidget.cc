#include "asmwidget.h"


#include <QPainter>
#include <QColor>

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
AsmWidget::AsmWidget(QWidget *parent)
	: QWidget{parent}
	{

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
