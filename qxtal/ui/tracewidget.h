#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QObject>
#include <QWidget>

class TraceWidget : public QWidget
	{
		Q_OBJECT
	public:
		explicit TraceWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e) override;

		/*********************************************************************\
		|* Override the size hint to help out the QScrollArea parent
		\*********************************************************************/
		virtual QSize sizeHint(void) const override;
		virtual QSize minimumSizeHint(void) const override;

	signals:

	};

#endif // TRACEWIDGET_H
