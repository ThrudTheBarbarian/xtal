#include <QMouseEvent>

#include "asmwidget.h"
#include "asmitem.h"

#include "notifications.h"
#include "StringUtils.h"
#include "traceitem.h"
#include "ui/fontmgr.h"

#include "predicates/predicateeditor.h"


typedef struct
	{
	uint32_t address;
	AsmItem *item;

	} PredicateContextInfo;


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


	/*************************************************************************\
	|* Create the icons
	\*************************************************************************/
	QPixmap blank = QPixmap(12,12);
	blank.fill(Qt::transparent);
	_blank	= QIcon(blank);
	_redDot	= QIcon(":/icon/rsrc/icons/red-dot.png");

	/*************************************************************************\
	|* Map instructions
	\*************************************************************************/
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
	|* Connect up the selection-changed handler
	\*************************************************************************/
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
	_hw->sim()->setTraceMemory(false);
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
	_hw->sim()->setTraceMemory(true);

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
			item->setIcon(_blank);
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
		item->setIcon(_blank);
		addItem(item);

		_itemMap[info.addr] = item;

		infoId++;
		}

	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->notify(NTFY_ASM_DONE, &_infoList);
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
	if ((item != nullptr) && _itemMap.find(item->regs().pc) != _itemMap.end())
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



#pragma mark -- events

/*****************************************************************************\
|* Event: check if we want to toggle a breakpoint
\*****************************************************************************/
void AsmWidget::mousePressEvent(QMouseEvent *event)
	{
	if (event->button() != Qt::RightButton)
		{
		event->ignore();
		QListWidget::mousePressEvent(event);
		return;
		}

	AsmItem *item	= static_cast<AsmItem *>(itemAt(event->pos()));
	if (item != nullptr)
		{
		if (item->type() != AsmItem::TYPE_BLANK)
			{
			int infoId		= item->infoId();
			if (infoId >=0 && infoId < _infoList.size())
				{
				bool shift = ((event->modifiers() & Qt::ShiftModifier) != 0);
				_toggleBreakpoint(item, shift);
				}
			}
		}
	}




#pragma mark -- Private methods


/*****************************************************************************\
|* Private Method: toggle a breakpoint on or off
\*****************************************************************************/
void AsmWidget::_toggleBreakpoint(AsmItem *item, bool shift)
	{
	int infoId = item->infoId();
	Simulator::InstructionInfo info = _infoList[infoId];

	PredicateInfo bpInfo = _hw->sim()->breakpointAt(info.addr);
	bool active = (bpInfo.num > 0);
	bool edit	= (active && shift);

	if (edit || (!active))
		{
		fprintf(stderr, "Editing breakkpoint at $%04x\n", info.addr);
		PredicateEditor *pe = new PredicateEditor("Configure breakpoint", this);

		PredicateContextInfo *pci = new PredicateContextInfo();
		pci->item = item;
		pci->address = info.addr;

		pe->setContext(pci);

		QStringList what = {"Always", "A", "X", "Y", "Status", "02222"};
		pe->setWhat(what);

		QStringList cond = {"is less than",
							"is less than or equal to",
							"is equal to",
							"is greater than or equal to",
							"is greater than",
							"is not equal to"};
		pe->setConditions(cond);
		if (edit)
			pe->configure(bpInfo);
		else
			pe->addRow();

		connect(pe, &PredicateEditor::ok, this, &AsmWidget::_bpEdited);
		pe->show();
		}
	else
		{
		// Clear
		bpInfo.release();
		_hw->sim()->clearBreakpoint(info.addr);
		item->setIcon(_blank);
		fprintf(stderr, "Clearing breakkpoint at $%04x\n", info.addr);
		}

	}




#pragma mark -- Private slots



/*****************************************************************************\
|* Private Method: Breakpoint edited ok
\*****************************************************************************/
void AsmWidget::_bpEdited(PredicateInfo info)
	{
	PredicateEditor *editor = static_cast<PredicateEditor *>(sender());
	auto pci = static_cast<PredicateContextInfo *>(editor->context());

	_hw->sim()->setBreakpoint(pci->address, info);
	pci->item->setIcon(_redDot);

	delete pci;
	editor->deleteLater();
	}
