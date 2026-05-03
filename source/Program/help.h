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

#ifndef _DOPUS_HELP
#define _DOPUS_HELP

/*#if defined(__amigaos3__)
#include <clib/amigaguide_protos.h>
#else*/
#include <proto/amigaguide.h>
//#endif

#define GENERIC_BUTTON "Button"
#define GENERIC_TOOLBAR_BUTTON "ToolbarButton"
#define GENERIC_LISTER_MENU "ListerMenu"
#define GENERIC_USER_MENU "UserMenu"
#define HELP_ARROW_BUTTON "Toolbar Arrow"
#define HELP_LISTER_MENU_POPUP "Lister Menu PopUp"
#define HELP_LISTER_PATH_POPUP "Lister Path PopUp"
#define HELP_LISTER_STATUS_POPUP "Lister Status PopUp"
#define HELP_LISTER_FILE_COUNT "Lister File Count"
#define HELP_LISTER_HELP "Lister"
#define HELP_PROGRAM_GROUP "Program Group"
#define HELP_MAIN "Main"
#define HELP_APPMENUITEM "AppMenuItem"

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	UWORD id;
	char *name;
} MenuHelp;

#ifndef __amigaos3__
	#pragma pack()
#endif

void help_get_help(short, short, unsigned short);
void help_button_help(Cfg_Button *, Cfg_Function *, unsigned short, short, char *);
void help_menu_help(long, char *);
void help_show_help(char *, char *);
AMIGAGUIDECONTEXT help_open_database(struct NewAmigaGuide *, char *, struct Screen *, char *);

enum {
	HELPCMD_base,

	HELPCMD_LINK,
};

#define HELP_REAL (1 << 31)
#define HELP_NEWFILE (1 << 30)

#endif
