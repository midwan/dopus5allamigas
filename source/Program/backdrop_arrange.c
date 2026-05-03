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

// Position an icon
void backdrop_icon_position(BackdropInfo *info, BackdropObject *icon, short x, short y)
{
	// Set position
	icon->pos.Left = x;
	icon->pos.Top = y;
	icon->flags &= ~BDOF_NO_POSITION;

	// If a backdrop window, check if object off screen now
	if (info->window->Flags & WFLG_BACKDROP)
	{
		// Do bounds check, adjust position if necessary
		if (icon->pos.Left + icon->pos.Width + 4 > info->window->Width)
			icon->pos.Left = info->window->Width - icon->pos.Width - 4;
		if (icon->pos.Left < 4)
			icon->pos.Left = 4;
		if (icon->pos.Top + icon->pos.Height + 3 > info->window->Height)
			icon->pos.Top = info->window->Height - icon->pos.Height - 3;
		if (icon->pos.Top < 3)
			icon->pos.Top = 3;
	}
}

// Clean up object positions
BOOL backdrop_cleanup(BackdropInfo *info, short type, UWORD flags)
{
	struct Rectangle rect, *rect_ptr = 0;
	BOOL align = 0;

	// Check list is not empty
	if (IsListEmpty(&info->objects.list))
		return 0;

	// If flag is set, see if control is down; if so, we want to align
	if (flags & CLEANUPF_ALIGN_OK && PeekQualifier() & IEQUALIFIER_CONTROL)
		align = 1;

	// Lock backdrop list
	lock_listlock(&info->objects, 1);

	// Sort objects according to type
	backdrop_sort_objects(info, type, align);

	// Aligning?
	if (align)
	{
		BackdropObject *object, *last = 0;

		// First icon must be selected
		object = (BackdropObject *)info->objects.list.lh_Head;
		if (object->state)
		{
			// Get rectangle to size within; top-left is first icon
			rect.MinX = object->show_rect.MinX;	 // objects are centered in show area
			rect.MinY = object->pos.Top;

			// Find last selected icon
			for (; object->node.ln_Succ && object->state;
				 last = object, object = (BackdropObject *)object->node.ln_Succ)
				;

			// Got last?
			if (last)
			{
				// Bottom-right is last icon
				rect.MaxX = last->pos.Left + last->pos.Width - 1;
				rect.MaxY = last->pos.Top + last->pos.Height - 1;
			}

			// Set pointer
			rect_ptr = &rect;
		}
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Cleanup objects
	if (backdrop_cleanup_objects(info, rect_ptr, flags) && !(flags & CLEANUPF_CHECK_POS))
	{
		// Show icons
		backdrop_show_objects(info, BDSF_CLEAR | BDSF_RECALC);
		return 1;
	}

	return 0;
}

// Cleanup backdrop objects
BOOL backdrop_cleanup_objects(BackdropInfo *info, struct Rectangle *rect, UWORD flags)
{
	BackdropObject *object, *col_start = 0, *last_icon = 0;
	long col_width = 0, icon_left = 0;
	long col_height = 0, icon_top = 0;
	long space_x, start_x, start_y, off_x, off_y;
	short icon_width = 0, icon_height = 0;
	BOOL align = 0, vert = 0, ret = 0, retry = 0;

	// Valid list and window?
	if (IsListEmpty(&info->objects.list) || !info->window)
		return 0;

	// flags&=~CLEANUPF_CHECK_POS;

	// No rectangle supplied?
	if (!rect)
	{
		rect = &info->size;
		start_x = CLEANUP_START_X;
		start_y = CLEANUP_START_Y;
	}

	// Otherwise, we're aligning
	else
	{
		align = 1;
		start_x = rect->MinX;
		start_y = rect->MinY;

		// Vertical aligning?
		if (RECTHEIGHT(rect) > RECTWIDTH(rect))
			vert = 1;
	}

	// Get icon offset
	off_x = info->size.MinX - info->offset_x;
	off_y = info->size.MinY - info->offset_y;

	// Check coordinates against grid
	backdrop_check_grid(&start_x, &start_y);

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Get x space (for screen ratio)
	space_x = GUI->icon_space_x;
	if (GUI->screen_info & SCRI_LORES)
		space_x <<= 1;

	// Vertical?
	if (vert)
	{
		// Get top and width of each column
		icon_top = start_y;
		col_width = RECTWIDTH(rect);
	}

	// Horizontal
	else
	{
		// Get left and height of each column
		icon_left = start_x;
		col_height = RECTHEIGHT(rect);
	}

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		short end_col = 0;

		// Check object isn't locked
		if (object->flags & BDOF_LOCKED)
			continue;

		// If checking existing positions, skip objects that have a position
		if (flags & CLEANUPF_CHECK_POS && !(object->flags & BDOF_NO_POSITION))
			continue;

		// If aligning, break when we reach an unselected icon
		if (!align || object->state)
		{
			// New column?
			if (!col_start)
			{
				// Store icon at head of column
				col_start = object;

				// Vertical?
				if (vert)
				{
					// Initialise
					icon_left = start_x;
					col_height = 0;
				}

				// Horizontal
				else
				{
					// Initialise
					icon_top = start_y;
					col_width = 0;
				}
			}

			// Get icon size if we haven't already
			if (object != last_icon)
			{
				backdrop_icon_size(info, object, &icon_width, &icon_height);
				last_icon = object;
			}

			// Vertical?
			if (vert)
			{
				// Check if icon will fit horizontally
				if (icon_left + icon_width > col_width + space_x)
				{
					// If this is the first icon in the column, squeeze it in anyway
					if (col_height > 0 && !retry)
						end_col = 1;
				}
			}

			// Horizontal
			else
			{
				// Check if icon will fit vertically
				if (icon_top + icon_height > col_height + GUI->icon_space_y)
				{
					// If this is the first icon in the column, squeeze it in anyway
					if (col_width > 0 && !retry)
						end_col = 1;
				}
			}

			// Otherwise, icon goes in this column
			if (!end_col)
			{
				// Check for largest icon
				if (vert)
				{
					if (icon_height > col_height)
						col_height = icon_height;
				}
				else
				{
					if (icon_width > col_width)
						col_width = icon_width;
				}

				// Checking existion positions?
				if (flags & CLEANUPF_CHECK_POS)
				{
					struct Rectangle rect;
					BackdropObject *icon;

					// Build icon rectangle
					rect.MinX = icon_left + off_x;
					rect.MinY = icon_top + off_y;
					rect.MaxX = rect.MinX + ((vert) ? icon_width : col_width) - 1;
					rect.MaxY = rect.MinY + ((vert) ? col_height : icon_height) - 1;

					// See if there's an icon there already
					if ((icon = backdrop_icon_in_rect(info, &rect)))
					{
						// Skip this position
						if (vert)
						{
							icon_left += icon_width;
							backdrop_check_grid(&icon_left, 0);
						}
						else
						{
							icon_top += icon_height + GUI->icon_space_y;
							backdrop_check_grid(0, &icon_top);
						}

						// Use this icon again
						object = (BackdropObject *)object->node.ln_Pred;
						retry = 1;
						continue;
					}
				}

				// Vertical?
				if (vert)
				{
					// Store horizontal position
					object->pos.Left = icon_left;

					// Increment left position
					icon_left += icon_width;

					// Check coordinates against grid
					backdrop_check_grid(&icon_left, 0);
				}

				// Horizontal
				else
				{
					// Store icon's vertical position
					object->pos.Top = icon_top;

					// Increment top position
					icon_top += icon_height + GUI->icon_space_y;

					// Check coordinates against grid
					backdrop_check_grid(0, &icon_top);
				}
			}
		}

		// Want to end
		else
		{
			// Set end flag
			end_col = 2;

			// Move back to last icon
			object = (BackdropObject *)object->node.ln_Pred;

			// If locked, or has a position, keep going
			while (object->flags & BDOF_LOCKED || (flags & CLEANUPF_CHECK_POS && !(object->flags & BDOF_NO_POSITION)))
			{
				// Get previous icon
				object = (BackdropObject *)object->node.ln_Pred;

				// Check for valid
				if (!object->node.ln_Pred)
					break;
			}
		}

		// Clear retry flag
		retry = 0;

		// Test for the last icon
		if (!end_col && object->node.ln_Succ->ln_Succ)
		{
			BackdropObject *test;
			for (test = (BackdropObject *)object->node.ln_Succ; test->node.ln_Succ;
				 test = (BackdropObject *)test->node.ln_Succ)
			{
				// Icon that can be used in cleanup? (not locked or with a position already)
				if (!(test->flags & BDOF_LOCKED) && (!(flags & CLEANUPF_CHECK_POS) || test->flags & BDOF_NO_POSITION))
					break;
			}
			if (!test->node.ln_Succ)
				end_col = 2;
		}

		// End of a column, or the last icon?
		if (end_col || !object->node.ln_Succ->ln_Succ)
		{
			BackdropObject *icon;

			// Go from column start to this icon
			for (icon = col_start; icon->node.ln_Succ; icon = (BackdropObject *)icon->node.ln_Succ)
			{
				// Check object isn't locked, or doesn't already have a position
				if (!(icon->flags & BDOF_LOCKED) && (!(flags & CLEANUPF_CHECK_POS) || icon->flags & BDOF_NO_POSITION))
				{
					// Vertical?
					if (vert)
					{
						// Center within row
						icon->pos.Top = icon_top + ((col_height - icon->pos.Height) >> 1);
					}

					// Horizontal
					else
					{
						// Center within column
						icon->pos.Left = icon_left + ((col_width - icon->pos.Width) >> 1);
					}

					// Now have a valid position
					icon->flags |= BDOF_TEMP_FLAG;
				}

				// Break if the last icon in column
				if (icon == object)
					break;
			}

			// Finished?
			if (end_col == 2)
				break;

			// Clear column pointer
			col_start = 0;

			// Vertical?
			if (vert)
			{
				// Bump column/row position
				icon_top += col_height + GUI->icon_space_y;
			}

			// Horizontal
			else
			{
				// Bump column/row position
				icon_left += col_width + space_x;
			}

			// Check coordinates against grid
			backdrop_check_grid(&icon_left, &icon_top);

			// If an icon didn't fit, use it again next time
			if (end_col)
				object = (BackdropObject *)object->node.ln_Pred;
		}
	}

	// Go through entire list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Temporary flag set?
		if (object->flags & BDOF_TEMP_FLAG)
		{
			// Clear temp flag and set (clear) pos flag
			object->flags &= ~(BDOF_TEMP_FLAG | BDOF_NO_POSITION);
			ret = 1;
		}
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);
	return ret;
}

// Sort objects in position order
void backdrop_sort_objects(BackdropInfo *info, short type, BOOL align)
{
	BackdropObject *object;
	struct List temp;

	// Check list is not empty
	if (IsListEmpty(&info->objects.list))
		return;

	// Set flag to indicate sort method
	if (type == BSORT_HORIZ)
		info->flags |= BDIF_LAST_SORT_HORIZ;
	else
		info->flags &= ~BDIF_LAST_SORT_HORIZ;

	// Initialise temporary list
	NewList(&temp);

	// Lock filetype lists if sorting by type
	if (type == BSORT_TYPE)
		lock_listlock(&GUI->filetypes, FALSE);

	// Go through objects
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;)
	{
		BackdropObject *sort_object, *before_object = 0;
		BackdropObject *next = (BackdropObject *)object->node.ln_Succ;

		// If we're aligning, and object is not selected, skip over it
		if (align && !object->state)
		{
			// Get next and continue
			object = next;
			continue;
		}

		// Sort by position?
		if (type < BSORT_NAME)
		{
			unsigned short left, top;

			// Valid position?
			if (!(object->flags & BDOF_NO_POSITION))
			{
				// Sorting with a vertical weighting?
				if (type == BSORT_NORM)
				{
					// Calculate object value from position (center of object)
					left = (object->pos.Left + (object->pos.Width >> 1)) >> 4;
					top = object->pos.Top + (object->pos.Height >> 1);
					object->value = (left << 16) | top;
				}

				// Horizontal weighting
				else
				{
					// Calculate object value from position (center of object)
					top = (object->pos.Top + (object->pos.Height >> 1)) >> 4;
					left = object->pos.Left + (object->pos.Width >> 1);
					object->value = (top << 16) | left;
				}
			}

			// Nope
			else
				object->value = 0xffffffff;
		}

		// Go through temporary list
		for (sort_object = (BackdropObject *)temp.lh_Head; sort_object->node.ln_Succ;
			 sort_object = (BackdropObject *)sort_object->node.ln_Succ)
		{
			short sort_type;

			// Initial sort type
			sort_type = type;

			// Sort by date?
			if (type == BSORT_DATE)
			{
				short res;

				// Compare dates
				if ((res = CompareDates(&object->date, &sort_object->date)) > 0)
					break;
				else if (res == 0)
					sort_type = BSORT_NAME;
			}

			// Sort by size
			else if (type == BSORT_SIZE)
			{
				// Compare sizes
				if (object->size < sort_object->size)
					break;
				else if (object->size == sort_object->size)
					sort_type = BSORT_NAME;
			}

			// Sort by type
			else if (type == BSORT_TYPE)
			{
				Cfg_Filetype *type1, *type2;
				short res;

				// Get filetypes
				type1 = backdrop_get_filetype(info, object);
				type2 = backdrop_get_filetype(info, sort_object);

				// Got filetypes?
				if (type1 && type2)
				{
					if ((res = stricmp(type1->type.name, type2->type.name)) < 0)
						break;
					else if (res == 0)
						sort_type = BSORT_NAME;
				}
				else
					sort_type = BSORT_NAME;
			}

			// Sort by name?
			if (sort_type == BSORT_NAME)
			{
				char *name1, *name2;

				// Get name pointers
				name1 = (object->flags & BDOF_CUSTOM_LABEL) ? object->device_name : object->name;
				name2 = (sort_object->flags & BDOF_CUSTOM_LABEL) ? sort_object->device_name : sort_object->name;

				// Compare names
				if (stricmp(name1, name2) < 0)
					break;
			}

			// Sort by position
			else if (sort_type < BSORT_NAME && object->value < sort_object->value)
				break;

			// Remember previous
			before_object = sort_object;
		}

		// If we're aligning, we need to remove from main list
		if (align)
			Remove((struct Node *)object);

		// Valid position?
		if (sort_object->node.ln_Succ)
		{
			// Object to add after?
			if (before_object)
			{
				// Add before this object
				Insert(&temp, (struct Node *)object, (struct Node *)before_object);
			}

			// Add to head
			else
				AddHead(&temp, (struct Node *)object);
		}

		// Otherwise add to end
		else
			AddTail(&temp, (struct Node *)object);

		// Get next
		object = next;
	}

	// Unlock filetype lists if sorting by type
	if (type == BSORT_TYPE)
		unlock_listlock(&GUI->filetypes);

	// Aligning?
	if (align)
	{
		struct Node *node, *next;

		// Move from temp list into main list manually
		for (node = temp.lh_TailPred; node->ln_Pred; node = next)
		{
			// Cache next
			next = node->ln_Pred;

			// Add to main list
			AddHead(&info->objects.list, node);
		}
	}

	// Can copy pointers
	else
	{
		// Copy from temp list back into backdrop list
		info->objects.list = temp;
		info->objects.list.lh_Head->ln_Pred = (struct Node *)&info->objects.list.lh_Head;
		info->objects.list.lh_TailPred->ln_Succ = (struct Node *)&info->objects.list.lh_Tail;
	}
}

// Make sure that a given icon is visible
void backdrop_make_visible(BackdropInfo *info, BackdropObject *object)
{
	struct Rectangle rect;
	short scroll_x = 0, scroll_y = 0;

	// Non-backdrop window open?
	if (!info->window || info->window->Flags & WFLG_BACKDROP ||
		(info->lister && info->lister->more_flags & LISTERF_TITLEBARRED))
		return;

	// Build visible rectangle
	rect.MinX = info->offset_x;
	rect.MinY = info->offset_y;
	rect.MaxX = rect.MinX + RECTWIDTH(&info->size) - 1;
	rect.MaxY = rect.MinY + RECTHEIGHT(&info->size) - 1;

	// Icon off to the right?
	if (object->full_size.MaxX > rect.MaxX)
		scroll_x = (object->full_size.MaxX - rect.MaxX) + 8;

	// Or off to the left?
	else if (object->full_size.MinX < rect.MinX)
		scroll_x = (object->full_size.MinX - rect.MinX) - 8;

	// Icon off to the bottom?
	if (object->full_size.MaxY > rect.MaxY)
		scroll_y = (object->full_size.MaxY - rect.MaxY) + 8;

	// Or off to the top?
	else if (object->full_size.MinY < rect.MinY)
		scroll_y = (object->full_size.MinY - rect.MinY) - 8;

	// Need to scroll?
	if (scroll_x || scroll_y)
	{
		long oldpos;
		short which;

		// Get old horizontal position
		GetAttr(PGA_Top, info->horiz_scroller, (ULONG *)&oldpos);

		// Add scroll factor
		oldpos += scroll_x;
		if (oldpos < 0)
			oldpos = 0;

		// Update gadget
		SetGadgetAttrs(info->horiz_scroller, info->window, 0, PGA_Top, oldpos, TAG_END);

		// Get old vertical position
		GetAttr(PGA_Top, info->vert_scroller, (ULONG *)&oldpos);

		// Add scroll factor
		oldpos += scroll_y;
		if (oldpos < 0)
			oldpos = 0;

		// Update gadget
		SetGadgetAttrs(info->vert_scroller, info->window, 0, PGA_Top, oldpos, TAG_END);

		// Sliders that need changing
		which = 0;
		if (scroll_x)
			which |= SLIDER_HORZ;
		if (scroll_y)
			which |= SLIDER_VERT;

		// Update display
		backdrop_pos_slider(info, which);
	}
}

// Get the size of an icon (including text)
void backdrop_icon_size(BackdropInfo *info, BackdropObject *object, short *width, short *height)
{
	struct TextExtent extent;
	char *name, namebuf[40];
	short w, border = 0;

	// Bad disk?
	if (object->type == BDO_BAD_DISK)
	{
		// Get bad disk name
		backdrop_bad_disk_name(object, namebuf);
		name = namebuf;
	}

	// Otherwise, get name pointer from icon
	else
		name = (object->flags & BDOF_CUSTOM_LABEL) ? object->device_name : object->name;

	// Get icon width
	w = object->pos.Width;
	if (backdrop_icon_border(object))
	{
		w += ICON_BORDER_X * 2;
		border = 1;
	}

	// Initialise extent
	extent.te_Height = 8;
	extent.te_Width = strlen(name) * 8;

	// Need to get text size?
	if (info->window && (width || height))
	{
		short max_width = 0;

		// Splitting long labels?
		if (environment->env->desktop_flags & DESKTOPF_SPLIT_LABELS)
		{
			// Calculate maximum text width (minimum 6 chars)
			max_width = w + (w >> 1);
			if (max_width < info->rp.Font->tf_XSize * 6)
				max_width = info->rp.Font->tf_XSize * 6;
		}

		// Get text pixel size
		TextMultiLine(&info->rp, name, -1, max_width, TMLF_CENTER | TMLF_EXTENT, &extent);
	}

	// Want height?
	if (height)
	{
		// Get icon height
		*height = object->pos.Height;
		if (border)
			*height += ICON_BORDER_Y_TOP + ICON_BORDER_Y_BOTTOM - 2;
		if (!(object->flags & BDOF_NO_LABEL))
			*height += extent.te_Height + ICON_LABEL_SPACE;
	}

	// Want width?
	if (width)
	{
		// Get icon width
		*width = w;

		// Icon label?
		if (!(object->flags & BDOF_NO_LABEL))
		{
			// See if text is bigger than image
			if (extent.te_Width > *width)
				*width = extent.te_Width;
		}
	}
}

// Get filetype for an object
Cfg_Filetype *backdrop_get_filetype(BackdropInfo *info, BackdropObject *object)
{
	Cfg_Filetype *filetype = 0;

	// Object has filetype set?
	if (object->filetype)
	{
		// Check filetype is still in list
		if (filetype_check(object->filetype))
		{
			// Use cached pointer
			return object->filetype;
		}
	}

	// Is backdrop attached to a lister?
	if (info->lister)
	{
		DirEntry *entry;

		// Look for entry in lister
		if ((entry = find_entry(&info->lister->cur_buffer->entry_list, object->name, 0, DWF_CASE)))
		{
			char *ptr;

			// Does entry have a description?
			if ((ptr = (char *)GetTagData(DE_Filetype, 0, entry->de_Tags)))
			{
				// Get filetype from list
				filetype = filetype_find(ptr, 0);
			}
		}
	}

	// Don't have a filetype yet?
	if (!filetype)
	{
		BPTR lock, old;

		// Get icon lock
		if ((lock = backdrop_icon_lock(object)))
		{
			// Change current directory
			old = CurrentDir(lock);

			// Match filetype
			filetype = filetype_identify(object->name, FTTYPE_ANY, 0, 0);

			// Restore directory, free lock
			UnLock(CurrentDir(old));
		}
	}

	// Save filetype pointer
	object->filetype = filetype;

	// Return filetype pointer
	return filetype;
}

// Get a 'fake' show rectangle
void backdrop_show_rect(BackdropInfo *info, BackdropObject *object, short x, short y)
{
	short width, height;

	// Get icon size
	backdrop_icon_size(info, object, &width, &height);

	// Fake show rectangle
	object->show_rect.MinX = x - ((width - object->pos.Width) >> 1);
	object->show_rect.MinY = y;
	object->show_rect.MaxX = object->show_rect.MinX + width - 1;
	object->show_rect.MaxY = object->show_rect.MinY + height - 1;
}

// Check grid coordinates
void backdrop_check_grid(long *x, long *y)
{
	if (x && GUI->icon_grid_x > 1 && *x > 0)
		*x = ((*x + GUI->icon_grid_x - 1) / GUI->icon_grid_x) * GUI->icon_grid_x;
	if (y && GUI->icon_grid_y > 1 && *y > 0)
		*y = ((*y + GUI->icon_grid_y - 1) / GUI->icon_grid_y) * GUI->icon_grid_y;
}

// Line-up backdrop objects
void backdrop_lineup_objects(BackdropInfo *info)
{
	BackdropObject *object;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		long x, y;

		// Check object isn't locked
		if (object->flags & BDOF_LOCKED)
			continue;

		// Get position
		x = object->pos.Left;
		y = object->pos.Top;

		// Check coordinates against grid
		backdrop_check_grid(&x, &y);

		// Shift position
		x -= GUI->icon_grid_x - CLEANUP_START_X;
		y -= GUI->icon_grid_y - CLEANUP_START_Y;

		// Store new position
		object->pos.Left = x;
		object->pos.Top = y;
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Show objects
	backdrop_show_objects(info, BDSF_CLEAR | BDSF_RECALC);
}
