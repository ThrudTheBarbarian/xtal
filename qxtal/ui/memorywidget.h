#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QObject>
#include <QWidget>
#include <fontmgr.h>

#include "sim/atari.h"

class MemoryWidget : public QWidget
	{
	Q_OBJECT

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(Atari*, hw);				// Hardware being simulated
	GET(int, page);					// Page to display values for

	private:
		uint8_t _mem[65536];		// Current values of memory
		uint8_t _written[65536];	// Whether memory was written to
		uint8_t _last[65536];		// Last values of memory
		QFont   _font;				// Font to draw the memory with

	public:
		MemoryWidget(QWidget *parent = nullptr);

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

#endif // MEMORYWIDGET_H
