#ifndef MATHPACK_H
#define MATHPACK_H

#include <QObject>

class Simulator;

class MathPack  : public QObject
	{
	Q_OBJECT

	public:
		explicit MathPack(QObject *parent = nullptr);

		/*************************************************************************\
		|* Load the math pack into the simulator
		\*************************************************************************/
		static int load(Simulator& sim);
	};

#endif // MATHPACK_H
