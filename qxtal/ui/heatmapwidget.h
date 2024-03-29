#ifndef HEATMAPWIDGET_H
#define HEATMAPWIDGET_H

#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QColor>

#include "sim/atari.h"

class HeatMapWidget : public QWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);				// Hardware being simulated

	private:
		uint32_t _rd[65536];		// Read operation heat-map
		uint32_t _wr[65536];		// Write operation heat-map
		uint32_t _pc[65536];		// PC heat-map

		QColor _red[8];				// Red colours
		QColor _grn[8];				// Green colours
		QColor _blu[8];				// Blue colours
		QColor _black;				// Black
		QColor _white;				// White

		/*********************************************************************\
		|* Paint a heatmap
		\*********************************************************************/
		void _heatmap(QPainter &P, uint32_t *data, int y);

	public:
		explicit HeatMapWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);


	public slots:
		/*********************************************************************\
		|* Get told to update the current map
		\*********************************************************************/
		void updateState(std::vector<MemoryOp>& ops, bool forwards);
	};

#endif // HEATMAPWIDGET_H
