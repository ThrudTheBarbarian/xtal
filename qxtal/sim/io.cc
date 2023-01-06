#include "io.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
IO::IO(void)
	{}


/*****************************************************************************\
|* Read a character - this is the 'system' default version, so it used stdin
\*****************************************************************************/
int IO::getchar(void)
	{
	int c = ::getchar();
	if (c == '\n')
		c = 0x9B;	// Atari EOF
	return c;
	}

/*****************************************************************************\
|* Read a character - this is the 'system' default version, so it used stdin
\*****************************************************************************/
int IO::peekchar(void)
	{
	int c = ::getchar();
	ungetc(c, stdin);

	if (c == '\n')
		c = 0x9B;	// Atari EOF
	return c;
	}

/*****************************************************************************\
|* Write a character - this is the 'system' default version, so it used stdout
\*****************************************************************************/
void IO::putchar(int c)
	{
	if (c == 0x9B)
		::putchar('\n');

	else if (c == 0x12)
		::putchar('-');
	else
		::putchar(c);
	fflush(stdout);
	}


/*****************************************************************************\
|* Print a string. As it stands this doesn't need to be virtual, and subclasses
|* can just use it directly, if they override the above 3
\*****************************************************************************/
int IO::printf(const char *format, ...)
	{
	char buf[1024];
	int size;

	va_list ap;
	va_start(ap, format);
	size = vsnprintf(buf, 1024, format, ap);
	va_end(ap);

	for (char *p = buf; *p; p++)
		putchar( 0xFF & (*p) );
	return size;
	}
