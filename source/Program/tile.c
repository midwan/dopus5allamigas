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

// Tile windows
void tile_windows(TileInfo *ti)
{
	TileNode *node;

	// Valid list?
	if (!ti || IsListEmpty((struct List *)&ti->box_list))
		return;

	// Cascade?
	if (ti->mode == TILE_CASCADE)
	{
		short num, te, le, wid, hgt;

		// Get cascade start position
		le = ti->base.Left;
		te = ti->base.Top;
		wid = ti->base.Width - ti->count * ti->x_offset;
		hgt = ti->base.Height - ti->count * ti->y_offset;

		// Cascade windows
		for (node = (TileNode *)ti->box_list.mlh_Head, num = 0; node->node.mln_Succ;
			 node = (TileNode *)node->node.mln_Succ, num++)
		{
			// Tile this box
			node->dims.Left = le + num * ti->x_offset;
			node->dims.Top = te + num * ti->y_offset;
			node->dims.Width = wid;
			node->dims.Height = hgt;
		}
	}

	// Otherwise tile
	else
	{
		short horiz, vert, remainder;
		short i, j, z, wid = 0, hgt = 0;

		// Get horiztonal and vertical tile counts
		for (horiz = 0;; horiz++)
		{
			// Integer square root, close enough
			if (horiz * horiz > ti->count)
			{
				--horiz;
				break;
			}
		}
		if (horiz == 0)
			return;
		vert = UDivMod32(ti->count, horiz);
		remainder = ti->count - (horiz * vert);

		// Horizontal?
		if (ti->mode == TILE_TILE_H)
			wid = UDivMod32(ti->base.Width, horiz);

		// Vertical
		else
			hgt = UDivMod32(ti->base.Height, horiz);

		// Tile windows
		node = (TileNode *)ti->box_list.mlh_Head;
		for (i = 0; i < horiz; i++)
		{
			if (i < horiz - remainder)
				z = vert;
			else
				z = vert + 1;

			if (ti->mode == TILE_TILE_H)
				hgt = UDivMod32(ti->base.Height, z);
			else
				wid = UDivMod32(ti->base.Width, z);

			// Position boxes
			for (j = 0; j < z && node->node.mln_Succ; j++)
			{
				// Store size
				node->dims.Height = hgt;
				node->dims.Width = wid;

				// Horizontal?
				if (ti->mode == TILE_TILE_H)
				{
					node->dims.Top = ti->base.Top + hgt * j;
					node->dims.Left = ti->base.Left + wid * i;
				}

				// Vertical
				else
				{
					node->dims.Top = ti->base.Top + hgt * i;
					node->dims.Left = ti->base.Left + wid * j;
				}

				// Get next
				node = (TileNode *)node->node.mln_Succ;
			}
		}
	}
}
