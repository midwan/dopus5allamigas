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
#ifndef _REXX_H
#define _REXX_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

struct RexxStem
{
	struct Node rs_Node;
	char rs_Value[2];
};

#ifndef __amigaos3__
	#pragma pack()
#endif

struct RexxMsg *LIBFUNC L_CreateRexxMsgEx(REG(a0, struct MsgPort *), REG(a1, UBYTE *), REG(d0, UBYTE *));

long LIBFUNC L_SetRexxVarEx(REG(a0, struct RexxMsg *), REG(a1, char *), REG(d0, char *), REG(d1, long));

#ifdef __amigaos4__
LONG GetRexxVar(CONST struct RexxMsg *rexxmsg, CONST_STRPTR name, STRPTR *result);
LONG SetRexxVar(struct RexxMsg *rexxmsg, CONST_STRPTR name, CONST_STRPTR value, LONG length);
#endif /* __amigaos4__ */

#endif /* _REXX_H */
