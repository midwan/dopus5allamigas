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

#include "dopus.h"

// Menu for toolbar button banks
MenuData button_toolbar_menu[] = {
	{NM_TITLE, 0, MSG_PROJECT, 0},
	{NM_ITEM, MENU_TOOLBAR_BUTTONS_NEW, MSG_NEW_LISTER_MENU, MENUFLAG_COMM_SEQ},
	{NM_ITEM, MENU_OPEN_BUTTONS_LOCAL, MSG_OPEN, MENUFLAG_COMM_SEQ},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_SAVE_BUTTONS, MSG_SAVE_MENU, MENUFLAG_COMM_SEQ},
	{NM_ITEM, MENU_SAVEAS_BUTTONS, MSG_SAVEAS_MENU, MENUFLAG_COMM_SEQ},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_CLOSE_BUTTONS, MSG_QUIT, MENUFLAG_COMM_SEQ},
	{NM_TITLE, 0, MSG_EDIT, 0},
	{NM_ITEM, MENU_TOOLBAR_RESET_DEFAULTS, MSG_DEFAULTS, MENUFLAG_COMM_SEQ | MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('D')},
	{NM_ITEM, MENU_TOOLBAR_LAST_SAVED, MSG_LAST_SAVED, MENUFLAG_COMM_SEQ},
	{NM_ITEM, MENU_TOOLBAR_RESTORE, MSG_RESTORE, MENUFLAG_COMM_SEQ},
	{NM_END}};
