#ifndef ASMWIDGET_H
#define ASMWIDGET_H

#include <QObject>
#include <QWidget>

class AsmWidget : public QWidget
	{
	Q_OBJECT

	public:
		explicit AsmWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);

	signals:

	};

#endif // ASMWIDGET_H
