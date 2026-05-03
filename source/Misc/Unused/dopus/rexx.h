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

#ifndef _DOPUS_AREXX
#define _DOPUS_AREXX

/****************************************************************************

 Functions for creating and handling ARexx messages

 ****************************************************************************/

#define RexxTag_Arg0 (TAG_USER + 0x1)
#define RexxTag_Arg1 (TAG_USER + 0x2)
#define RexxTag_Arg2 (TAG_USER + 0x3)
#define RexxTag_Arg3 (TAG_USER + 0x4)
#define RexxTag_Arg4 (TAG_USER + 0x5)
#define RexxTag_Arg5 (TAG_USER + 0x6)
#define RexxTag_Arg6 (TAG_USER + 0x7)
#define RexxTag_Arg7 (TAG_USER + 0x8)
#define RexxTag_Arg8 (TAG_USER + 0x9)
#define RexxTag_Arg9 (TAG_USER + 0xa)
#define RexxTag_Arg10 (TAG_USER + 0xb)
#define RexxTag_Arg11 (TAG_USER + 0xc)
#define RexxTag_Arg12 (TAG_USER + 0xd)
#define RexxTag_Arg13 (TAG_USER + 0xe)
#define RexxTag_Arg14 (TAG_USER + 0xf)
#define RexxTag_Arg15 (TAG_USER + 0x10)
#define RexxTag_VarName (TAG_USER + 0x11)
#define RexxTag_VarValue (TAG_USER + 0x12)

struct RexxMsg *CreateRexxMsgEx(struct MsgPort *, UBYTE *, UBYTE *);
void FreeRexxMsgEx(struct RexxMsg *);
long SetRexxVarEx(struct RexxMsg *, char *, char *, long);
long GetRexxVarEx(struct RexxMsg *, char *, char **);
struct RexxMsg *BuildRexxMsgEx(struct MsgPort *, UBYTE *, UBYTE *, struct TagItem *);
struct RexxMsg *BuildRexxMsgExTags(struct MsgPort *, UBYTE *, UBYTE *, Tag, ...);

#endif
