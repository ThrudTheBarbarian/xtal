#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QObject>
#include <QLineEdit>
#include <QMouseEvent>
#include <QWidget>

#include <fontmgr.h>

#include "sim/atari.h"
#include "NotifyCenter.h"

class MainWindow;

class MemoryWidget : public QWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);						// Hardware being simulated
	GET(uint16_t, offset);					// Where to display values for
	GET(QLineEdit *, memStart);				// Pointer to mem-start editor

	private:
		uint8_t *_mem;				// Pointer to simulator memory
		uint8_t _written[65536];	// Whether memory was written to
		uint8_t _last[65536];		// Last values of memory
		QFont   _font;				// Font to draw the memory with

		uint32_t _rd[65536];		// Read operation heat-map
		uint32_t _wr[65536];		// Write operation heat-map
		uint32_t _pc[65536];		// PC heat-map
		uint32_t *_counts;			// Pointer to the on-display count

		uint32_t _pgWr[256];		// Page heat-map for writes
		uint32_t _pgRd[256];		// Page heat-map for reads
		uint32_t _pgPc[256];		// Page heat-map for PC
		uint32_t* _page;			// Pointer to the on-display page

		QColor _red[4];				// Red colours
		QColor _grn[4];				// Green colours
		QColor _blu[4];				// Blue colours
		QColor _black;				// Black
		QColor _white;				// White

		QRect	_hm1Rect;			// Box for heatmap 1
		QString _hm1Str;			// String for display

		QRect	_hm2Rect;			// Box for heatmap 2
		QString _hm2Str;			// String for display


		/*********************************************************************\
		|* Paint a heatmap
		\*********************************************************************/
		void _heatmap(QPainter &P, uint32_t *data, QRect&r, int offset);

		/*********************************************************************\
		|* Listen for the simulator to become ready
		\*********************************************************************/
		void _simulatorReady(NotifyData &nd);

		/*********************************************************************\
		|* Notification: a binary was just loaded
		\*********************************************************************/
		void _assemblyComplete(NotifyData &nd);

	protected:

		/*********************************************************************\
		|* Handle mouse moves
		\*********************************************************************/
		void mouseMoveEvent(QMouseEvent *e);

		/*********************************************************************\
		|* Handle mouse clicks
		\*********************************************************************/
		void mousePressEvent(QMouseEvent *e);

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

		/*********************************************************************\
		|* UI : The type-of-counts-to-display changed
		\*********************************************************************/
		void _countTypeChanged(int index);
	};

#endif // MEMORYWIDGET_H
