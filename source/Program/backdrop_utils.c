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

struct line
{
	Point p1;
	Point p2;
};

// Find if an intersection point is on a line
short static inline geo_ccw(Point *p0, Point *p1, Point *p2)
{
	short dx1, dx2, dy1, dy2;

	dx1 = p1->x - p0->x;
	dy1 = p1->y - p0->y;
	dx2 = p2->x - p0->x;
	dy2 = p2->y - p0->y;

	if (dx1 * dy2 > dy1 * dx2)
		return 1;
	if (dx1 * dy2 < dy1 * dx2)
		return -1;

	if ((dx1 * dx2 < 0) || (dy1 * dy2 < 0))
		return -1;
	if ((dx1 * dx1 + dy1 * dy1) < (dx2 * dx2 + dy2 * dy2))
		return 1;

	return 0;
}

// Find if two lines intersect
short static inline geo_line_intersect(struct line *l1, struct line *l2)
{
	return (short)(((geo_ccw(&l1->p1, &l1->p2, &l2->p1) * geo_ccw(&l1->p1, &l1->p2, &l2->p2)) <= 0) &&
				   ((geo_ccw(&l2->p1, &l2->p2, &l1->p1) * geo_ccw(&l2->p1, &l2->p2, &l1->p2)) <= 0));
}

// See if a point is within a box
BOOL static inline geo_point_in_box(WORD x, WORD y, struct Rectangle *box)
{
	return (BOOL)(((box->MinX <= box->MaxX && x >= box->MinX && x <= box->MaxX) ||
				   (box->MaxX < box->MinX && x >= box->MaxX && x <= box->MinX)) &&
				  ((box->MinY <= box->MaxY && y >= box->MinY && y <= box->MaxY) ||
				   (box->MaxY < box->MinY && y >= box->MaxY && y <= box->MinY)));
}

// Find if two rectangles intersect
short geo_box_intersect(struct Rectangle *r1, struct Rectangle *r2)
{
	struct line lines[2][4];
	register struct Rectangle *r;
	register short b, a;

	// Build lines
	r = r1;
	for (a = 0; a < 2; a++)
	{
		// Top side
		lines[a][0].p1.x = r->MinX;
		lines[a][0].p1.y = r->MinY;
		lines[a][0].p2.x = r->MaxX;
		lines[a][0].p2.y = r->MinY;

		// Right side
		lines[a][1].p1.x = r->MaxX;
		lines[a][1].p1.y = r->MinY;
		lines[a][1].p2.x = r->MaxX;
		lines[a][1].p2.y = r->MaxY;

		// Bottom side
		lines[a][2].p1.x = r->MaxX;
		lines[a][2].p1.y = r->MaxY;
		lines[a][2].p2.x = r->MinX;
		lines[a][2].p2.y = r->MaxY;

		// Left side
		lines[a][3].p1.x = r->MinX;
		lines[a][3].p1.y = r->MaxY;
		lines[a][3].p2.x = r->MinX;
		lines[a][3].p2.y = r->MinY;

		// Do next rectangle
		r = r2;
	}

	// See if rectangle sides intersect
	for (a = 0; a < 4; a++)
	{
		for (b = 0; b < 4; b++)
		{
			if (geo_line_intersect(&lines[0][a], &lines[1][b]))
				return 1;
		}
	}

	// If no sides intersect, one rectangle may be inside another completely
	for (a = 0; a < 2; a++)
	{
		if (geo_point_in_box(r1->MinX, r1->MinY, r))
			return 1;
		r = r1;
		r1 = r2;
	}

	return 0;
}

// Find an object from coordinates
BackdropObject *backdrop_get_object(BackdropInfo *info, short x, short y, UWORD flags)
{
	register BackdropObject *object;

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Is object position ok?
		if (!(object->flags & BDOF_NO_POSITION))
		{
			// See if point is within object
			if (x >= object->image_rect.MinX && x <= object->image_rect.MaxX && y >= object->image_rect.MinY &&
				y <= object->image_rect.MaxY)
			{
				/*
								// Image mask?
								if ((flags&BDGOF_CHECK_MASK) && object->state==0 && object->image_mask[0])
								{
									short width,bit,cx,cy;

									// Convert coordinates to image-relative
									cx=x-object->image_rect.MinX;
									cy=y-object->image_rect.MinY;

									// Get width of image in columns
									width=(((struct Image *)object->icon->do_Gadget.GadgetRender)->Width+15)>>4;

									// Get column bit
									bit=15-(cx%16);

									// Is mask set for this pixel?
									if (object->image_mask[0][(cy*width)+(cx>>4)]&(1<<bit))
										return object;
								}
								else
				*/

				return object;
			}

			// Checking label?
			else if (flags & BDGOF_CHECK_LABEL)
			{
				// See if point is within label
				if (x >= object->show_rect.MinX && x <= object->show_rect.MaxX && y > object->image_rect.MaxY &&
					y <= object->show_rect.MaxY)
				{
					// Use object
					return object;
				}
			}
		}
	}

	return 0;
}

// Find an icon in a rectangle
BackdropObject *backdrop_icon_in_rect(BackdropInfo *info, struct Rectangle *testrect)
{
	BackdropObject *object;

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Is object position ok?
		if (!(object->flags & BDOF_NO_POSITION))
		{
			// See if rectangles intersect
			if (geo_box_intersect(&object->show_rect, testrect))
			{
				return object;
			}
		}
	}

	return 0;
}

// Find an icon in a rectangle
BackdropObject *backdrop_icon_in_rect_full(BackdropInfo *info, struct Rectangle *testrect)
{
	BackdropObject *object;

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Is object position ok?
		if (!(object->flags & BDOF_NO_POSITION))
		{
			// See if rectangles intersect
			if (geo_box_intersect(&object->full_size, testrect))
			{
				return object;
			}
		}
	}

	return 0;
}

// Fix selection count
void backdrop_fix_count(BackdropInfo *info, BOOL update)
{
	Lister *lister;
	DirBuffer *buffer;
	BackdropObject *object;

	// Get lister pointer
	if (!(lister = info->lister))
		return;
	buffer = lister->cur_buffer;

	// Lock list
	lock_listlock(&info->objects, 0);

	// Clear selection counts
	buffer->buf_TotalEntries[1] = 0;
	buffer->buf_TotalFiles[1] = 0;
	buffer->buf_TotalDirs[1] = 0;
	buffer->buf_TotalBytes[1] = 0;
	buffer->buf_SelectedFiles[1] = 0;
	buffer->buf_SelectedDirs[1] = 0;
	buffer->buf_SelectedBytes[1] = 0;

	// Go through list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Increment count
		++buffer->buf_TotalEntries[1];

		// Drawer?
		if (object->icon && (object->icon->do_Type == WBDRAWER || object->icon->do_Type == WBGARBAGE))
		{
			++buffer->buf_TotalDirs[1];
		}

		// File
		else
		{
			++buffer->buf_TotalFiles[1];
			buffer->buf_TotalBytes[1] += object->size;
		}

		// Is object selected?
		if (object->state)
		{
			// Drawer?
			if (object->icon->do_Type == WBDRAWER || object->icon->do_Type == WBGARBAGE)
			{
				++buffer->buf_SelectedDirs[1];
			}

			// File
			else
			{
				++buffer->buf_SelectedFiles[1];
				buffer->buf_SelectedBytes[1] += object->size;
			}
		}
	}

	// Unlock list
	unlock_listlock(&info->objects);

	// Show info
	if (update)
		IPC_Command(lister->ipc, LISTER_SHOW_INFO, 0, 0, 0, 0);
}

// Find a disk object
BackdropObject *backdrop_find_disk(BackdropInfo *info, char *path)
{
	BPTR lock;
	BackdropObject *object;
	struct List *search;
	D_S(struct InfoData, data)
	struct DosList *dl;
	char name[80];

	// Lock path
	if (!(lock = Lock(path, ACCESS_READ)))
	{
		// Try for icon
		strcpy(info->buffer, path);
		strcat(info->buffer, ".info");
		if (!(lock = Lock(info->buffer, ACCESS_READ)))
			return 0;
	}

	// Get disk information
	if (!(Info(lock, data)) || !(dl = (struct DosList *)BADDR(data->id_VolumeNode)))
	{
		UnLock(lock);
		return 0;
	}

	// Get disk name
	BtoCStr(dl->dol_Name, name, 80);

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Look for volume by name
	search = &info->objects.list;
	while ((object = (BackdropObject *)FindNameI(search, name)))
	{
		// Disk?
		if (object->type == BDO_DISK)
		{
			// Compare dates
			if (CompareDates(&dl->dol_misc.dol_volume.dol_VolumeDate, &object->date) == 0)
				break;
		}

		// Keep searching from this object
		search = (struct List *)object;
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Unlock lock
	UnLock(lock);

	return object;
}

// Get next object
BackdropObject *backdrop_next_object(BackdropInfo *info, BackdropObject *object, BackdropObject *only_one)
{
	// Had an object, was it the only one?
	if (object && object == only_one)
		return 0;

	// No object yet?
	if (!object)
	{
		// Get first object
		object = (BackdropObject *)info->objects.list.lh_Head;
	}

	// Get next object
	else
		object = (BackdropObject *)object->node.ln_Succ;

	// If list is empty, return 0
	if (!object->node.ln_Succ)
		return 0;

	// Loop until successful
	while (object->node.ln_Succ)
	{
		// If object is ok, return it
		if (object->icon && ((only_one && only_one == object) || (!only_one && object->state)))
			return object;

		// Get next object
		object = (BackdropObject *)object->node.ln_Succ;
	}

	return 0;
}
