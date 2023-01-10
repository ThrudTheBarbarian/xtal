
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QColor>

#include "notifications.h"
#include "vcrwidget.h"
#include "sim/atari.h"
#include "sim/worker.h"

#define ICON_SIZE		(50)
#define ICON_OFF		(QIcon::Disabled)
#define ICON_ACTIVE		(QIcon::Selected)
#define ICON_PRESSED	(QIcon::Normal)

typedef enum
	{
	BTN_PLAY_BACK = 0,
	BTN_STEP_BACK,
	BTN_STOP,
	BTN_STEP_FORWARD,
	BTN_PLAY_FORWARD
	} ButtonAction;

static String _notifications[] =
	{
	NTFY_BTN_PLAY_BACK,
	NTFY_BTN_PLAY_BACK,
	NTFY_BTN_STOP,
	NTFY_BTN_STEP_FORWARD,
	NTFY_BTN_PLAY_FORWARD
	};

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
VcrWidget::VcrWidget(QWidget *parent)
		  :QWidget{parent}
	{
	// These need to be pushed in the order of the enum above
	_icons.push_back(QIcon(":/icon/rsrc/icons/play-back.png"));
	_icons.push_back(QIcon(":/icon/rsrc/icons/step-back.png"));
	_icons.push_back(QIcon(":/icon/rsrc/icons/stop.png"));
	_icons.push_back(QIcon(":/icon/rsrc/icons/step-forward.png"));
	_icons.push_back(QIcon(":/icon/rsrc/icons/play-forward.png"));

	for (int i=0; i<_icons.size(); i++)
		{
		_normal.push_back(ICON_OFF);
		_current.push_back(ICON_OFF);
		}

	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_binaryLoaded(nd);}, NTFY_BINARY_LOADED);
	nc->addObserver([=](NotifyData &nd){_simulatorReady(nd);}, NTFY_SIM_AVAILABLE);
	}


#pragma mark -- Drawing


/*****************************************************************************\
|* Paint the widget
\*****************************************************************************/
void VcrWidget::paintEvent(QPaintEvent *)
	{
	QPainter painter(this);

	_offset = (width() - ICON_SIZE * _icons.size() ) / 2;
	int x = _offset;
	int idx = 0;
	for (QIcon& icon : _icons)
		{
		_drawIcon(painter, icon, x, idx);
		x += ICON_SIZE;
		idx ++;
		}
	}

void VcrWidget::_drawIcon(QPainter &painter, QIcon &icon, int x, int idx)
	{
	QPixmap pixmap = icon.pixmap(QSize(ICON_SIZE-4, ICON_SIZE-4), _current[idx]);
	painter.drawPixmap(x, 0, pixmap);
	}



#pragma mark -- Events


/*****************************************************************************\
|* Mouse pressed, set the state and notify
\*****************************************************************************/
void VcrWidget::mousePressEvent(QMouseEvent *event)
	{
	int which = (event->pos().x() - _offset) / ICON_SIZE;
	if ((which < _icons.size()) && (_normal[which] != ICON_OFF))
		{
		_current[which] = ICON_PRESSED;

		//auto nc = NotifyCenter::defaultNotifyCenter();
		//nc->notify(_notifications[which], which);
		switch (which)
			{
			case BTN_PLAY_BACK:
				_hw->worker()->schedule(CMD_PLAY_BACK);
				break;

			case BTN_STEP_BACK:
				_hw->worker()->schedule(CMD_STEP_BACK);
				break;

			case BTN_STEP_FORWARD:
				_hw->worker()->schedule(CMD_STEP_FORWARD);
				break;

			case BTN_PLAY_FORWARD:
				_hw->worker()->schedule(CMD_PLAY_FORWARD);
				break;

			case BTN_STOP:
				break;
			}

		if (which != BTN_STOP)

		repaint();
		}
	}

/*****************************************************************************\
|* Mouse released, clear the state
\*****************************************************************************/
void VcrWidget::mouseReleaseEvent(QMouseEvent *event)
	{
	for (int i=0; i<_icons.size(); i++)
		_current[i] = _normal[i];

	repaint();
	}


#pragma mark -- Notifications




/*****************************************************************************\
|* A binary was loaded
\*****************************************************************************/
void VcrWidget::_binaryLoaded(NotifyData& nd)
	{
	_normal[BTN_STEP_FORWARD] = ICON_ACTIVE;
	_current[BTN_STEP_FORWARD] = ICON_ACTIVE;

	_normal[BTN_PLAY_FORWARD] = ICON_ACTIVE;
	_current[BTN_PLAY_FORWARD] = ICON_ACTIVE;
	repaint();
	}


/*****************************************************************************\
|* The simulator is ready to run
\*****************************************************************************/
void VcrWidget::_simulatorReady(NotifyData& nd)
	{
	_hw = static_cast<Atari *>(nd.voidValue());
	}

