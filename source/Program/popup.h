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

#ifndef DOPUS_POPUP_STUFF
#define DOPUS_POPUP_STUFF

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct _PopUpExt
{
	struct Node pe_Node;
	ULONG pe_Type;		   // WBDISK, etc
	char pe_FileType[40];  // Filetype name
	char pe_Command[40];   // Command to run
	char pe_Menu[40];	   // Menu text
	ULONG pe_Flags;		   // Flags
} PopUpExt;

#ifndef __amigaos3__
	#pragma pack()
#endif

#define POPUP_ALL ((ULONG)-1)
#define POPUP_LEFTOUT 7
#define POPUP_LISTER 8
#define POPUP_LISTER2 9

#define POPUPEXTF_HANDLER (1 << 0)

void add_popup_ext(char *, Att_List *, char *, ULONG);
void remove_popup_ext(char *);
BOOL popup_ext_check(char *);
void popup_run_func(PopUpExt *, BackdropObject *, char *, Lister *);
PopUpHandle *popup_from_bank(Cfg_ButtonBank *bank, short *);

#endif
