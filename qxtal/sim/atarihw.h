#ifndef ATARIHW_H
#define ATARIHW_H

#include <QObject>

class Simulator;

class AtariHW : public QObject
	{
	Q_OBJECT

	public:
		explicit AtariHW(QObject *parent = nullptr);

		static void initialise(Simulator &sim);
	signals:

	};

#endif // ATARIHW_H
