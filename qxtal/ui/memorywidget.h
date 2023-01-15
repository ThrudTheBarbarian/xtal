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
