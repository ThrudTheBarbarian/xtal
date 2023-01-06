#include <algorithm>
#include <cstdint>

#include <QPainter>

#include "display.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
Display::Display(QObject *parent)
		: QObject{parent}
		,_x(0)
		,_y(0)
		,_w(40)
		,_h(24)
		,_colours(256)
		,_region(PAL)
	{
	}


/*****************************************************************************\
|* Make the display
\*****************************************************************************/
void Display::init(void)
	{
	_x = 0;
	_y = 0;
	_display = QImage(_w, _h, QImage::Format_Indexed8);
	_makePalette();
	_display.setColorTable(_rgb);
	}


/*****************************************************************************\
|* Fill the screen
\*****************************************************************************/
void Display::fill(uint8_t colour)
	{
	_display.fill(colour);
	}


/*****************************************************************************\
|* Return the colour index at a given point
\*****************************************************************************/
uint8_t Display::colourAt(int x, int y)
	{
	_x = x;
	_y = y;
	return (uint8_t) _display.pixelIndex(x,y);
	}

/*****************************************************************************\
|* Return whether a point is within the screen bounds
\*****************************************************************************/
bool Display::inBounds(int x, int y)
	{
	return (x >= 0) && (x < _w) && (y >= 0) && (y < _h);
	}

/*****************************************************************************\
|* Plot to the display with a colour-index
\*****************************************************************************/
void Display::plot(int x, int y, uint8_t colour)
	{
	_display.setPixel(x, y, colour);
	_x = x;
	_y = y;
	}

/*****************************************************************************\
|* Draw a line to the display with a colour-index
\*****************************************************************************/
void Display::drawTo(int x, int y, uint8_t colour)
	{
	QPainter p(&_display);
	p.drawLine(_x, _y, x, y);
	_display.setPixel(x, y, colour);
	_x = x;
	_y = y;
	}



#pragma mark -- Private methods


/*****************************************************************************\
|* Make the palette
\*****************************************************************************/
static void _fromNtsc(uint8_t val, uint8_t& R, uint8_t &G, uint8_t &B)
	{
	int cr			= (val >> 4) & 15;
	int lm			= val & 15;
	int crlv		= cr ? 50 : 0;
	double phase	= ((cr-1)*25 - 58) * (2 * M_PI / 360);

	double y = 255*(lm+1)/16;
	double i = crlv*cos(phase);
	double q = crlv*sin(phase);

	double r = y + 0.956*i + 0.621*q;
	double g = y - 0.272*i - 0.647*q;
	double b = y - 1.107*i + 1.704*q;

	R = std::clamp((int)(r * INT8_MAX), INT8_MIN, INT8_MAX);
	G = std::clamp((int)(g * INT8_MAX), INT8_MIN, INT8_MAX);
	B = std::clamp((int)(b * INT8_MAX), INT8_MIN, INT8_MAX);
	}

static void _fromPal(uint8_t val, uint8_t& R, uint8_t &G, uint8_t &B)
	{
	int cr = (val >> 4) & 15;
	int lm = val & 15;
	int crlv = cr ? 50 : 0;

	double phase = ((cr-1)*25.7 - 15) * (2 * M_PI / 360);

	double y = 255*(lm+1)/16;
	double i = crlv*cos(phase);
	double q = crlv*sin(phase);

	double r = y + 0.956*i + 0.621*q;
	double g = y - 0.272*i - 0.647*q;
	double b = y - 1.107*i + 1.704*q;

	R = std::clamp((int)(r * INT8_MAX), INT8_MIN, INT8_MAX);
	G = std::clamp((int)(g * INT8_MAX), INT8_MIN, INT8_MAX);
	B = std::clamp((int)(b * INT8_MAX), INT8_MIN, INT8_MAX);
	}

void Display::_makePalette(void)
	{
	uint8_t R,G,B;

	_rgb.clear();
	for (int i=0; i<256; i++)
		{
		if (_region == PAL)
			_fromPal(i, R, G, B);
		else
			_fromNtsc(i, R, G, B);

		QRgb rgb = qRgb(R, G, B);
		_rgb.push_back(rgb);
		}
	}



//// BUG Flood fill : on clicking the color of pixel is gridColor thus flood fill on it colors only
////that pixel thus after flood fill click on boundary fill to make sure all pixels has white i.e
////boundaryfill color then click floodfill
//void MainWindow::on_floodFill_clicked()
//{
////    int x1=p1.x();
////    int y1=p1.y();
//    int x1 = ui->frame->x;
//    int y1 = ui->frame->y;
//    int k = ui->gridsize->value();

//    x1=(x1/k)*k+k/2;
//    y1=(y1/k)*k+k/2;
//    QRgb current = img.pixel(x1,y1);
//    int r=qRed(MainWindow::fillColor), g=qGreen(MainWindow::fillColor), b=qBlue(MainWindow::fillColor);
//    flood_fill_util(x1,y1,k,current,r,g,b);

//}
//void MainWindow::flood_fill_util(int x1, int y1,int k, QRgb q1, int r,int g,int b)
//{
//    QRgb current = img.pixel(x1,y1);
//    if(x1<=0 || x1>img.width()|| y1<=0 || y1>img.height())
//        return;
//    else if(current!=q1)
//        return;
//    else if(current==qRgb(r,g,b))
//        return;
////    if(current != gridColor)
//        point(x1,y1,r,g,b);

//    flood_fill_util(x1+k,y1,k,q1,r,g,b);
//    flood_fill_util(x1-k,y1,k,q1,r,g,b);
//    flood_fill_util(x1,y1+k,k,q1,r,g,b);
//    flood_fill_util(x1,y1-k,k,q1,r,g,b);
//}


