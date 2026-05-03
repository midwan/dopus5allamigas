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

#ifndef _DOPUS_DRAG
#define _DOPUS_DRAG

/*****************************************************************************

 Drag routines

 *****************************************************************************/

#ifndef GRAPHICS_GELS_H
	#include <graphics/gels.h>
#endif

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

	unsigned long flags;  // Flags

	struct RastPort drag_rp;  // RastPort we can draw into
	struct BitMap drag_bm;	  // BitMap we can draw into

	struct Window *window;	// Window pointer
} DragInfo;

typedef struct
{
	struct VSprite head;   // GEL list head sprite
	struct VSprite tail;   // GEL list tail sprite
	struct GelsInfo info;  // GEL info
} DragInfoExtra;

#ifndef __amigaos3__
	#pragma pack()
#endif

#define DRAGF_VALID (1 << 0)		// Bob is valid
#define DRAGF_OPAQUE (1 << 1)		// Bob should be opaque
#define DRAGF_DONE_GELS (1 << 2)	// Installed GelsInfo
#define DRAGF_NO_LOCK (1 << 3)		// Don't lock layers
#define DRAGF_TRANSPARENT (1 << 4)	// Bob should be transparent (use with opaque)

void FreeDragInfo(DragInfo *);
void GetDragImage(DragInfo *, long, long);
DragInfo *GetDragInfo(struct Window *, struct RastPort *, long, long, long);
void GetDragMask(DragInfo *);
void HideDragImage(DragInfo *);
void ShowDragImage(DragInfo *, long, long);
void StampDragImage(DragInfo *, long, long);

#endif
