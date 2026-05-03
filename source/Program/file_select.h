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

#ifndef _DOPUS_FILE_SELECT
#define _DOPUS_FILE_SELECT

#define DRAG_NO 0	  // No dragging
#define DRAG_OK 1	  // Ok to drag
#define DRAG_DOING 2  // Dragging as we speak
#define DRAG_START 3  // Ready to start dragging

#define SCROLL_UP -1   // Scrolling up
#define SCROLL_NO 0	   // Not scrolling
#define SCROLL_DOWN 1  // Scrolling down

#define SELECT_OFF 0	  // Entry is now deselected
#define SELECT_ON 1		  // Entry is now selected
#define SELECT_SPECIAL 2  // Special result from selection

// Prototypes
short select_select_files(Lister *, UWORD, short, short);
void select_rmb_scroll(Lister *, short, short);
int select_toggle_entry(Lister *, int, UWORD, BOOL);
int select_off_entry(Lister *, DirEntry *);
void deselect_entry(DirBuffer *, DirEntry *);
short select_default_state(Lister *, int, int);
void select_auto_icon_select(DirEntry *, Lister *);
void select_global_state(Lister *, int);
void select_global_toggle(Lister *);
void select_update_info(DirEntry *, DirBuffer *);
void select_check_off(DirEntry *, Lister *);
void select_global_wild(Lister *, SelectData *, PathList *);
void entry_change_state(Lister *lister, DirEntry *entry, short position);
short select_key_select(Lister *lister, BOOL doubleclick);
void file_doubleclick(Lister *lister, DirEntry *entry, UWORD);
void file_run_function(Lister *lister, DirEntry *entry, Cfg_Function *func, char *, ULONG);
BOOL lister_subdrop_ok(Lister *lister, struct IntuiMessage *);

#endif
