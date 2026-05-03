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
#include "layout_routines.h"

// Initialise window dimensions
void LIBFUNC L_InitWindowDims(REG(a0, struct Window *window), REG(a1, WindowDimensions *dims))
{
	// Not valid?
	if (!(dims->wd_Flags & WDF_VALID))
	{
		// Store normal dimensions
		dims->wd_Normal = *((struct IBox *)&window->LeftEdge);

		// Use minimum as zoomed dimensions
		dims->wd_Zoomed.Left = window->LeftEdge;
		dims->wd_Zoomed.Top = window->TopEdge;
		dims->wd_Zoomed.Width = window->MinWidth;
		dims->wd_Zoomed.Height = window->MinHeight;

		// Set valid flag
		dims->wd_Flags |= WDF_VALID;
	}
}

// Store window dimensions
void LIBFUNC L_StoreWindowDims(REG(a0, struct Window *window), REG(a1, WindowDimensions *dims))
{
	// Is window zoomed?
	if (window->Flags & WFLG_ZOOMED)
	{
		// Store zoom dimensions
		dims->wd_Zoomed = *((struct IBox *)&window->LeftEdge);

		// Set zoomed flag
		dims->wd_Flags |= WDF_ZOOMED;
	}

	// Not zoomed
	else
	{
		// Store normal dimensions
		dims->wd_Normal = *((struct IBox *)&window->LeftEdge);

		// Clear zoomed flag
		dims->wd_Flags &= ~WDF_ZOOMED;
	}
}

// See if size has changed
BOOL LIBFUNC L_CheckWindowDims(REG(a0, struct Window *window), REG(a1, WindowDimensions *dims))
{
	struct IBox *box;

	// If zoom state has changed, sense changes
	if ((window->Flags & WFLG_ZOOMED && !(dims->wd_Flags & WDF_ZOOMED)) ||
		(!(window->Flags & WFLG_ZOOMED) && dims->wd_Flags & WDF_ZOOMED))
	{
		// Is window zoomed?
		if (window->Flags & WFLG_ZOOMED)
			box = &dims->wd_Normal;
		else
			box = &dims->wd_Zoomed;
	}

	// Otherwise, normal sense
	else
	{
		// Is window zoomed?
		if (window->Flags & WFLG_ZOOMED)
			box = &dims->wd_Zoomed;
		else
			box = &dims->wd_Normal;
	}

	// See if size is different
	if (window->Width != box->Width || window->Height != box->Height)
		return 1;

	return 0;
}
