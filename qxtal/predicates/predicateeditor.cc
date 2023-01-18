#include <cstdlib>

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "predicateeditor.h"
#include "structures.h"

/*****************************************************************************\
|* Define the constant static (!) strings from the header
\*****************************************************************************/
const QString PredicateEditor::TITLE			= "title";
const QString PredicateEditor::SCROLLAREA		= "scrollarea";
const QString PredicateEditor::ADD				= "add";
const QString PredicateEditor::OK				= "ok";
const QString PredicateEditor::CANCEL			= "cancel";

const int H										= 220;
const int W										= 600;

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
PredicateEditor::PredicateEditor(QString title, QWidget *parent)
				:QDialog(parent)
				,_title(title)
	{
	/*************************************************************************\
	|* Add a title to the top
	\*************************************************************************/
	QLabel *titleWidget = new QLabel(title, this);
	titleWidget->setMinimumSize(QSize(W,20));
	titleWidget->setMaximumSize(QSize(W,20));
	titleWidget->move(5,12);
	_widgetMap[TITLE] = titleWidget;

	/*************************************************************************\
	|* Add a scrollarea in the middle
	\*************************************************************************/
	QScrollArea *sa = new QScrollArea(this);
	sa->setMinimumSize(QSize(W-25,H-95));
	sa->setMaximumSize(QSize(W-25,H-95));
	sa->resize(W-25,300);
	sa->move(15,40);
	_widgetMap[SCROLLAREA] = sa;

	QWidget *scrolled = new QWidget;
	scrolled->setMinimumWidth(W-50);
	sa->setWidget(scrolled);

	_saLayout = new QVBoxLayout;
	_saLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	_saLayout->setSpacing(0);
	_saLayout->setContentsMargins(-1, -1, -1, 0);
	scrolled->setLayout(_saLayout);

	/*************************************************************************\
	|* Add an 'ok' button to the bottom right
	\*************************************************************************/
	QPushButton *ok = new QPushButton("Ok", this);
	ok->setMinimumSize(QSize(120,40));
	ok->setMaximumSize(QSize(120,40));
	ok->setAutoDefault(true);
	ok->move(W-130,H-45);
	connect(ok, &QPushButton::pressed, this, &PredicateEditor::_ok);
	_widgetMap[OK] = ok;

	/*************************************************************************\
	|* Add a 'cancel' button to the bottom right
	\*************************************************************************/
	QPushButton *cancel = new QPushButton("Cancel", this);
	cancel->setMinimumSize(QSize(120,40));
	cancel->setMaximumSize(QSize(120,40));
	cancel->move(W-130-130,H-45);
	connect(cancel, &QPushButton::pressed, this, &PredicateEditor::_cancel);
	_widgetMap[CANCEL] = cancel;

	/*************************************************************************\
	|* Add an 'add' button to the bottom left
	\*************************************************************************/
	QPushButton *pb = new QPushButton("+", this);
	pb->setMinimumSize(QSize(40,40));
	pb->setMaximumSize(QSize(40,40));
	pb->move(15,H-45);
	connect(pb, &QPushButton::pressed, this, &PredicateEditor::_addRow);
	_widgetMap[ADD] = pb;

	/*************************************************************************\
	|* Configure this widget's size
	\*************************************************************************/
	setMinimumSize(QSize(610,H));
	setMaximumSize(QSize(610,H));
	resize(610,400);

	/*************************************************************************\
	|* Add in a dummy entry last, to take up any extra space
	\*************************************************************************/
	_lastEntry = new QLabel("", this);
	_lastEntry->setObjectName("$ignore");
	}


/*****************************************************************************\
|* Add a row with a default value
\*****************************************************************************/
void PredicateEditor::addRow(const QString& defaultValue, int wIdx, int cIdx)
	{
	if (_saLayout->count() > 0)
		_saLayout->removeWidget(_lastEntry);

	QWidget *w			= new QWidget;
	w->setMaximumHeight(35);
	w->setObjectName("predicate-row");

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setContentsMargins(-1, -1, -1, 0);
	w->setLayout(layout);

	QComboBox *what = new QComboBox(this);
	what->addItems(_what);
	what->setMinimumHeight(30);
	what->setObjectName("what");
	what->setCurrentIndex(wIdx);
	layout->addWidget(what);
	connect(what, &QComboBox::currentIndexChanged,
			this, &PredicateEditor::_whatChanged);

	QComboBox *cond = new QComboBox(this);
	cond->setMinimumHeight(30);
	cond->addItems(_conditions);
	cond->setObjectName("cond");
	cond->setCurrentIndex(cIdx);
	layout->addWidget(cond);
	_condMap[what] = cond;

	QLineEdit *val = new QLineEdit(defaultValue);
	val->setMinimumHeight(30);
	val->setObjectName("val");
	layout->addWidget(val);
	_valMap[what] = val;

	QPushButton *del = new QPushButton("-", this);
	del->setMinimumSize(QSize(30,30));
	del->setMaximumSize(QSize(30,30));
	layout->addWidget(del);
	connect(del, &QPushButton::pressed, this, &PredicateEditor::_delRow);
	_delMap[del] = w;
	_hideMap[w] = del;

	_saLayout->addWidget(w);
	_saLayout->addWidget(_lastEntry, 1);

	if (_how[wIdx] == 0)
		{
		cond->hide();
		val->hide();
		}

	_checkIfLastItem();
	}

/*****************************************************************************\
|* Set the title text
\*****************************************************************************/
void PredicateEditor::setTitle(const QString& title)
	{
	QLabel *titleWidget = static_cast<QLabel *>(_widgetMap[TITLE]);
	titleWidget->setText(title);
	}


/*****************************************************************************\
|* Set the preferred size
\*****************************************************************************/
QSize PredicateEditor::sizeHint()
	{
	return QSize(610,400);
	}

/*****************************************************************************\
|* Set 'what' and how it ought to be represented
\*****************************************************************************/
void PredicateEditor::setWhat(QStringList& what)
	{
	QString how = what.takeLast();
	_what = what;

	int len = how.length();
	String info = how.toStdString();
	const char *bytes =info.c_str();

	_how.clear();
	for (int i=0; i<len; i++)
		_how.push_back(bytes[i]-'0');
	}


/*****************************************************************************\
|* Configure the current editor
\*****************************************************************************/
void PredicateEditor::configure(PredicateInfo info)
	{
	for (int i=0; i<info.num; i++)
		{
		QString val = QString("%1").arg(info.values[i]);
		addRow(val, info.what[i], info.cond[i]);
		}
	}

#pragma mark -- private methods


/*****************************************************************************\
|* Private method - cancel out
\*****************************************************************************/
void PredicateEditor::_cancel(void)
	{
	emit cancel();
	done(0);
	}

/*****************************************************************************\
|* Private method - construct information about the dialog and close
\*****************************************************************************/
void PredicateEditor::_ok(void)
	{
	done(0);
	PredicateInfo info;
	info.num		= _hideMap.size();
	info.what		= new int[info.num];
	info.cond		= new int[info.num];
	info.values		= new int[info.num];
	info.enabled	= (info.num > 0);

	QScrollArea *scroll = static_cast<QScrollArea *>(_widgetMap[SCROLLAREA]);
	QObjectList kids = scroll->widget()->children();

	int idx = 0;
	for (QObject * child : kids)
		{
		QWidget *widget = static_cast<QWidget*>(child);
		if (widget->objectName() == "predicate-row")
			{
			QObjectList rowkids = widget->children();
			for (int i=0; i<rowkids.size(); i++)
				{
				QWidget *obj = static_cast<QWidget *>(rowkids.at(i));
				if (obj->objectName() == "what")
					{
					QComboBox *what = static_cast<QComboBox*>(obj);
					info.what[idx] = what->currentIndex();
					}
				if (obj->objectName() == "cond")
					{
					QComboBox *cond = static_cast<QComboBox*>(obj);
					info.cond[idx] = cond->currentIndex();
					}
				if (obj->objectName() == "val")
					{
					QLineEdit *edit = static_cast<QLineEdit *>(obj);
					QString num		= edit->text();
					if (num.startsWith("$"))
						{
						QString strip = num.remove(0,1);
						info.values[idx] = strip.toInt(nullptr, 16);
						}
					else
						info.values[idx] = num.toInt(nullptr, 16);
					}
				}
			idx ++;
			}
		}

	emit ok(info);
	}

/*****************************************************************************\
|* Private method - see if there's only 1 option left in the editor
\*****************************************************************************/
void PredicateEditor::_checkIfLastItem(void)
	{
	if (_hideMap.size() == 1)	// lastItem + entry
		{
		for (Elements<QWidget*,QWidget*> kv : _hideMap)
			kv.value->hide();
		}
	else
		{
		for (Elements<QWidget*,QWidget*> kv : _hideMap)
			kv.value->show();
		}
	}



#pragma mark -- private slots



/*****************************************************************************\
|* Private slot: the user changed 'what'
\*****************************************************************************/
void PredicateEditor::_whatChanged(int idx)
	{
	if ((idx >= 0) && (idx < _how.size()))
		{
		QWidget* obj = static_cast<QWidget *>(sender());
		if (_how[idx] == 0)
			{
			_condMap[obj]->hide();
			_valMap[obj]->hide();
			}
		else
			{
			_condMap[obj]->show();
			_valMap[obj]->show();
			}
		}
	}


#pragma mark -- signal handlers


/*****************************************************************************\
|* Handle adding a row
\*****************************************************************************/
void PredicateEditor::_addRow(void)
	{
	addRow("");
	}

/*****************************************************************************\
|* Handle adding a row
\*****************************************************************************/
void PredicateEditor::_delRow(void)
	{
	QWidget* obj = static_cast<QWidget *>(sender());
	_delMap[obj]->hide();
	_hideMap.erase(_delMap[obj]);
	_saLayout->removeWidget(_delMap[obj]);
	_checkIfLastItem();
	}
