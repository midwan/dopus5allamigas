/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
AROS Public License for more details.

The release of Directory Opus 5 under the GPL in NO WAY affects
the existing commercial status of Directory Opus for Windows.

For more information on Directory Opus for Windows please see:

				 http://www.gpsoft.com.au

*/

#include <ctype.h>
#include <stdlib.h>

#include "dopuslib.h"

void parse_number(char **ptr, short *val);

BOOL LIBFUNC L_SavePos(REG(a0, char *name), REG(a1, struct IBox *box), REG(d0, short fontsize))
{
	char buf[100];
	lsprintf(buf, "%ld/%ld/%ld/%ld/%ld", box->Left, box->Top, box->Width, box->Height, fontsize);
	L_SetEnv(name, buf, TRUE);
	return TRUE;
}

BOOL LIBFUNC L_LoadPos(REG(a0, char *name), REG(a1, struct IBox *box), REG(d0, short *fontsize))
{
	char buf[100];
	if (GetVar(name, buf, sizeof(buf), GVF_GLOBAL_ONLY) > 0)
	{
		char *ptr = buf;
		parse_number(&ptr, &box->Left);
		parse_number(&ptr, &box->Top);
		parse_number(&ptr, &box->Width);
		parse_number(&ptr, &box->Height);
		parse_number(&ptr, fontsize);
		return TRUE;
	}
	return FALSE;
}

void parse_number(char **ptr, short *val)
{
	// Digit?
	if (isdigit(*(*ptr)))
	{
		*val = atoi(*ptr);
		while (*(*ptr) && isdigit(*(*ptr)))
			++*ptr;
		++*ptr;
	}
}
