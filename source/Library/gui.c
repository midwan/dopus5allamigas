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

#include "dopuslib.h"

// Draw a 3D box
void LIBFUNC L_DrawBox(REG(a0, struct RastPort *rp),
					   REG(a1, struct Rectangle *rect),
					   REG(a2, struct DrawInfo *info),
					   REG(d0, BOOL recessed))
{
	UBYTE shine, shadow;

	// Get pens we need
	if (recessed)
	{
		shine = info->dri_Pens[SHADOWPEN];
		shadow = info->dri_Pens[SHINEPEN];
	}
	else
	{
		shine = info->dri_Pens[SHINEPEN];
		shadow = info->dri_Pens[SHADOWPEN];
	}

	// Do shine lines
	SetAPen(rp, shine);
	Move(rp, rect->MinX, rect->MaxY);
	Draw(rp, rect->MinX, rect->MinY);
	Draw(rp, rect->MaxX - 1, rect->MinY);

	// Do shadow lines
	SetAPen(rp, shadow);
	Move(rp, rect->MaxX, rect->MinY);
	Draw(rp, rect->MaxX, rect->MaxY);
	Draw(rp, rect->MinX + 1, rect->MaxY);
}

// Draw a 3D string box
void LIBFUNC L_DrawFieldBox(REG(a0, struct RastPort *rp),
							REG(a1, struct Rectangle *rect),
							REG(a2, struct DrawInfo *info))
{
	draw_field_box(rp, rect, info, (struct Library *)GfxBase);
}

void draw_field_box(struct RastPort *rp, struct Rectangle *rect, struct DrawInfo *info, struct Library *GfxBase)
{
	// Do shine lines
	SetAPen(rp, info->dri_Pens[SHINEPEN]);
	Move(rp, rect->MinX, rect->MaxY);
	Draw(rp, rect->MinX, rect->MinY);
	Draw(rp, rect->MaxX - 1, rect->MinY);
	Draw(rp, rect->MaxX - 1, rect->MaxY - 1);
	Draw(rp, rect->MinX + 2, rect->MaxY - 1);

	// Do shadow lines
	SetAPen(rp, info->dri_Pens[SHADOWPEN]);
	Move(rp, rect->MaxX, rect->MinY);
	Draw(rp, rect->MaxX, rect->MaxY);
	Draw(rp, rect->MinX + 1, rect->MaxY);
	Draw(rp, rect->MinX + 1, rect->MinY + 1);
	Draw(rp, rect->MaxX - 2, rect->MinY + 1);
}

// Get information about a screen
ULONG LIBFUNC L_ScreenInfo(REG(a0, struct Screen *screen))
{
	ULONG info = 0, mode;
	struct DisplayInfo disp;

	// Valid screen?
	if (!screen)
		return 0;

	// Get display mode
	if ((mode = GetVPModeID(&screen->ViewPort)) == INVALID_ID)
		return 0;

	// Get DisplayInfo
	if (!(GetDisplayInfoData(0, (char *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, mode)))
		return 0;

	// See if screen is lo-res
	if (disp.Resolution.y >= disp.Resolution.x * 2)
		info = SCRI_LORES;

	return info;
}
