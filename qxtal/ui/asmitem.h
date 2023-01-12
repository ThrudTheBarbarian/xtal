#ifndef ASMITEM_H
#define ASMITEM_H

#include <QListWidgetItem>
#include <QObject>

#include "properties.h"

class AsmItem : public QObject, public QListWidgetItem
	{
	Q_OBJECT

	public:
		/*********************************************************************\
		|* Typedefs and defines
		\*********************************************************************/
		typedef enum
			{
			TYPE_ASM = 0,
			TYPE_LABEL,
			TYPE_BLANK
			} ItemType;

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GET(ItemType, type);			// Which type of item this is
	GETSET(int, infoId, InfoId);	// Which info this represents

	public:
		AsmItem(ItemType type=TYPE_ASM);
	};

#endif // ASMITEM_H
