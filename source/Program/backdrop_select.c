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

// Select all objects
void backdrop_select_all(BackdropInfo *info, short state)
{
	BackdropObject *object;
	BOOL draw = 0;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Fast validate object before processing (use fast path for performance)
		if (!backdrop_validate_icon_fast(object)) {
			// Skip corrupted objects
			continue;
		}
		
		// Is object not selected?
		if (((state && !object->state) || (!state && object->state)) && object->icon)
		{
			// Select object
			// Select object using safe atomic operation
			backdrop_set_icon_state_safe(object, state);

			/*
						// Add to selection list
						backdrop_selection_list(info,object);
			*/
			draw = 1;
		}
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Fix selection count
	backdrop_fix_count(info, 1);

	// Fix menus for a lister
	if (info->lister)
		lister_fix_menus(info->lister, 1);

	// Show objects
	if (draw)
		backdrop_show_objects(info, 0);
}

// Select all objects within an area
void backdrop_select_area(BackdropInfo *info, short state)
{
	BackdropObject *object;

	// Dragging?
	if (GUI->flags2 & GUIF2_ICONPOS && info->flags & BDIF_MAIN_DESKTOP)
		return;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Install clip
	if (info->clip_region)
		InstallClipRegion(info->window->WLayer, info->clip_region);

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Fast validate object before processing (use fast path for performance)
		if (!backdrop_validate_icon_fast(object)) {
			// Skip corrupted objects
			continue;
		}
		
		// Valid icon?
		if (object->icon)
		{
			// Icon temporarily selected?
			if (object->state == 2)
			{
				// Turn it on?
				if (state == 1)
				{
				// Select object using safe atomic operation
				backdrop_set_icon_state_safe(object, 1);

					// Is this a tool?
					if (object->type != BDO_APP_ICON && object->icon->do_Type == WBTOOL)
					{
						// If no tools selected, remember this one
						if (!info->first_sel_tool)
							info->first_sel_tool = object;
					}
				}

				// See if it needs to be turned off
				else if (state == 0 || !(geo_box_intersect(&object->image_rect, &info->select)))
				{
				// Deselect this object using safe atomic operation
				backdrop_set_icon_state_safe(object, 0);
				backdrop_render_object(info, object, 0);
					backdrop_render_object(info, object, 0);
				}
			}

			// Currently off?
			else if (!object->state && state == 2)
			{
				// See if icon is in select area
				if (geo_box_intersect(&object->image_rect, &info->select))
				{
				// Select this object using safe atomic operation
				backdrop_set_icon_state_safe(object, 2);
				backdrop_render_object(info, object, 0);
				}
			}
		}
	}

	// Fix selection count
	backdrop_fix_count(info, 1);

	// Remove clip
	if (info->clip_region)
		InstallClipRegion(info->window->WLayer, 0);

	// Unlock backdrop list
	unlock_listlock(&info->objects);
}
