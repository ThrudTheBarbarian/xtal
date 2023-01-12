#include "fontmgr.h"

#include <QFontInfo>

/*****************************************************************************\
|* Get a monospaced font
\*****************************************************************************/
static bool isFixedPitch(const QFont &font)
	{
	const QFontInfo fi(font);
	return fi.fixedPitch();
	}

static QFont getMonospaceFont()
	{
	QFont font("monospace");
	if (isFixedPitch(font)) return font;

	font.setStyleHint(QFont::Monospace);
	if (isFixedPitch(font)) return font;

	font.setStyleHint(QFont::TypeWriter);
	if (isFixedPitch(font)) return font;

	font.setFamily("courier");
	if (isFixedPitch(font)) return font;

	return font;
	}


/*****************************************************************************\
|* Constructor
\*****************************************************************************/
FontMgr::FontMgr()
	{}

/*****************************************************************************\
|* Return a monospaced font
\*****************************************************************************/
QFont FontMgr::monospacedFont(void)
	{
	return getMonospaceFont();
	}
