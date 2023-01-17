#include <cstdlib>

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "predicateeditor.h"

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
	_widgetMap[OK] = ok;

	/*************************************************************************\
	|* Add a 'cancel' button to the bottom right
	\*************************************************************************/
	QPushButton *cancel = new QPushButton("Cancel", this);
	cancel->setMinimumSize(QSize(120,40));
	cancel->setMaximumSize(QSize(120,40));
	cancel->move(W-130-130,H-45);
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
	}


/*****************************************************************************\
|* Add a row with a default value
\*****************************************************************************/
void PredicateEditor::addRow(const QString& defaultValue)
	{
	if (_saLayout->count() > 0)
		_saLayout->removeWidget(_lastEntry);

	QWidget *w			= new QWidget;
	w->setMaximumHeight(35);

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setContentsMargins(-1, -1, -1, 0);
	w->setLayout(layout);

	QComboBox *what = new QComboBox(this);
	what->addItems(_what);
	what->setMinimumHeight(30);
	layout->addWidget(what);
	connect(what, &QComboBox::currentIndexChanged,
			this, &PredicateEditor::_whatChanged);

	QComboBox *cond = new QComboBox(this);
	cond->setMinimumHeight(30);
	cond->addItems(_conditions);
	layout->addWidget(cond);
	_condMap[what] = cond;

	QLineEdit *val = new QLineEdit(defaultValue);
	val->setMinimumHeight(30);
	layout->addWidget(val);
	_valMap[what] = val;

	QPushButton *del = new QPushButton("-", this);
	del->setMinimumSize(QSize(30,30));
	del->setMaximumSize(QSize(30,30));
	layout->addWidget(del);
	_valMap[what] = val;

	int idx = _saLayout->count();
	_saLayout->addWidget(w);
	_saLayout->addWidget(_lastEntry, 1);

	QString name = QString("%1").arg(idx);
	_widgetMap[name] = w;

	if (_how[0] == 0)
		{
		cond->hide();
		val->hide();
		}
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
