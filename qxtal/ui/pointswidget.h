#ifndef POINTSWIDGET_H
#define POINTSWIDGET_H

#include <QObject>
#include <QWidget>

class PointsWidget : public QWidget
	{
	Q_OBJECT

	public:
		explicit PointsWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e) override;

	signals:

	};

#endif // POINTSWIDGET_H
