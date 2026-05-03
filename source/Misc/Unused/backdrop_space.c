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

// Initialise space region
void backdrop_init_space(BackdropInfo *info)
{
	BackdropObject *icon;
	struct Rectangle rect;

	// Clear region
	ClearRegion(info->icon_space);

	// Lock icon list
	lock_listlock(&info->objects, FALSE);

	// Go through icons
	for (icon = (BackdropObject *)info->objects.list.lh_Head; icon->node.ln_Succ;
		 icon = (BackdropObject *)icon->node.ln_Succ)
	{
		// No position?
		if (icon->flags & BDOF_NO_POSITION)
			continue;

		// Or icon into region
		OrRectRegion(info->icon_space, &icon->full_size);
	}

	// Unlock list
	unlock_listlock(&info->objects);

	// Invert region so that the free spaces are marked
	rect = info->area;

	// If virtual area smaller than display area, use that
	if (RECTWIDTH(&rect) < RECTWIDTH(&info->size) || RECTHEIGHT(&rect) < RECTHEIGHT(&info->size))
		rect = info->size;

	// Xor into region
	XorRectRegion(info->icon_space, &rect);
}

// Add an icon to the display
void backdrop_add_space(BackdropInfo *info, BackdropObject *icon)
{
	// Invert space region
	XorRectRegion(info->icon_space, &info->area);

	// Or icon into region
	OrRectRegion(info->icon_space, &icon->full_size);

	// Invert region again
	XorRectRegion(info->icon_space, &info->area);
}
