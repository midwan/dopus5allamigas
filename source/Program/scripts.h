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

#ifndef SCRIPTS_H
#define SCRIPTS_H

enum {
	SCRIPT_STARTUP,
	SCRIPT_SHUTDOWN,
	SCRIPT_HIDE,
	SCRIPT_REVEAL,
	SCRIPT_DISKINSERT,
	SCRIPT_DISKREMOVE,
	SCRIPT_DOUBLECLICK,
	SCRIPT_OPEN_LISTER,
	SCRIPT_OPEN_BUTTONS,
	SCRIPT_OPEN_GROUP,
	SCRIPT_CLOSE_LISTER,
	SCRIPT_CLOSE_BUTTONS,
	SCRIPT_CLOSE_GROUP,
	SCRIPT_BAD_DISKINSERT,
	SCRIPT_RIGHT_DOUBLECLICK,
	SCRIPT_MID_DOUBLECLICK,
	SCRIPT_PRESTARTUP,
};

void InitScripts(void);
void FreeScripts(void);
BOOL RunScript_Node(Att_Node *, char *);
BOOL RunScript_Name(char *, char *);
Cfg_ButtonFunction *FindScript_Function(char *);
void InitSoundEvents(BOOL force);

extern Att_List *script_list;

#define SCRIPTF_NO_LOCK (1 << 0)
#define SCRIPTF_NO_SOUND (1 << 1)
#define SCRIPTF_STARTUP (1 << 2)
#define SCRIPTF_SYNC (1 << 3)

#endif
