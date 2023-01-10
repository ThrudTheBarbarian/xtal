#include "asmtextedit.h"

#include "notifications.h"
#include "Stringutils.h"
#include "sim/atari.h"

#include <QHeaderView>
#include <QTextBlock>
#include <QTextBrowser>
#include <QPainter>
#include <QColor>
#include <QCoreApplication>



/*****************************************************************************\
|* Constructor
\*****************************************************************************/
AsmTextEdit::AsmTextEdit(QWidget *parent)
	: QTextEdit{parent}
	,_upperCase(false)
	{
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_binaryLoaded(nd);}, NTFY_BINARY_LOADED);
	nc->addObserver([=](NotifyData &nd){_simulatorReady(nd);}, NTFY_SIM_AVAILABLE);

	_insnMap[iADC] = "adc";
	_insnMap[iAND] = "and";
	_insnMap[iASL] = "asl";
	_insnMap[iBCC] = "bcc";
	_insnMap[iBCS] = "bcs";
	_insnMap[iBEQ] = "beq";
	_insnMap[iBIT] = "bit";
	_insnMap[iBMI] = "bmi";
	_insnMap[iBNE] = "bne";
	_insnMap[iBPL] = "bpl";
	_insnMap[iBRK] = "brk";
	_insnMap[iBVC] = "bvc";
	_insnMap[iBVS] = "bvs";
	_insnMap[iCLC] = "clc";
	_insnMap[iCLD] = "cld";
	_insnMap[iCLI] = "cli";
	_insnMap[iCLV] = "clv";
	_insnMap[iCMP] = "cmp";
	_insnMap[iCPX] = "cpx";
	_insnMap[iCPY] = "cpy";
	_insnMap[iDEX] = "dex";
	_insnMap[iDEY] = "dey";
	_insnMap[iEOR] = "eor";
	_insnMap[iINC] = "inc";
	_insnMap[iINX] = "inx";
	_insnMap[iINY] = "iny";
	_insnMap[iJMP] = "jmp";
	_insnMap[iJSR] = "jsr";
	_insnMap[iLDA] = "lda";
	_insnMap[iLDX] = "ldx";
	_insnMap[iLDY] = "ldy";
	_insnMap[iLSR] = "lsr";
	_insnMap[iNOP] = "nop";
	_insnMap[iORA] = "ora";
	_insnMap[iPHA] = "pha";
	_insnMap[iPHP] = "php";
	_insnMap[iPLA] = "pla";
	_insnMap[iPLP] = "plp";
	_insnMap[iROL] = "rol";
	_insnMap[iROR] = "ror";
	_insnMap[iRTI] = "rti";
	_insnMap[iRTS] = "rts";
	_insnMap[iSBC] = "sbc";
	_insnMap[iSEC] = "sec";
	_insnMap[iSED] = "sed";
	_insnMap[iSEI] = "sei";
	_insnMap[iSTA] = "sta";
	_insnMap[iSTX] = "stx";
	_insnMap[iSTY] = "sty";
	_insnMap[iTAX] = "tax";
	_insnMap[iTAY] = "tay";
	_insnMap[iTSX] = "tsx";
	_insnMap[iTXA] = "txa";
	_insnMap[iTXS] = "txs";
	_insnMap[iTYA] = "tya";

	/*************************************************************************\
	|* Configure for assembly display
	\*************************************************************************/
	setReadOnly(true);
	_highlight = QColor(100,200,50,50);

	/*************************************************************************\
	|* Connect signals/slots
	\*************************************************************************/
	QObject::connect(this, &AsmTextEdit::cursorPositionChanged,
					 this, &AsmTextEdit::cursorChanged);
	}


#pragma mark -- Events

/*****************************************************************************\
|* Cursor event
\*****************************************************************************/
void AsmTextEdit::cursorChanged(void)
	{
	int line	= _lineNumber();
	if ((line >= 0) && (line < _lines.size()))
		{
		int infoId	= _lines[line];
		if ((infoId >= 0) && (infoId <_infoList.size()))
			{
			fprintf(stderr, "%5d => [%5d] =>", line, infoId);
			_infoList[infoId].dump();

			auto selections = extraSelections();
			selections.clear();

			QTextBrowser::ExtraSelection selection ;
			selection.format.setBackground(_highlight);
			selection.format.setProperty(QTextFormat::FullWidthSelection, true);
			selection.cursor = textCursor();
			selection.cursor.clearSelection();
			selections.append(selection);
			setExtraSelections(selections);
			}
		}
	}


/*****************************************************************************\
|* Figure out the line number from the cursor position
\*****************************************************************************/
int AsmTextEdit::_lineNumber(void)
	{
	QTextCursor cursor = textCursor();
	cursor.movePosition(QTextCursor::StartOfLine);

	int lines = 0;
	while(cursor.positionInBlock()>0)
		{
		cursor.movePosition(QTextCursor::Up);
		lines++;
		}
	QTextBlock block = cursor.block().previous();

	while(block.isValid())
		{
		lines += block.lineCount();
		block = block.previous();
		}

	return lines;
	}

#pragma mark -- Notifications

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
	_infoList.clear();

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
	int infoId = 0;
	String txt = "<pre>\n";
	for (Simulator::InstructionInfo& info : _infoList)
		{
		if (info.label.length() > 0)
			{
			txt += "\n" + info.label + ":\n   ";
			_lines.push_back(-1);
			_lines.push_back(infoId);
			}
		else
			txt += "   ";
		_lines.push_back(infoId++);

		String tmp = toHexString(info.addr, "$");
		txt += _upperCase ? ucase(tmp) : tmp;

		tmp  = "  :  " + _insnMap[info.op];
		txt += _upperCase ? ucase(tmp) : tmp;

		String X	= ",x";
		String Y	= ",y";
		int8_t rel	= (int8_t) info.arg1;

		switch (info.mode)
			{
			case anon:
				txt += "<unknown addressing mode>";
				break;

			case aACC:
				txt += _upperCase ? " A" : " a";
				break;

			case aABS:
				if (info.argLabel.length() > 0)
					txt += " "+info.argLabel;
				else
					{
					tmp = toHexString(info.arg1 + 256*info.arg2, " $");
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aABX:
				if (info.argLabel.length() > 0)
					txt += " "+info.argLabel+X;
				else
					{
					tmp  = toHexString(info.arg1 + 256*info.arg2, " $") + X;
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aABY:
				if (info.argLabel.length() > 0)
					txt += " "+info.argLabel+Y;
				else
					{
					tmp = toHexString(info.arg1 + 256*info.arg2, " $") + Y;
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aIMM:
				tmp = toHexString(info.arg1, " #$");
				txt += _upperCase ? ucase(tmp) : tmp;
				break;

			case aIMP:
				break;

			case aIND:
				if (info.argLabel.length() > 0)
					txt += " ("+info.argLabel+")";
				else
					{
					tmp  = toHexString(info.arg1 + 256*info.arg2, " ($") + ")";
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aXIN:
				if (info.argLabel.length() > 0)
					txt += " ("+info.argLabel+X+")";
				else
					{
					tmp  = toHexString(info.arg1, " ($") + X +")";
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aINY:
				if (info.argLabel.length() > 0)
					txt += " ("+info.argLabel+")"+Y;
				else
					{
					tmp  = toHexString(info.arg1, " ($") +")"+Y;
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aREL:
				if (info.argLabel.length() > 0)
					txt += " "+info.argLabel;
				else
					{
					tmp  = toHexString(rel + info.addr + 2, " $");
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aZPG:
				if (info.argLabel.length() > 0)
					txt += " "+info.argLabel;
				else
					{
					tmp  = toHexString(info.arg1, " $");
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aZPX:
				if (info.argLabel.length() > 0)
					txt += " "+info.argLabel+X;
				else
					{
					tmp  = toHexString(info.arg1, " $")+X;
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;

			case aZPY:
				if (info.argLabel.length() > 0)
					txt += " "+info.argLabel+Y;
				else
					{
					tmp  = toHexString(info.arg1, " $")+Y;
					txt += _upperCase ? ucase(tmp) : tmp;
					}
				break;
			}

		txt += "\n";
		}
	txt += "</pre>";

	setHtml(txt.c_str());
	}


/*****************************************************************************\
|* The simulator is ready to run
\*****************************************************************************/
void AsmTextEdit::_simulatorReady(NotifyData& nd)
	{
	_hw = static_cast<Atari *>(nd.voidValue());
	}

