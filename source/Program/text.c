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

#include <stdlib.h>

#include "dopus.h"

void TextMultiLine(struct RastPort *rp,
				   char *text,
				   short text_len,
				   short max_width,
				   short flags,
				   struct TextExtent *extent)
{
	short x, startx, y, first = 1;

	// No text?
	if (!text)
		return;

	// Get length
	if (text_len == -1)
		text_len = strlen(text);

	// Get current position
	x = rp->cp_x;
	y = rp->cp_y;
	startx = x;

	// No maximum width?
	if (max_width <= rp->Font->tf_XSize)
	{
		struct TextExtent ex;

		// Get extent
		TextExtent(rp, text, text_len, &ex);

		// Center?
		if (flags & TMLF_CENTER)
		{
			// Center horizontally
			x -= ex.te_Width >> 1;

			// Position for text
			if (!(flags & TMLF_EXTENT))
				Move(rp, x, y);
		}

		// User wants extent
		if (extent)
		{
			// Fix rectangle
			ex.te_Extent.MinX = x;
			ex.te_Extent.MinY = y - rp->TxBaseline;
			ex.te_Extent.MaxX = x + ex.te_Width - 1;
			ex.te_Extent.MaxY = ex.te_Extent.MinY + ex.te_Height - 1;

			// Get extent
			*extent = ex;
		}

		// Draw if not just getting extent
		if (!(flags & TMLF_EXTENT))
		{
			Text(rp, text, text_len);
		}

		return;
	}

	// Clear extent
	if (extent)
	{
		extent->te_Width = 0;
		extent->te_Height = 0;
		extent->te_Extent.MinX = 0;
		extent->te_Extent.MinY = y - rp->TxBaseline;
	}

	// Go through string
	while (text_len > 0)
	{
		short fit, fit1, skip = 0;
		struct TextExtent ex;

		// Get amount of the string that will fit
		fit = TextFit(rp, text, text_len, &ex, 0, 1, max_width + 1, rp->TxHeight);

		// Nothing will fit?
		if (!fit)
			break;

		// Don't split less than 4 characters onto a line
		if (fit < text_len && text_len - fit < 4)
		{
			// Get full length
			fit = text_len;

			// Get new size
			ex.te_Width = TextLength(rp, text, fit);
		}

		// Did it not all fit?
		else if (fit < text_len)
		{
			short back_limit;

			// Calculate the amount we can go backwards
			back_limit = (environment->env->desktop_flags & DESKTOPF_FORCE_SPLIT) ? (fit >> 1) : 0;

			// See if there's a space not too far away
			for (fit1 = fit + 1; fit1 > back_limit; fit1--)
			{
				// Split at punctuation
				if (text[fit1] == ' ' || text[fit1] == '-' || text[fit1] == '.' || text[fit1] == '*' ||
					text[fit1] == '+' || text[fit1] == '&' || text[fit1] == '|' || text[fit1] == '=' ||
					text[fit1] == ',' || text[fit1] == '_')
					break;
			}

			// No split here?
			if (fit1 <= back_limit)
			{
				// Split at big letter/little letter
				for (fit1 = fit + 1; fit1 > back_limit; fit1--)
				{
					if (text[fit1] >= 'a' && text[fit1] <= 'z' && text[fit1 + 1] >= 'A' && text[fit1 + 1] <= 'Z')
						break;
				}
			}

			// Get a space?
			if (fit1 > back_limit)
			{
				// Break at this point
				if (text[fit1] == ' ')
					skip = 1;
				else
					++fit1;
				fit = fit1;

				// Get new size
				ex.te_Width = TextLength(rp, text, fit);
			}

			// Otherwise, don't split?
			else if (!(environment->env->desktop_flags & DESKTOPF_FORCE_SPLIT))
			{
				// Get full width of text
				fit = text_len;
				ex.te_Width = TextLength(rp, text, text_len);
			}
		}

		// Center for text?
		if (flags & TMLF_CENTER)
		{
			x = startx - (ex.te_Width >> 1);
		}

		// Start at left
		else
		{
			x = startx;
		}

		// Draw if not just getting extent
		if (!(flags & TMLF_EXTENT))
		{
			// Draw text
			Move(rp, x, y);
			Text(rp, text, fit);
		}

		// Bump text pointer
		text += fit + skip;
		text_len -= fit + skip;

		// Bump y position
		y += rp->TxHeight;

		// Got user extent?
		if (extent)
		{
			// First time through?
			if (first)
			{
				// Initialise x position
				extent->te_Extent.MinX = x;
				first = 0;
			}

			// Otherwise, check x position
			else if (x < extent->te_Extent.MinX)
				extent->te_Extent.MinX = x;

			// Increment width
			if (ex.te_Width > extent->te_Width)
				extent->te_Width = ex.te_Width;

			// Increment height
			extent->te_Height += rp->TxHeight;
		}
	}

	// Fix extent rectangle
	if (extent)
	{
		extent->te_Extent.MaxX = extent->te_Extent.MinX + extent->te_Width - 1;
		extent->te_Extent.MaxY = extent->te_Extent.MinY + extent->te_Height - 1;
	}
}

void TabbedTextOut(struct RastPort *rp, char *string, short len, short tabsize)
{
	short segment, pos, start;

	// Get starting x-coordinate
	start = rp->cp_x;

	// Calculate tabsize in pixels
	tabsize *= rp->TxWidth;

	// Go through string
	for (pos = 0, segment = 0; pos <= len; pos++)
	{
		// Got a tab, or end of string?
		if (string[pos] == '\t' || string[pos] == 0 || pos == len)
		{
			short offset;

			// Draw this segment
			if (pos > segment)
				Text(rp, string + segment, pos - segment);

			// End of string?
			if (string[pos] != '\t')
				break;

			// Pixel position given?
			if (string[pos + 1] == '\b')
			{
				short col;

				// Get pixel column
				col = atoi(&string[pos + 2]);
				for (pos = pos + 2; string[pos] && string[pos] != '\b'; pos++)
					;

				// Move to this offset
				offset = start + col;
			}

			// Normal tab
			else
			{
				// Get next tab offset
				offset = rp->cp_x - start;
				offset += tabsize;
				offset /= tabsize;
				offset *= tabsize;
				offset += start;
			}

			// In JAM2 mode, fill background
			if (rp->DrawMode == JAM2)
			{
				short fgpen = rp->FgPen, y = rp->cp_y - rp->TxBaseline;
				SetAPen(rp, rp->BgPen);
				RectFill(rp, rp->cp_x, y, offset - 1, y + rp->TxHeight - 1);
				SetAPen(rp, fgpen);
			}

			// Move forward for next tab
			Move(rp, offset, rp->cp_y);

			// Get pointer for next segment
			segment = pos + 1;
		}
	}
}

short TabbedTextExtent(struct RastPort *rp, char *string, short len, short tabsize, struct TextExtent *extent)
{
	short segment, pos, start, x;

	// Get starting x-coordinate
	start = rp->cp_x;
	x = rp->cp_x;

	// Calculate tabsize in pixels
	tabsize *= rp->TxWidth;

	// Initialise extent
	if (extent)
	{
		extent->te_Height = rp->TxHeight;
		extent->te_Extent.MinX = x;
		extent->te_Extent.MinY = rp->cp_y - rp->TxBaseline;
		extent->te_Extent.MaxY = extent->te_Extent.MinY + rp->TxHeight - 1;
	}

	// Go through string
	for (pos = 0, segment = 0; pos <= len; pos++)
	{
		// Got a tab, or end of string?
		if (string[pos] == '\t' || string[pos] == 0 || pos == len)
		{
			short offset;

			// Add length of this segment
			if (pos > segment)
				x += TextLength(rp, string + segment, pos - segment);

			// End of string?
			if (string[pos] != '\t')
				break;

			// Pixel position given?
			if (string[pos + 1] == '\b')
			{
				short col;

				// Get pixel column
				col = atoi(&string[pos + 2]);
				for (pos = pos + 2; string[pos] && string[pos] != '\b'; pos++)
					;

				// Move to this offset
				offset = start + col;
			}

			// Normal tab
			else
			{
				// Get next tab offset
				offset = x - start;
				offset += tabsize;
				offset /= tabsize;
				offset *= tabsize;
				offset += start;
			}

			// Add offset for next tab
			x = offset;

			// Get pointer for next segment
			segment = pos + 1;
		}
	}

	// Complete extent
	if (extent)
	{
		extent->te_Extent.MaxX = x - 1;
		extent->te_Width = x - extent->te_Extent.MinX;
	}

	return (short)(x - start);
}
