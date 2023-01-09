#include "asmtextedit.h"

#include "notifications.h"
#include "Stringutils.h"
#include "sim/atari.h"

#include <QHeaderView>
#include <QPainter>
#include <QColor>
#include <QCoreApplication>



/*****************************************************************************\
|* Constructor
\*****************************************************************************/
AsmTextEdit::AsmTextEdit(QWidget *parent)
	: QTextEdit{parent}
	{
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_binaryLoaded(nd);}, NTFY_BINARY_LOADED);
	nc->addObserver([=](NotifyData &nd){_simulatorReady(nd);}, NTFY_SIM_AVAILABLE);
	}


#pragma mark -- Private methods

/*****************************************************************************\
|* A binary was loaded
\*****************************************************************************/
void AsmTextEdit::_binaryLoaded(NotifyData& nd)
	{
	uint32_t addr	= _org = nd.integerValue();
	uint8_t badMem	= Simulator::MS_INVALID | Simulator::MS_UNDEFINED;

	StringList lines;

	/*************************************************************************\
	|* Prepare for new data
	\*************************************************************************/
	clear();

	/*************************************************************************\
	|* Fetch the information on each assembly instruction
	\*************************************************************************/
	bool done = false;
	while (!done)
		{
		Simulator::InstructionInfo info = _hw->sim()->insnInfo(addr);
		if ((info.state & badMem) == 0)
			{
			_infoList.push_back(info);
			addr += info.bytes;
			}
		else done = true;
		}

	/*************************************************************************\
	|* Render to something we can display
	\*************************************************************************/
	int idx = 0;
	String txt = "<pre>";
	for (Simulator::InstructionInfo& info : _infoList)
		{
		if (info.label.length() > 0)
			txt += "\n" + info.label + ":\n   ";
		else
			txt += "   ";

		txt += toHexString(info.addr, "$");


		txt += "\n";
		}
	txt += "</pre>";

	setHtml(txt.c_str());
	}


/*****************************************************************************\
|* A binary was loaded
\*****************************************************************************/
void AsmTextEdit::_simulatorReady(NotifyData& nd)
	{
	_hw = static_cast<Atari *>(nd.voidValue());
	}

