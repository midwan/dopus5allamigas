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

// Lister menu data
UWORD
lister_popup_data[] = {MSG_LISTER_SOURCE,
					   MENU_LISTER_SOURCE,
					   MSG_LISTER_DEST,
					   MENU_LISTER_DEST,
					   MSG_LISTER_OFF,
					   MENU_LISTER_OFF,
					   (UWORD)-1,
					   0,
					   MSG_LISTER_LOCK_SOURCE,
					   MENU_LISTER_LOCK_SOURCE,
					   MSG_LISTER_LOCK_DEST,
					   MENU_LISTER_LOCK_DEST,
					   MSG_LISTER_UNLOCK,
					   MENU_LISTER_UNLOCK,
					   (UWORD)-1,
					   0,
					   MSG_EDIT_MENU,
					   MENU_EDIT_LISTER,
					   (UWORD)-1,
					   0,
					   MSG_LISTER_LOCK_POS,
					   MENU_LISTER_LOCK_POS,
					   0,
					   0};
