#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QObject>
#include <QLineEdit>
#include <QWidget>
#include <fontmgr.h>

#include "sim/atari.h"
#include "NotifyCenter.h"

class MemoryWidget : public QWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);				// Hardware being simulated
	GET(uint16_t, offset);			// Where to display values for
	GET(QLineEdit *, memStart);		// Pointer to mem-start editor

	private:
		uint8_t *_mem;				// Pointer to simulator memory
		uint8_t _written[65536];	// Whether memory was written to
		uint8_t _last[65536];		// Last values of memory
		QFont   _font;				// Font to draw the memory with

		uint32_t _rd[65536];		// Read operation heat-map
		uint32_t _wr[65536];		// Write operation heat-map
		uint32_t _pc[65536];		// PC heat-map
		uint32_t _pg[256];			// Page heat-map

		QColor _red[4];				// Red colours
		QColor _grn[4];				// Green colours
		QColor _blu[4];				// Blue colours
		QColor _black;				// Black
		QColor _white;				// White

		/*********************************************************************\
		|* Paint a heatmap
		\*********************************************************************/
		void _heatmap(QPainter &P, uint32_t *data, int y);

		/*********************************************************************\
		|* Listen for the simulator to become ready
		\*********************************************************************/
		void _simulatorReady(NotifyData &nd);

		/*********************************************************************\
		|* Notification: a binary was just loaded
		\*********************************************************************/
		void _assemblyComplete(NotifyData &nd);

	public:
		MemoryWidget(QWidget *parent = nullptr);

		/*********************************************************************\
		|* Draw the widget
		\*********************************************************************/
		void paintEvent(QPaintEvent *e);

		/*********************************************************************\
		|* Set up / configure the editor
		\*********************************************************************/
		void setMemStartEditor(QLineEdit *editor);


	public slots:
		/*********************************************************************\
		|* Get told to update the current map
		\*********************************************************************/
		void updateState(std::vector<MemoryOp>& ops, bool forwards);

		/*********************************************************************\
		|* User changed the offset
		\*********************************************************************/
		void memStartChanged(const QString& text);

	};

#endif // MEMORYWIDGET_H
