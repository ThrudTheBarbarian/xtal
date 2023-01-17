#ifndef PREDICATEEDITOR_H
#define PREDICATEEDITOR_H

#include <QDialog>
#include <QObject>

class QPushButton;
class QLabel;
class QVBoxLayout;

#include "properties.h"
#include "predicateinfo.h"

class PredicateEditor : public QDialog
	{
	Q_OBJECT

	public:
		/*********************************************************************\
		|* Typedefs
		\*********************************************************************/
		typedef std::map<QWidget *, QWidget*> RefMap;
		typedef std::map<QString, QWidget*> WidgetMap;
		typedef std::vector<int>	IntList;

		/*********************************************************************\
		|* Keys to access the map
		\*********************************************************************/
		static const QString TITLE;
		static const QString SCROLLAREA;
		static const QString ADD;
		static const QString OK;
		static const QString CANCEL;

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(WidgetMap, widgetMap);					// Map of widgets
	GET(QString, title);						// Title string for the widget
	GET(QVBoxLayout*, saLayout);				// Scrollarea layout
	GET(QStringList, what);						// Possible values for 'what'
	GETSET(QStringList, conditions, Conditions);// Possible conditions
	GETSET(void*, context, Context);			// Context key

	private:
		QWidget *_lastEntry;					// Dummy last predicate widget
		IntList _how;							// How to represent predicate
		RefMap _condMap;						// Condition-widget map
		RefMap _valMap;							// Value-widget map
		RefMap _delMap;							// Delete-widget map
		RefMap _hideMap;						// What to hide if <2 items


		/*********************************************************************\
		|* Check to see if there's only one real item left in the list
		\*********************************************************************/
		void _checkIfLastItem(void);

	public:
		PredicateEditor(QString title = "",QWidget *parent = 0);


		/*********************************************************************\
		|* Set the title text
		\*********************************************************************/
		void setTitle(const QString& text);

		/*********************************************************************\
		|* Set what the 'what' ought to be, last entry defines how many
		|* columns are used
		\*********************************************************************/
		void setWhat(QStringList& what);

		/*********************************************************************\
		|* Add a row
		\*********************************************************************/
		void addRow(const QString& defaultValue = "");

		/*********************************************************************\
		|* Set a preferred size
		\*********************************************************************/
		QSize sizeHint(void);

	private slots:

		/*********************************************************************\
		|* User changed 'what'
		\*********************************************************************/
		void _whatChanged(int index);

		/*********************************************************************\
		|* OK / Cancel callbacks
		\*********************************************************************/
		void _cancel(void);
		void _ok(void);

		/*********************************************************************\
		|* Add/delete a row (callback from signal)
		\*********************************************************************/
		void _addRow(void);
		void _delRow(void);

	signals:
		/*********************************************************************\
		|* User cancelled out
		\*********************************************************************/
		void cancel(void);
		void ok(PredicateInfo info);
	};

#endif // PREDICATEEDITOR_H
