#ifndef DISPLAY_H
#define DISPLAY_H

#include <QObject>
#include <QImage>
#include "properties.h"

class Display : public QObject
	{
	Q_OBJECT

	typedef enum
		{
		NTSC,
		PAL
		} Region;

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GETSET(int, x, X);					// Current X cursor
	GETSET(int, y, Y)					// Current Y cursor
	GETSET(int, w, W);					// Screen width
	GETSET(int, h, H)					// Screen height
	GETSET(int, colours, Colours);		// Number of screen colours
	GET(QImage, display);				// The actual image
	GETSET(Region, region, Region);		// Which region we're emulating

	private:
		QVector<QRgb> _rgb;				// The screen palette


		/*********************************************************************\
		|* Create the screen palette
		\*********************************************************************/
		virtual void _makePalette(void);

	public:
		/*********************************************************************\
		|* Constructor
		\*********************************************************************/
		explicit Display(QObject *parent = nullptr);

		/*********************************************************************\
		|* Create the screen
		\*********************************************************************/
		virtual void init(void);

		/*********************************************************************\
		|* Fill the display with a colour-index
		\*********************************************************************/
		virtual void fill(uint8_t colour);

		/*********************************************************************\
		|* Plot to the display with a colour-index
		\*********************************************************************/
		virtual void plot(int x, int y, uint8_t colour);

		/*********************************************************************\
		|* Draw to the display with a colour-index
		\*********************************************************************/
		virtual void drawTo(int x, int y, uint8_t colour);

		/*********************************************************************\
		|* Return the colour index at a given point
		\*********************************************************************/
		virtual uint8_t colourAt(int x, int y);

		/*********************************************************************\
		|* Return whether a point is within the screen bounds
		\*********************************************************************/
		virtual bool inBounds(int x, int y);

	signals:

	};

#endif // DISPLAY_H
