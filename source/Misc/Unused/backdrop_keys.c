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

// Get next icon for a key selection - list must be sorted in position order
BackdropObject *backdrop_key_select(BackdropObject *last, UWORD code)
{
	BackdropObject *icon;
	unsigned short icon_left, icon_top, left, top;

	// Get left and top positions
	icon_left = last->pos.Left + (last->pos.Width >> 1);
	icon_top = last->pos.Top + (last->pos.Height >> 1);

	// Right?
	if (code == CURSORRIGHT)
	{
		// Go through icons until the end
		for (icon = (BackdropObject *)last->node.ln_Succ; icon->node.ln_Succ;
			 icon = (BackdropObject *)icon->node.ln_Succ)
		{
			// Get left and top for this icon
			left = icon->pos.Left + (icon->pos.Width >> 1);
			top = icon->pos.Top + (icon->pos.Height >> 1);

			// Must be roughly the same vertically and greater horizontally
			if (left >= icon_left + (last->pos.Width >> 1) && top >= icon_top - (last->pos.Height >> 1) &&
				top <= icon_top + last->pos.Height)
				break;
		}

		// Didn't find anything?
		if (!icon->node.ln_Succ)
		{
			// Go through icons until the end
			for (icon = (BackdropObject *)last->node.ln_Succ; icon->node.ln_Succ;
				 icon = (BackdropObject *)icon->node.ln_Succ)
			{
				// Get left for this icon
				left = icon->pos.Left + (icon->pos.Width >> 1);

				// Must be just be greater horizontally
				if (left >= icon_left + (last->pos.Width >> 1))
					break;
			}

			// Still didn't find anything?
			if (!icon->node.ln_Succ)
				code = CURSORDOWN;
		}
	}

	// Down?
	if (code == CURSORDOWN)
	{
		// Moving down, we should just be able to grab the next one
		icon = (BackdropObject *)last->node.ln_Succ;
	}

	// Left?
	if (code == CURSORLEFT)
	{
		// Go through icons until the beginning
		for (icon = (BackdropObject *)last->node.ln_Pred; icon->node.ln_Pred;
			 icon = (BackdropObject *)icon->node.ln_Pred)
		{
			// Get left and top for this icon
			left = icon->pos.Left + (icon->pos.Width >> 1);
			top = icon->pos.Top + (icon->pos.Height >> 1);

			// Must be roughly the same vertically and less horizontally
			if (left <= icon_left - (last->pos.Width >> 1) && top >= icon_top - (last->pos.Height >> 1) &&
				top <= icon_top + last->pos.Height)
				break;
		}

		// Didn't find anything?
		if (!icon->node.ln_Pred)
		{
			// Go through icons until the beginning
			for (icon = (BackdropObject *)last->node.ln_Pred; icon->node.ln_Pred;
				 icon = (BackdropObject *)icon->node.ln_Pred)
			{
				// Get left for this icon
				left = icon->pos.Left + (icon->pos.Width >> 1);

				// Must be just be less horizontally
				if (left <= icon_left - (last->pos.Width >> 1))
					break;
			}

			// Still didn't find anything?
			if (!icon->node.ln_Pred)
				code = CURSORUP;
		}
	}

	// Up?
	if (code == CURSORUP)
	{
		// Moving up, we should just be able to grab the previous one
		icon = (BackdropObject *)last->node.ln_Pred;
	}

	return icon;
}
