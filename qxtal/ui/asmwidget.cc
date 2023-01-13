#include "asmwidget.h"
#include "asmitem.h"

#include "notifications.h"
#include "StringUtils.h"
#include "traceitem.h"
#include "ui/fontmgr.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
AsmWidget::AsmWidget(QWidget *parent)
		  :QListWidget{parent}
		  ,_upperCase(false)
		  ,_propagateSelection(true)
	{
	_font = FontMgr::monospacedFont();

	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_simulatorReady(nd);}, NTFY_SIM_AVAILABLE);
	nc->addObserver([=](NotifyData &nd){_binaryLoaded(nd);}, NTFY_BINARY_LOADED);
	nc->addObserver([=](NotifyData &nd){_traceSelection(nd);}, NTFY_TRACE_SEL_CHG);


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


	QObject::connect(this, &AsmWidget::currentItemChanged,
					 this, &AsmWidget::_handleSelectionChanged);
	}



#pragma mark -- Notifications

/*****************************************************************************\
|* Notification: A binary was loaded
\*****************************************************************************/
void AsmWidget::_binaryLoaded(NotifyData& nd)
	{
	uint32_t addr	= _org = nd.integerValue();
	uint8_t badMem	= Simulator::MS_INVALID | Simulator::MS_UNDEFINED;

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
	for (Simulator::InstructionInfo& info : _infoList)
		{
		if (info.label.length() > 0)
			{
			AsmItem *item = new AsmItem(AsmItem::TYPE_BLANK);
			item->setText("");
			addItem(item);

			item = new AsmItem(AsmItem::TYPE_LABEL);
			item->setText(QString::fromStdString(info.label + ":"));
			item->setInfoId(infoId);
			item->setData(Qt::FontRole, _font);
			addItem(item);
			}

		String txt = "   ";

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

		AsmItem *item = new AsmItem();
		item->setText(QString::fromStdString(txt));
		item->setInfoId(infoId);
		item->setData(Qt::FontRole, _font);
		addItem(item);

		_itemMap[info.addr] = item;

		infoId++;
		}
	}


/*****************************************************************************\
|* The simulator is ready to run
\*****************************************************************************/
void AsmWidget::_simulatorReady(NotifyData& nd)
	{
	_hw = static_cast<Atari *>(nd.voidValue());
	}



/*****************************************************************************\
|* Notification: A binary was loaded
\*****************************************************************************/
void AsmWidget::_traceSelection(NotifyData& nd)
	{
	TraceItem *item = static_cast<TraceItem *>(nd.voidValue());
	if (_itemMap.find(item->regs().pc) != _itemMap.end())
		{
		_propagateSelection = false;
		setCurrentItem(_itemMap[item->regs().pc]);
		}
	}



#pragma mark -- signals



/*****************************************************************************\
|* Signal handler: our selection changed
\*****************************************************************************/
void AsmWidget::_handleSelectionChanged(QListWidgetItem *current,
										QListWidgetItem *previous)
	{
	if (_propagateSelection)
		{
		AsmItem *item = static_cast<AsmItem *>(current);
		if (item != nullptr)
			{
			int infoId = item->infoId();
			if ((infoId > 0) && (infoId < _infoList.size()))
				{
				Simulator::InstructionInfo info = _infoList[infoId];
				int address = info.addr;
				fprintf(stderr, "info addr: $%04x\n", address);

				auto nc = NotifyCenter::defaultNotifyCenter();
				nc->notify(NTFY_ASM_SEL_CHG, address);
				}
			}
		}
	_propagateSelection = true;
	}
