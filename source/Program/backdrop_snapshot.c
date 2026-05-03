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

// Snapshot objects
void backdrop_snapshot(BackdropInfo *info, BOOL unsnapshot, BOOL all, BackdropObject *icon)
{
	BackdropObject *object;
	Lister *lister;
	IPCData *ipc;
	BPTR old, lock;
	short count = 0, beep = 0;
#ifdef DISTINCT_OK
	BOOL save_env = 0;
#endif
	BOOL savepos = 0;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Lock lister list
	lock_listlock(&GUI->lister_list, 0);

	// Lock group list
	lock_listlock(&GUI->group_list, 0);

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		BOOL ok = 0;

		// Icon supplied?
		if (icon)
		{
			// Does it match?
			if (object != icon || !object->icon)
				continue;
		}

		// Is object selected?
		else if ((!object->state && !all) || !object->icon)
			continue;
		beep = 1;

		// Appicon?
		if (object->type == BDO_APP_ICON)
		{
			// Does icon support snapshot itself?
			if (WB_AppIconFlags((struct AppIcon *)object->misc_data) & APPENTF_SNAPSHOT)
			{
				backdrop_appicon_message(object, (unsnapshot) ? BAPPF_UNSNAPSHOT : 0);
				++count;
			}

			// Otherwise, fake snapshot
			else if (*object->name)
				ok = 1;
		}

		// Otherwise, is it ok to snapshot?
		else if (object->type != BDO_BAD_DISK)
			ok = 1;

		// Ok to snapshot?
		if (ok)
		{
			short x, y;

			// UnShapshot?
			if (unsnapshot)
			{
				// Set "no position" position
				x = -1;
				y = -1;
			}

			// Snapshot
			else
			{
				// Drawer?
				if (object->icon->do_DrawerData)
				{
					// Group icon?
					if (object->type == BDO_GROUP)
					{
						GroupData *group;

						// See if group is open
						if ((group = backdrop_find_group(object)))
						{
							// Update window position
							*((struct IBox *)&object->icon->do_DrawerData->dd_NewWindow.LeftEdge) = group->dimensions;
						}
					}

					// Otherwise go through listers
					else
						for (ipc = (IPCData *)GUI->lister_list.list.lh_Head; ipc->node.mln_Succ;
							 ipc = (IPCData *)ipc->node.mln_Succ)
						{
							// Get lister
							lister = IPCDATA(ipc);

							// Is lister from this icon?
							if (lister->backdrop == object)
							{
								// Update lister
								PositionUpdate(lister, POSUPF_SAVE);
								break;
							}
						}
				}

				// Get position
				x = object->pos.Left;
				y = object->pos.Top;
			}

			// AppIcon?
			if (object->type == BDO_APP_ICON)
			{
				leftout_record *left;

				// Lock position list
				lock_listlock(&GUI->positions, TRUE);

				// Look for entry for icon
				for (left = (leftout_record *)&GUI->positions.list.lh_Head; left->node.ln_Succ;
					 left = (leftout_record *)left->node.ln_Succ)
				{
					// Leftout?
					if (left->node.ln_Type == PTYPE_APPICON)
					{
						// Match this icon?
						if (strcmp(left->icon_label, object->name) == 0)
							break;
					}
				}

				// Didn't find one?
				if (!(left->node.ln_Succ) && !unsnapshot)
				{
					// Create a new entry
					if ((left = AllocMemH(GUI->position_memory, sizeof(leftout_record))))
					{
						// Fill out entry
						stccpy(left->icon_label, object->name, sizeof(left->icon_label));
						left->node.ln_Name = left->name;
						left->node.ln_Type = PTYPE_APPICON;

						// Add to list
						AddTail((struct List *)&GUI->positions, (struct Node *)left);
					}
				}

				// Got one?
				if (left->node.ln_Succ)
				{
					// Unsnapshot?
					if (unsnapshot)
					{
						// Remove entry and free it
						Remove((struct Node *)left);
						FreeMemH(left);
					}

					// Update entry
					else
					{
						ULONG iflags;

						// Adjust for borders?
						if (!((iflags = GetIconFlags(object->icon)) & ICONF_BORDER_OFF) &&
							(!(environment->env->desktop_flags & DESKTOPF_NO_BORDERS) || (iflags & ICONF_BORDER_ON)))
						{
							// Shift back by border size
							x -= ICON_BORDER_X;
							y -= ICON_BORDER_Y_TOP;
						}

						// Save position
						left->icon_x = x;
						left->icon_y = y;
						left->flags &= ~LEFTOUTF_NO_POSITION;
					}

					// Set flag to save positions
					savepos = 1;
				}

				// Can't snapshot for some reason
				else if (!unsnapshot)
					beep = 1;

				// Unlock position list
				unlock_listlock(&GUI->positions);
			}

			// Left-out on desktop?
			else if (object->type == BDO_LEFT_OUT && !(object->flags & BDOF_DESKTOP_FOLDER) &&
					 info->flags & BDIF_MAIN_DESKTOP)
			{
				leftout_record *left;

				// Lock position list
				lock_listlock(&GUI->positions, 0);

				// Look for entry for icon
				for (left = (leftout_record *)&GUI->positions.list.lh_Head; left->node.ln_Succ;
					 left = (leftout_record *)left->node.ln_Succ)
				{
					// Leftout?
					if (left->node.ln_Type == PTYPE_LEFTOUT)
					{
						// Match this icon?
						if (object->misc_data == (ULONG)left)
							break;
					}
				}

				// Got one?
				if (left->node.ln_Succ)
				{
					// Save position
					left->icon_x = x;
					left->icon_y = y;
					left->flags &= ~LEFTOUTF_NO_POSITION;

					// Set flag to save positions
					savepos = 1;
				}

				// Must be temporary
				else
					beep = 1;

				// Unlock position list
				unlock_listlock(&GUI->positions);
			}

#ifdef DISTINCT_OK
			// Distinct icon positions?
			else if (environment->env->desktop_flags & DESKTOPF_DISTINCT && info->flags & BDIF_MAIN_DESKTOP)
			{
				// Snapshot icon for this environment
				desktop_snapshot_icon(object, x, y);
				save_env = 1;
			}
#endif

			// Group icon?
			else if (info->flags & BDIF_GROUP)
			{
				// Snapshot in group
				group_snapshot_icon(info, object, x, y);
			}

			// Save to icon
			else
			{
				// Using Workbench positions?
				if (environment->env->display_options & DISPOPTF_ICON_POS)
				{
					// Set position
					object->icon->do_CurrentX = (unsnapshot) ? NO_ICON_POSITION : x - WBICONMAGIC_X;
					object->icon->do_CurrentY = (unsnapshot) ? NO_ICON_POSITION : y - WBICONMAGIC_Y;

					// Removing Opus positions?
					if (environment->env->display_options & DISPOPTF_REMOPUSPOS)
					{
						// Remove Opus position from icon
						SetIconFlags(object->icon, GetIconFlags(object->icon) & ~ICONF_POSITION_OK);
					}
				}

				// Using Opus positions
				else
				{
					// Set "position ok" flag
					SetIconFlags(object->icon, GetIconFlags(object->icon) | ICONF_POSITION_OK);

					// Update icon position
					SetIconPosition(object->icon, x, y);
				}

				// Get icon lock
				if ((lock = backdrop_icon_lock(object)))
				{
					// Change directory
					old = CurrentDir(lock);

					// Save this object out
					if (!(WriteIcon((object->icon->do_Type == WBDISK) ? "Disk" : object->name, object->icon)))
						beep = 1;

					// Success; is this in a lister?
					else if (info->lister)
					{
						char name[256];

						// Get icon name
						strcpy(name, (object->icon->do_Type == WBDISK) ? "Disk" : object->name);
						strcat(name, ".info");

						// Reload the file
						rexx_lister_reload_file(info->lister, name);
					}

					// Restore directory
					CurrentDir(old);
					UnLock(lock);
				}
			}

			++count;
		}

		// Icon supplied?
		if (icon)
			break;
	}

	// Failed to do anything?
	if (!count && beep)
		DisplayBeep(info->window->WScreen);

	// Unlock group list
	unlock_listlock(&GUI->group_list);

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Unlock lister list
	unlock_listlock(&GUI->lister_list);

#ifdef DISTINCT_OK
	// Save environment file?
	if (save_env && environment->env->flags & ENVF_CHANGED)
	{
		// Save environment
		if (environment_save(environment, environment->path, 0, 0))
			environment->env->flags &= ~ENVF_CHANGED;
	}
#endif

	// Save positions?
	if (savepos)
	{
		// Lock list
		lock_listlock(&GUI->positions, 0);

		// Save list
		SavePositions(&GUI->positions.list, GUI->position_name);

		// Unlock list
		unlock_listlock(&GUI->positions);
	}

	// If this is a lister, update its datestamp
	if (info->lister)
		update_buffer_stamp(info->lister);
}
