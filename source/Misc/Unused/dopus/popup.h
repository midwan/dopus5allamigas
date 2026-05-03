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

#ifndef _DOPUS_POPUP
#define _DOPUS_POPUP

/*****************************************************************************

 Popup menus

 *****************************************************************************/
#ifndef __amigaos3__
	#pragma pack(2)
#endif

// Defines an item in a menu
typedef struct
{
	struct MinNode node;
	char *item_name;  // Menu item name
	UWORD id;		  // Menu ID
	UWORD flags;	  // Menu item flags
	APTR data;		  // Menu item data
} PopUpItem;

#define POPUPF_LOCALE (1 << 0)	  // Item name is a locale ID
#define POPUPF_CHECKIT (1 << 1)	  // Item can be checked
#define POPUPF_CHECKED (1 << 2)	  // Item is checked
#define POPUPF_SUB (1 << 3)		  // Item has sub-items
#define POPUPF_DISABLED (1 << 4)  // Item is disabled

#define POPUP_BARLABEL (char *)-1

#define POPUP_HELPFLAG (1 << 15)  // Set if help key pressed

// Defines a popup menu
typedef struct
{
	struct MinList item_list;	 // List of menu items
	struct DOpusLocale *locale;	 // Locale data
	ULONG flags;				 // Flags
	ULONG userdata;				 // User data
	REF_CALLBACK callback;		 // Refresh callback
} PopUpMenu;

#define POPUPMF_HELP (1 << 0)	  // Supports help
#define POPUPMF_REFRESH (1 << 1)  // Use refresh callback
#define POPUPMF_ABOVE (1 << 2)	  // Open above parent window

UWORD DoPopUpMenu(struct Window *, PopUpMenu *, PopUpItem **, UWORD);
PopUpItem *GetPopUpItem(PopUpMenu *, UWORD);

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
