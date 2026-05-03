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

static const char *const icon_types[] =
	{"all", "disk", "drawer", "tool", "project", "trash", "baddisk", "leftout", "lister", "lister2", 0};

// Add PopUp extensions
void add_popup_ext(char *menu, Att_List *type_list, char *command, ULONG flags)
{
	Att_Node *node;

	// Go through menu list
	for (node = (Att_Node *)type_list->list.lh_Head; node->node.ln_Succ; node = (Att_Node *)node->node.ln_Succ)
	{
		Cfg_Filetype *ftype = 0;
		ULONG type = 0;
		short a;

		// No name?
		if (!node->node.ln_Name)
			continue;

		// Special type?
		for (a = 0; icon_types[a]; a++)
		{
			// Compare string
			if (stricmp(node->node.ln_Name, icon_types[a]) == 0)
			{
				// All?
				if (a == 0)
					type = POPUP_ALL;
				else
					type = a;
				break;
			}
		}

		// Search filetypes?
		if (!type)
		{
			// Try to find filetype
			if (!(ftype = filetype_find(node->node.ln_Name, 1)))
				ftype = filetype_find(node->node.ln_Name, 0);
		}

		// Got something to match on?
		if (type || ftype)
		{
			PopUpExt *ext;

			// Allocate PopUpExtension
			if ((ext = AllocMemH(global_memory_pool, sizeof(PopUpExt))))
			{
				// Fill it out
				ext->pe_Type = type;
				if (ftype)
					stccpy(ext->pe_FileType, ftype->type.name, sizeof(ext->pe_FileType));
				stccpy(ext->pe_Command, command, 40);
				stccpy(ext->pe_Menu, menu, 40);
				ext->pe_Flags = flags;

				// Lock list and add new entry
				lock_listlock(&GUI->popupext_list, TRUE);
				AddTail(&GUI->popupext_list.list, (struct Node *)ext);
				unlock_listlock(&GUI->popupext_list);
			}
		}
	}
}

// Remove PopUp extensions for a command
void remove_popup_ext(char *name)
{
	PopUpExt *ext, *next;

	// Lock list
	lock_listlock(&GUI->popupext_list, TRUE);

	// Go through list
	for (ext = (PopUpExt *)GUI->popupext_list.list.lh_Head; ext->pe_Node.ln_Succ; ext = next)
	{
		// Cache next
		next = (PopUpExt *)ext->pe_Node.ln_Succ;

		// Match command name
		if (stricmp(ext->pe_Command, name) == 0)
		{
			// Remove this and free it
			Remove((struct Node *)ext);
			FreeMemH(ext);
		}
	}

	// Unlock list
	unlock_listlock(&GUI->popupext_list);
}

// See if there's any extensions for this filetype
BOOL popup_ext_check(char *name)
{
	PopUpExt *ext;
	BOOL ok = 0;

	// Lock list
	lock_listlock(&GUI->popupext_list, FALSE);

	// Go through list
	for (ext = (PopUpExt *)GUI->popupext_list.list.lh_Head; ext->pe_Node.ln_Succ;
		 ext = (PopUpExt *)ext->pe_Node.ln_Succ)
	{
		// Match filetype
		if (stricmp(ext->pe_FileType, name) == 0)
		{
			// Got one
			ok = 1;
			break;
		}
	}

	// Unlock list
	unlock_listlock(&GUI->popupext_list);
	return ok;
}

// Run PopUp function
void popup_run_func(PopUpExt *ext, BackdropObject *object, char *filename, Lister *source)
{
	char *buffer, *ptr;
	Cfg_Function *func = 0;
	PopUpExt *check;

	// Lock PopUp list
	lock_listlock(&GUI->popupext_list, FALSE);

	// Check function is still valid
	for (check = (PopUpExt *)GUI->popupext_list.list.lh_Head; check->pe_Node.ln_Succ && check != ext;
		 check = (PopUpExt *)check->pe_Node.ln_Succ)
		;

	// Valid?
	if (check == ext)
	{
		// Allocate buffer
		if ((buffer = AllocVec(580, MEMF_CLEAR)))
		{
			// Copy command name to buffer
			stccpy(buffer, check->pe_Command, 580);

			// Get pointer to end
			ptr = buffer + strlen(buffer);

			// Add space
			*(ptr++) = ' ';
			*ptr = 0;

			// Got filename?
			if (filename)
			{
				// Add filename in quotes
				lsprintf(ptr, "\"%s\"", filename);
			}

			// Got icon?
			else if (object)
			{
				BPTR lock;

				// Disk?
				if (object->type == BDO_DISK)
				{
					// Get device name
					if (object->device_name)
					{
						// Add device name in quotes
						lsprintf(ptr, "\"%s\"", object->device_name);
					}
				}

				// Otherwise
				else
				{
					// Add quote
					*(ptr++) = '\"';

					// Get object lock
					if ((lock = backdrop_icon_lock(object)))
					{
						// Get path
						DevNameFromLockDopus(lock, ptr, 512);
						UnLock(lock);
					}

					// Add name to path
					AddPart(ptr, object->name, 512);

					// Add trailing quote
					strcat(ptr, "\"");
				}
			}

			// Create function
			func = new_default_function(buffer, global_memory_pool);

			// Free buffer
			FreeVec(buffer);
		}
	}

	// Unlock list
	unlock_listlock(&GUI->popupext_list);

	// No function?
	if (!func)
		return;

	// Set flag to free function automatically
	func->function.flags2 |= FUNCF2_FREE_FUNCTION;

	// Launch function
	function_launch_quick(FUNCTION_RUN_FUNCTION, func, source);
}
