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

#ifndef _DOPUS_GRAPHICS
#define _DOPUS_GRAPHICS

// Prototypes
void do3dbox(struct RastPort *, struct Rectangle *, BOOL);
void do3dstringbox(struct RastPort *, struct Rectangle *);
void fill_out_cover_gadget(struct Gadget *, struct Gadget *);
void show_list_selector(Lister *, short, BOOL);
void erase_window(struct Window *);
struct Gadget *create_iconify_gadget(struct Screen *, struct List *, BOOL);
void LockScreenLayer(struct Screen *screen);
void UnlockScreenLayer(struct Screen *screen);
void setwindowtitle_fix(struct Window *window, char *title);

#endif
