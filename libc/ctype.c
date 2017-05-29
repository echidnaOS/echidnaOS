#include "ctype.h"

int isalnum(int ch)
{
	if ( isalpha(ch) || isdigit(ch) )
		return 1;

	return 0;
}

int isalpha(int ch)
{
	if ( isupper(ch) || islower(ch) )
		return 1;

	return 0;
}

int isblank(int ch)
{
	if ( ch == 0x09 || ch == 0x20 )
		return 1;

	return 0;
}

int iscntrl(int ch)
{
	if ( (ch >= 0x00 && ch <= 0x1F) || (ch == 0x7F) )
		return 1;

	return 0;
}

int isdigit(int ch)
{
	if ( ch >= 0x30 && ch <= 0x39 )
		return 1;

	return 0;
}

int isgraph(int ch)
{
	if ( ch >= 0x21 && ch <= 0x7E )
		return 1;

	return 0;
}

int islower(int ch)
{
	if ( ch >= 0x61 && ch <= 0x7A )
		return 1;

	return 0;
}

int isprint(int ch)
{
	if ( ch == 0x20 || isgraph(ch) )
		return 1;

	return 0;
}

int ispunct(int ch)
{
	if ( 
	     (ch >= 0x21 && ch <= 0x2F) ||
	     (ch >= 0x3A && ch <= 0x40) ||
	     (ch >= 0x5B && ch <= 0x60) ||
	     (ch >= 0x7B && ch <= 0x7E)
	   ) return 1;

	return 0;
}

int isspace(int ch)
{
	if ( ch == 0x20 || (ch >= 0x09 && ch <= 0x0D) )
		return 1;

	return 0;
}

int isupper(int ch)
{
	if ( ch >= 0x41 && ch <= 0x5A )
		return 1;

	return 0;
}

int isxdigit(int ch)
{
	if ( (ch >= 0x41 && ch <= 0x46) ||
	     (ch >= 0x61 && ch <= 0x66) ||
	     isdigit(ch)
	   ) return 1;

	return 0;
}

int tolower(int ch)
{
	if ( isupper(ch) )
		return ch ^ 0x20;

	return ch;
}

int toupper(int ch)
{
	if ( islower(ch) )
		return ch ^ 0x20;

	return ch;
}
