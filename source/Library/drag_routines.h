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
#ifndef _DRAG_ROUTINES_H
#define _DRAG_ROUTINES_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct _DragInfo
{
	struct RastPort *rastport;	// Stores RastPort this bob belongs to
	struct ViewPort *viewport;	// Stores ViewPort

	WORD width;	  // Bob width
	WORD height;  // Bob height

	struct VSprite sprite;	// VSprite structure
	struct Bob bob;			// BOB structure

	struct VSprite head;   // GEL list head sprite
	struct VSprite tail;   // GEL list tail sprite
	struct GelsInfo info;  // GEL info

	int show_flag;	// Indicates if bob is valid

	struct RastPort drag_rp;  // RastPort we can draw into
	struct BitMap drag_bm;	  // BitMap we can draw into

	struct Window *window;	// Window pointer

	long offset_x;
	long offset_y;
	long userdata;
	long pad[4];
} DragInfo;

#ifndef __amigaos3__
	#pragma pack()
#endif

// prototypes
DragInfo *GetDragInfo(struct Window *, WORD, WORD);
void FreeDragInfo(DragInfo *);
void GetDragImage(DragInfo *, WORD, WORD);
void GetDragMask(DragInfo *);
void ShowDragImage(DragInfo *, WORD, WORD);
void HideDragImage(DragInfo *);
void StampDragImage(DragInfo *, WORD, WORD);

#endif