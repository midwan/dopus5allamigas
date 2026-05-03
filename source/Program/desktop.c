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

// Find item in the hidden list
BOOL desktop_find_hidden(char *name, BOOL bad)
{
	Cfg_Desktop *desk;

	// No name?
	if (!name || !*name)
		return 0;

	// Lock desktop list
	GetSemaphore(&environment->desktop_lock, SEMF_SHARED, 0);

	// Go through desktop list
	for (desk = (Cfg_Desktop *)environment->desktop.mlh_Head; desk->node.mln_Succ;
		 desk = (Cfg_Desktop *)desk->node.mln_Succ)
	{
		// Hidden drive?
		if (desk->data.dt_Type == ((bad) ? DESKTOP_HIDE_BAD : DESKTOP_HIDE))
		{
			// Compare name
			if (strcmp(name, (char *)&desk->data.dt_Data) == 0)
				break;
		}
	}

	// Unlock desktop list
	FreeSemaphore(&environment->desktop_lock);

	// Found?
	return (BOOL)((desk->node.mln_Succ) ? TRUE : FALSE);
}

// Snapshot an icon for the desktop
Cfg_Desktop *desktop_snapshot_icon(BackdropObject *icon, short x, short y)
{
	Cfg_Desktop *desk;
	char name[256];

	// Get icon path
	if (!(desktop_icon_path(icon, name, 256, 0)))
		return 0;

	// Lock desktop list
	GetSemaphore(&environment->desktop_lock, SEMF_EXCLUSIVE, 0);

	// See if icon already has an entry
	if (!(desk = desktop_find_icon(name, 0)))
	{
		// Allocate new entry
		if ((desk = AllocMemH(environment->desktop_memory, sizeof(Cfg_Desktop) + strlen(name) + 1)))
		{
			// Fill in entry
			desk->data.dt_Type = DESKTOP_ICON;
			desk->data.dt_Size = sizeof(CFG_DESK) + strlen(name) + 1;

			// Copy name
			strcpy((char *)(desk + 1), name);

			// Add to list
			AddTail((struct List *)&environment->desktop, (struct Node *)desk);
		}
	}

	// Got entry?
	if (desk)
	{
		// Store position
		desk->data.dt_Data = (x << 16) | y;

		// Set change flag
		environment->env->flags |= ENVF_CHANGED;
	}

	// Unlock desktop list
	FreeSemaphore(&environment->desktop_lock);

	return desk;
}

// See if an icon has been snapshotted for the desktop
Cfg_Desktop *desktop_find_icon(char *name, ULONG *pos)
{
	Cfg_Desktop *desk;

	// Lock desktop list
	GetSemaphore(&environment->desktop_lock, SEMF_SHARED, 0);

	// Go through list
	for (desk = (Cfg_Desktop *)environment->desktop.mlh_Head; desk->node.mln_Succ;
		 desk = (Cfg_Desktop *)desk->node.mln_Succ)
	{
		// Icon?
		if (desk->data.dt_Type == DESKTOP_ICON)
		{
			// Match name
			if (stricmp(name, (char *)(desk + 1)) == 0)
				break;
		}
	}

	// Not found?
	if (!desk->node.mln_Succ)
		desk = 0;

	// Store position
	else if (pos)
		*pos = desk->data.dt_Data;

	// Unlock desktop list
	FreeSemaphore(&environment->desktop_lock);

	return desk;
}

// Get the path of an icon
BOOL desktop_icon_path(BackdropObject *icon, char *path, short len, BPTR our_lock)
{
	BPTR lock;

	// Get icon lock
	if (our_lock)
		lock = our_lock;
	else if (!(lock = backdrop_icon_lock(icon)))
		return 0;

	// Get path name
	DevNameFromLockDopus(lock, path, len);
	if (!our_lock)
		UnLock(lock);

	// File/drawer?
	if (icon->icon && icon->icon->do_Type != WBDISK)
	{
		// Add icon name
		AddPart(path, icon->name, len);
	}

	return 1;
}

// Refresh icon positions
void desktop_refresh_icons(BackdropInfo *info, BOOL show)
{
	BackdropObject *object;
	BOOL refresh = 0;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Not AppIcon?
		if (object->type != BDO_APP_ICON)
		{
			/*
						char path[256];
						BOOL set=0;

						// Want distinct positions?
						if (environment->env->desktop_flags&DESKTOPF_DISTINCT)
						{
							// Get icon path
							if (desktop_icon_path(object,path,256,0))
							{
								ULONG pos;

								// See if position has been set
								if (desktop_find_icon(path,&pos))
								{
									// Erase object
									if (show) backdrop_render_object(info,object,BRENDERF_CLEAR|BRENDERF_CLIP);

									// Store position in icon
									object->custom_pos=pos;

									// Set "custom position" flag
									object->flags|=BDOF_CUSTOM_POS;

									// Fix position
									backdrop_get_icon(info,object,GETICON_POS_ONLY|GETICON_KEEP);
									set=1;
									refresh=1;
								}
							}
						}

						// No custom position?
						if (!set)
						{
			*/
			// Had a custom position?
			if (object->flags & BDOF_CUSTOM_POS)
			{
				// Erase object
				if (show)
					backdrop_render_object(info, object, BRENDERF_CLEAR | BRENDERF_CLIP);

				// Clear flag
				object->flags &= ~BDOF_CUSTOM_POS;

				// Get new position
				backdrop_get_icon(info, object, GETICON_POS_ONLY | GETICON_KEEP);
				refresh = 1;
			}
			/*
						}
			*/
		}
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Refresh display
	if (refresh && show)
	{
		// Arrange icons
		backdrop_arrange_objects(info);

		// Show desktop
		backdrop_show_objects(info, BDSF_RECALC);
	}
}

// Add item to the hidden list
BOOL desktop_add_hidden(char *name)
{
	Cfg_Desktop *desk;
	short len;

	// No name?
	if (!name || !*name)
		return 0;

	// Lock desktop list
	GetSemaphore(&environment->desktop_lock, SEMF_SHARED, 0);

	// Allocate desktop node
	if ((desk = AllocMemH(environment->desktop_memory, (len = (sizeof(Cfg_Desktop) + strlen(name))))))
	{
		// Fill out desktop node
		desk->data.dt_Type = DESKTOP_HIDE;
		desk->data.dt_Size = len;
		strcpy((char *)&desk->data.dt_Data, name);

		// Add to list
		AddTail((struct List *)&environment->desktop, (struct Node *)desk);
	}

	// Unlock desktop list
	FreeSemaphore(&environment->desktop_lock);

	return (BOOL)((desk) ? TRUE : FALSE);
}
