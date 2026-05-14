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

void desktop_delete(IPCData *ipc, BackdropInfo *info, BackdropObject *only_one)
{
	short groupcount = 0, filecount = 0, assigncount = 0, othercount = 0, dircount = 0, commandcount = 0;
	BackdropObject *object = 0;
	Att_List *list;
	Att_Node *node;
	char buf[100];

	// Create list
	if (!(list = Att_NewList(LISTF_POOL)))
		return;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Go through backdrop list
	while ((object = backdrop_next_object(info, object, only_one)))
	{
		// Group?
		if (object->type == BDO_GROUP)
			++groupcount;

		// Assign?
		else if (object->flags & BDOF_ASSIGN)
			++assigncount;

		// Group object?
		else if (object->type == BDO_LEFT_OUT && info->flags & BDIF_GROUP)
			++filecount;

		// Desktop object?
		else if (object->type == BDO_LEFT_OUT && object->flags & BDOF_DESKTOP_FOLDER)
		{
			++othercount;
			if (object->icon->do_Type == WBDRAWER)
				++dircount;
		}

		// Command left-out?
		else if (object->type == BDO_LEFT_OUT && info->flags & BDIF_MAIN_DESKTOP &&
				 !(object->flags & BDOF_TEMP_LEFTOUT) && command_filetype_match(object))
			++commandcount;

		// Something else
		else
			continue;

		// Add to list
		Att_NewNode(list, 0, (IPTR)object, 0);
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Nothing to delete?
	if (IsListEmpty((struct List *)list))
	{
		Att_RemList(list, 0);
		return;
	}

	// Build message; start with query
	strcpy(info->buffer, GetString(&locale, MSG_DESKTOP_REALLY_DELETE));

	// Any groups?
	if (groupcount > 0)
	{
		lsprintf(buf, GetString(&locale, MSG_DESKTOP_DELETE_GROUPS), groupcount);
		strcat(info->buffer, buf);
	}

	// Or assigns?
	else if (assigncount > 0)
	{
		lsprintf(buf, GetString(&locale, MSG_DESKTOP_DELETE_ASSIGNS), assigncount);
		strcat(info->buffer, buf);
	}

	// Any group files?
	else if (filecount > 0)
	{
		// Add file count
		lsprintf(buf, GetString(&locale, MSG_DESKTOP_DELETE_GROUP_OBJECTS), filecount);
		strcat(info->buffer, buf);
	}

	// Desktop objects or command left-outs?
	if (othercount > 0 || commandcount > 0)
	{
		BOOL cr = 0;
		short total_files = commandcount + othercount - dircount;

		// Add CR?
		if (groupcount > 0 || assigncount > 0 || filecount > 0)
			cr = 1;

		// Add files
		if (total_files > 0)
		{
			lsprintf(buf, GetString(&locale, MSG_DESKTOP_DELETE_DESKTOP_FILES), total_files);
			if (cr)
				strcat(info->buffer, "\n");
			strcat(info->buffer, buf);
			cr = 1;
		}

		// Add dirs
		if (dircount > 0)
		{
			lsprintf(buf, GetString(&locale, MSG_DESKTOP_DELETE_DESKTOP_DIRS), dircount);
			if (cr)
				strcat(info->buffer, "\n");
			strcat(info->buffer, buf);
		}
	}

	// Add question mark
	strcat(info->buffer, "?");

	// Display requester
	if (!(super_request_args(GUI->screen_pointer,
							 info->buffer,
							 SRF_IPC | SRF_SCREEN_PARENT,
							 ipc,
							 GetString(&locale, MSG_DELETE),
							 GetString(&locale, MSG_CANCEL),
							 0)))
	{
		Att_RemList(list, 0);
		return;
	}

	// Check owner for refresh
	if (info->lister)
		IPC_Command(info->lister->ipc, LISTER_CHECK_REFRESH, 0, 0, 0, REPLY_NO_PORT);

	// Group objects?
	if (info->flags & BDIF_GROUP)
	{
		// Send command to group
		IPC_Command(info->ipc, GROUP_DELETE, 0, only_one, 0, 0);
	}

	// Otherwise
	else
	{
		// Go through the list
		for (node = (Att_Node *)list->list.lh_Head; node->node.ln_Succ; node = (Att_Node *)node->node.ln_Succ)
		{
			// Lock backdrop list
			lock_listlock(&info->objects, 1);

			// Get object
			if ((object = find_backdrop_object(info, (BackdropObject *)node->data)))
			{
				// Group?
				if (object->type == BDO_GROUP)
				{
					// Delete this group
					backdrop_delete_group(info, object);
				}

				// Assign?
				else if (object->flags & BDOF_ASSIGN)
				{
					// Copy name, strip trailing colon
					strcpy(info->buffer + 2, object->name);
					info->buffer[strlen(info->buffer + 2) + 1] = 0;

					// Delete assign
					if (AssignLock(info->buffer + 2, 0))
					{
						// Erase object
						backdrop_erase_icon(info, object, 0);

						// Remove object
						backdrop_remove_object(info, object);
					}
				}

				// Command left-out?
				else if (object->type == BDO_LEFT_OUT && info->flags & BDIF_MAIN_DESKTOP &&
						 !(object->flags & BDOF_TEMP_LEFTOUT) && command_filetype_match(object))
				{
					char path[256];

					// Get full path
					stccpy(path, object->path, sizeof(path));
					AddPart(path, object->name, sizeof(path));

					// Delete command file, or clean up an orphaned left-out
					if (DeleteFile(path) || IoErr() == ERROR_OBJECT_NOT_FOUND)
					{
						// Delete icon if present
						DeleteDiskObject(path);

						// Remove command from the command list
						command_remove(object->name);

						// Erase object
						backdrop_erase_icon(info, object, 0);

						// Remove left-out record
						lock_listlock(&GUI->positions, TRUE);
						if (backdrop_remove_leftout(object))
							SavePositions(&GUI->positions.list, GUI->position_name);
						unlock_listlock(&GUI->positions);

						// Remove object
						backdrop_remove_object(info, object);
					}
				}
			}

			// Unlock backdrop list
			unlock_listlock(&info->objects);
		}
	}

	// Free list
	Att_RemList(list, 0);

	// Recalc backdrop objects
	backdrop_calc_virtual(info);

	// Delete other things?
	if (othercount > 0)
	{
		// Run delete function
		icon_function(info, 0, 0, def_function_delete_quiet, 0);
	}
}
