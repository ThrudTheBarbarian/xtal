#ifndef HEATMAPWIDGET_H
#define HEATMAPWIDGET_H

#include <QObject>
#include <QWidget>

class HeatMapWidget : public QWidget
	{
	Q_OBJECT

	public:
		explicit HeatMapWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);

	};

#endif // HEATMAPWIDGET_H
