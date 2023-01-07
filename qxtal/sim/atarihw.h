#ifndef ATARIHW_H
#define ATARIHW_H

#include <QObject>

class Simulator;

class AtariHW : public QObject
	{
	Q_OBJECT

	public:
		/*************************************************************************\
		|* Constructor
		\*************************************************************************/
		explicit AtariHW(QObject *parent = nullptr);

		/*************************************************************************\
		|* Initialise
		\*************************************************************************/
		static void init(Simulator *sim);
	signals:

	};

#endif // ATARIHW_H
