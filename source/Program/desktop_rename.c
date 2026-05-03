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

// Rename some objects
void icon_rename(IPCData *ipc, BackdropInfo *info, BackdropObject *icon)
{
	Att_List *list;
	Att_Node *node;
	BackdropObject *object;
	BOOL save_pos = 0;
	BPTR lock = 0, old = 0;

	// Group?
	if (info->flags & BDIF_GROUP)
	{
		GroupData *group;

		// Get group pointer
		group = (GroupData *)IPCDATA(info->ipc);

		// Build directory name
		lsprintf(info->buffer, "dopus5:groups/%s", group->name);

		// Lock directory
		if (!(lock = Lock(info->buffer, ACCESS_READ)))
			return;

		// Change directory
		old = CurrentDir(lock);
	}

	// Create list
	if (!(list = Att_NewList(0)))
	{
		if (lock)
			UnLock(CurrentDir(old));
		return;
	}

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Go through backdrop list (backwards, just for the hell of it)
	for (object = (BackdropObject *)info->objects.list.lh_TailPred; object->node.ln_Pred;
		 object = (BackdropObject *)object->node.ln_Pred)
	{
		// Correct type?
		if (object->type != BDO_APP_ICON && object->type != BDO_BAD_DISK && !(object->flags & BDOF_CACHE))
		{
			// Selected or supplied?
			if ((!icon && object->state) || icon == object)
			{
				// Add object to list
				Att_NewNode(
					list, (object->flags & BDOF_CUSTOM_LABEL) ? object->device_name : object->name, (ULONG)object, 0);
				if (icon)
					break;
			}
		}
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Go through list of objects
	for (node = (Att_Node *)list->list.lh_Head; node->node.ln_Succ; node = (Att_Node *)node->node.ln_Succ)
	{
		char *name, *origname;
		BackdropObject *object;
		short ret, copy = 1;
		BOOL retry, fail = 0;

		if (!(name = AllocVec(512, 0)))
			break;
		origname = name + 256;

		// Loop on retries
		do
		{
			// Clear retry flag
			retry = 0;

			// Copy name
			strcpy(name, node->node.ln_Name);

			// Build message
			lsprintf(info->buffer, GetString(&locale, MSG_RENAME_DISK), name);

			// Strip trailing colon for assigns
			if (name[(ret = (strlen(name) - 1))] == ':')
				name[ret] = 0;

			// Copy original name
			strcpy(origname, name);

			// Ask for new name
			if (!(ret = super_request_args(info->window,
										   info->buffer,
										   SRF_BUFFER | SRF_IPC | SRF_PATH_FILTER | SRF_MOUSE_POS,
										   name,
										   255,
										   ipc,
										   GetString(&locale, MSG_OKAY),
										   GetString(&locale, (node->node.ln_Succ->ln_Succ) ? MSG_SKIP : MSG_CANCEL),
										   (node->node.ln_Succ->ln_Succ) ? GetString(&locale, MSG_CANCEL) : 0,
										   0)))
			{
				// Cancel
				fail = 1;
				break;
			}

			// Ok to rename?
			else if (ret != 2)
			{
				// Lock backdrop list
				lock_listlock(&info->objects, 0);

				// Is object still valid?>
				if ((object = find_backdrop_object(info, (BackdropObject *)node->data)))
				{
					// Object in a group?
					if (info->flags & BDIF_GROUP)
					{
						// Must have custom label
						if (object->flags & BDOF_CUSTOM_LABEL)
						{
							// Try to rename object
							if (Rename(object->device_name, name))
							{
								// Store new name
								strcpy(object->device_name, name);
								ret = 1;
								copy = 0;
							}
						}
					}

					// Assign?
					else if (object->flags & BDOF_ASSIGN)
					{
						BPTR lock;

						// Lock the assign
						if ((lock = Lock(object->name, ACCESS_READ)))
						{
							// Assign the new name to it
							while (!(ret = AssignLock(name, lock)))
							{
								// Failed, display requester
								if (!(error_request(info->window,
													ERF_MOUSE,
													GetString(&locale, MSG_RENAMING),
													-1,
													object->name,
													":",
													0)))
									break;
							}

							// Success?
							if (ret)
							{
								// Delete the old assignment
								AssignLock(origname, 0);
							}

							// Otherwise free the lock
							else
								UnLock(lock);
						}
					}

					// Disk?
					else if (object->type == BDO_DISK)
					{
						// Try to relabel
						while (!(ret = OriginalRelabel(object->device_name, name)))
						{
							// Failed, display requester
							if (!(error_request(info->window,
												ERF_MOUSE,
												GetString(&locale, MSG_RENAMING),
												-1,
												object->name,
												":",
												0)))
								break;
						}

						// Success?
						if (ret)
						{
							// Pass name change through to things
							notify_disk_name_change(info, object->device_name, name);
						}
					}

					// Group, desktop folder
					else if (object->type == BDO_GROUP ||
							 (object->type == BDO_LEFT_OUT && object->flags & BDOF_DESKTOP_FOLDER))
					{
						BPTR old = 0, lock;

						// Change directory
						if ((lock = Lock(object->path, ACCESS_READ)))
							old = CurrentDir(lock);

						// Rename file
						if (!(ret = Rename(object->name, name)))
						{
							// Show error requester
							if (error_request(info->window,
											  ERF_MOUSE,
											  GetString(&locale, MSG_RENAMING),
											  -1,
											  object->name,
											  ":",
											  0))
							{
								// Set flag to retry
								retry = 1;
							}
						}

						// Success?
						if (ret)
						{
							GroupData *group;

							// Add .infos on
							strcpy(info->buffer, object->name);
							strcat(info->buffer, ".info");
							strcpy(info->buffer + 300, name);
							strcat(info->buffer + 300, ".info");

							// Rename icon
							Rename(info->buffer, info->buffer + 300);

							// Group?
							if (object->type == BDO_GROUP)
							{
								// Lock process list
								lock_listlock(&GUI->group_list, 0);

								// See if group is open
								if ((group = backdrop_find_group(object)))
								{
									char *name_copy;

									// Get a copy of the name
									if ((name_copy = AllocVec(strlen(name) + 1, 0)))
									{
										// Signal it to update its name
										strcpy(name_copy, name);
										IPC_Command(group->ipc, GROUP_NEW_NAME, 0, 0, name_copy, 0);
									}
								}

								// Unlock process list
								unlock_listlock(&GUI->group_list);
							}
						}

						// Restore directory
						if (lock)
							UnLock(CurrentDir(old));
					}

					// Left-out
					else if (object->type == BDO_LEFT_OUT)
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
								{
									// Store new label
									stccpy(left->icon_label, name, sizeof(left->icon_label));
									save_pos = 1;
									break;
								}
							}
						}

						// Unlock position list
						unlock_listlock(&GUI->positions);

						// Store name in icon
						if (object->flags & BDOF_CUSTOM_LABEL)
						{
							strcpy(object->device_name, name);
							ret = 1;
							copy = 0;
						}
					}

					// Successful?
					if (ret)
					{
						// Erase object
						backdrop_erase_icon(info, object, 0);

						// Store new name
						if (copy)
						{
							strcpy(object->name, name);
							if (object->flags & BDOF_ASSIGN)
								strcat(object->name, ":");
						}

						// Show new object
						backdrop_render_object(info, object, BRENDERF_CLIP);
					}
				}

				// Unlock backdrop list
				unlock_listlock(&info->objects);
			}
		} while (retry);

		// Abort?
		FreeVec(name);
		if (fail)
			break;
	}

	// Free list
	Att_RemList(list, 0);

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Save any left-outs
	if (save_pos)
	{
		// Lock list
		lock_listlock(&GUI->positions, 0);

		// Save list
		SavePositions(&GUI->positions.list, GUI->position_name);

		// Unlock list
		unlock_listlock(&GUI->positions);
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Restore CD
	if (lock)
		UnLock(CurrentDir(old));
}

// Change volume name of a disk in buffers
void notify_disk_name_change(BackdropInfo *info, char *device, char *new_volume)
{
	IPCData *ipc;
	Lister *lister;
	DirBuffer *buffer;
	char *ptr;

	// Get list lock
	lock_listlock(&GUI->buffer_list, FALSE);

	// Go through all buffers
	for (buffer = (DirBuffer *)GUI->buffer_list.list.lh_Head; buffer->node.ln_Succ;
		 buffer = (DirBuffer *)buffer->node.ln_Succ)
	{
		// Correct disk?
		if (strnicmp(device, buffer->buf_Path, strlen(device)) == 0)
		{
			// Change volume name
			strcpy(buffer->buf_VolumeLabel, new_volume);

			// Get new full path
			strcpy(buffer->buf_ExpandedPath, new_volume);
			if ((ptr = strchr(buffer->buf_Path, ':')))
				strcat(buffer->buf_ExpandedPath, ptr);

			// Is buffer currently displayed in a lister?
			if (buffer->buf_CurrentLister)
			{
				// Refresh path
				IPC_Command(buffer->buf_CurrentLister->ipc, LISTER_REFRESH_PATH, 0, 0, 0, 0);
			}
		}
	}

	// Free list lock
	unlock_listlock(&GUI->buffer_list);

	// Lock lister list
	lock_listlock(&GUI->lister_list, FALSE);

	// Go through listers
	for (ipc = (IPCData *)GUI->lister_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Get lister pointer
		lister = (Lister *)IPCDATA(ipc);

		// Is lister a device list?
		if (lister->cur_buffer->more_flags & DWF_DEVICE_LIST)
		{
			// Check it's not our current backdrop
			if (lister->backdrop_info != info)
			{
				devname_change *change;

				// Allocate data structure
				if ((change = AllocVec(sizeof(devname_change), MEMF_CLEAR)))
				{
					// Fill it out
					strcpy(change->old_name, device);
					strcpy(change->new_name, new_volume);

					// Signal it to update the name of the device
					IPC_Command(ipc, LISTER_UPDATE_DEVICE, 0, 0, change, 0);
				}
			}
		}
	}

	// Unlock lister list
	unlock_listlock(&GUI->lister_list);

	// Have to update main window?
	if (info != GUI->backdrop)
	{
		devname_change change;

		// Fill in change packet
		strcpy(change.old_name, device);
		strcpy(change.new_name, new_volume);

		// Update name
		backdrop_update_disk(GUI->backdrop, &change, TRUE);
	}
}

// Update a disk name
void backdrop_update_disk(BackdropInfo *info, devname_change *change, BOOL show)
{
	BackdropObject *object;

	// Lock main backdrop list
	lock_listlock(&info->objects, FALSE);

	// Go through icons
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Is this the critter?
		if (object->type == BDO_DISK && stricmp(change->old_name, object->device_name) == 0)
		{
			// Erase object
			if (show)
				backdrop_erase_icon(info, object, 0);

			// Store new name
			strcpy(object->name, change->new_name);

			// Show new object
			if (show)
				backdrop_render_object(info, object, BRENDERF_CLIP);
			break;
		}
	}

	// Unlock main backdrop list
	unlock_listlock(&info->objects);
}
