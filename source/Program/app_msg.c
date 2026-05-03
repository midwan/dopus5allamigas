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

// Allocate and fill out an AppMessage
DOpusAppMessage *alloc_appmsg_files(DirEntry *entry, DirBuffer *buffer, BOOL multifiles)
{
	DOpusAppMessage *msg;
	BPTR dir_lock = 0;
	short arg;

	// Get lock on directory
	if (!buffer->buf_Path[0] || !(dir_lock = Lock(buffer->buf_Path, ACCESS_READ)))
	{
		// If lock failed and we don't have a custom handler, and aren't a special path, fail
		if (!buffer->buf_CustomHandler[0] && !(buffer->more_flags & (DWF_DEVICE_LIST | DWF_CACHE_LIST)))
			return 0;
	}

	// Got a lock?
	if (dir_lock)
	{
		D_S(struct FileInfoBlock, fib)

		// Examine it
		Examine(dir_lock, fib);

		// If it's not a directory, ignore it
		if (fib->fib_DirEntryType < 0)
		{
			// Unlock it
			UnLock(dir_lock);
			dir_lock = 0;

			// If we don't have a custom handler, return
			if (!buffer->buf_CustomHandler[0])
				return 0;
		}
	}

	// Allocate AppMessage
	if (!(msg = AllocAppMessage(global_memory_pool,
								GUI->appmsg_port,
								(multifiles) ? (buffer->buf_SelectedFiles[0] + buffer->buf_SelectedDirs[0]) : 1)))
	{
		UnLock(dir_lock);
		return 0;
	}

	// Any arguments?
	if (msg->da_Msg.am_NumArgs > 0)
	{
		// If multi-drag, get first selected file
		if (multifiles)
			entry = get_entry(&buffer->entry_list, 1, ENTRY_ANYTHING);

		// Fill out arguments
		for (arg = 0; arg < msg->da_Msg.am_NumArgs && entry; arg++)
		{
			// Is entry a directory?
			if (entry->de_Node.dn_Type >= ENTRY_DIRECTORY && dir_lock)
			{
				BPTR old;

				// Go to directory
				old = CurrentDir(dir_lock);

				// Null-name
				SetWBArg(msg, arg, 0, 0, global_memory_pool);

				// Get lock on directory
				if (!(msg->da_Msg.am_ArgList[arg].wa_Lock = Lock(entry->de_Node.dn_Name, ACCESS_READ)))
				{
					// If a custom handler, just pass name
					if (buffer->buf_CustomHandler[0])
						SetWBArg(msg, arg, 0, entry->de_Node.dn_Name, global_memory_pool);
				}

				// Return to previous directory
				if (old)
					CurrentDir(old);
			}

			// A device?
			else if (entry->de_Node.dn_Type == ENTRY_DEVICE)
			{
				char *ptr;

				// Cache?
				if (entry->de_SubType == SUBENTRY_BUFFER)
				{
					// Use comment if available
					if (!(ptr = (char *)GetTagData(DE_Comment, 0, entry->de_Tags)))
					{
						// Otherwise use display string
						ptr = (char *)GetTagData(DE_DisplayString, 0, entry->de_Tags);
					}
				}

				// Otherwise, use name
				else
					ptr = entry->de_Node.dn_Name;

				// Null-name
				SetWBArg(msg, arg, 0, 0, global_memory_pool);

				// Lock thingy
				if (!(msg->da_Msg.am_ArgList[arg].wa_Lock = Lock(ptr, ACCESS_READ)))
				{
					// Just pass name
					SetWBArg(msg, arg, 0, entry->de_Node.dn_Name, global_memory_pool);
				}
			}

			// Otherwise, it's a file
			else
				SetWBArg(msg, arg, dir_lock, entry->de_Node.dn_Name, global_memory_pool);

			// Deselect entry
			deselect_entry(buffer, entry);

			// Get next entry (for multidrag)
			if (multifiles)
				entry = get_entry((struct MinList *)entry, 1, ENTRY_ANYTHING);
			else
				break;
		}
	}

	// Free directory lock
	UnLock(dir_lock);

	return msg;
}

// Get arguments as an array
struct ArgArray *AppArgArray(DOpusAppMessage *msg, short flags)
{
	return WBArgArray(msg->da_Msg.am_ArgList, msg->da_Msg.am_NumArgs, flags);
}

struct ArgArray *WBArgArray(struct WBArg *arglist, short count, short flags)
{
	struct ArgArray *array;

	// No arglist?
	if (!arglist || count < 1)
		return 0;

	// Allocate arg array
	if ((array = NewArgArray()))
	{
		short arg;
		char buf[256];

		// Go through arguments
		for (arg = 0; arg < count; arg++, arglist++)
		{
			struct ArgArrayEntry *entry;

			// Skip directories unless allow dirs is set
			if (!(flags & AAF_ALLOW_DIRS) && !(*arglist->wa_Name))
				continue;

			// Get name of file
			GetWBArgPath(arglist, buf, 256);

			// If a directory, add trailing /
			if (!arglist->wa_Name || !*arglist->wa_Name)
				AddPart(buf, "", 256);

			// Allocate this argument
			if ((entry = NewArgArrayEntry(array, buf)))
			{
				// Directory?
				if (!arglist->wa_Name || !*arglist->wa_Name)
					entry->ae_Flags |= AEF_DIR;

				// Increment array index
				++array->aa_Count;
			}
		}
	}

	return array;
}

// Free an array of arguments
void FreeArgArray(struct ArgArray *array)
{
	if (array)
	{
		// Free arguments and array
		FreeMemHandle(array->aa_Memory);
		FreeVec(array);
	}
}

// Set AppMessage custom data
void set_appmsg_data(DOpusAppMessage *msg, ULONG value1, ULONG value2, ULONG value3)
{
	if (msg)
	{
		// Set check pointer
		msg->da_Msg.am_Reserved[6] = (ULONG)msg;

		// Store data
		msg->da_Msg.am_Reserved[3] = value1;
		msg->da_Msg.am_Reserved[4] = value2;
		msg->da_Msg.am_Reserved[5] = value3;
	}
}

// Get AppMessage custom data
BOOL get_appmsg_data(DOpusAppMessage *msg, ULONG *value1, ULONG *value2, ULONG *value3)
{
	// Check for valid message
	if (!msg || msg->da_Msg.am_Reserved[6] != (ULONG)msg)
		return 0;

	// Return data
	if (value1)
		*value1 = msg->da_Msg.am_Reserved[3];
	if (value2)
		*value2 = msg->da_Msg.am_Reserved[4];
	if (value3)
		*value3 = msg->da_Msg.am_Reserved[5];
	return 1;
}

// Find a WBArg by name and lock
short FindWBArg(struct WBArg *args, short count, char *name)
{
	short num;

	// Go through arguments
	for (num = 0; num < count; num++)
	{
		// Compare by name?
		if (args[num].wa_Name && *args[num].wa_Name)
		{
			// Compare by name
			if (strcmp(args[num].wa_Name, name) == 0)
				return num;
		}

		// Compare by lock
		else
		{
			char buf[256];

			// Get full path
			DevNameFromLockDopus(args[num].wa_Lock, buf, 256);

			// Devices?
			if (buf[strlen(buf) - 1] == ':' && name[strlen(name) - 1] == ':')
			{
				// Compare device names
				if (strcmp(buf, name) == 0)
					return num;
			}

			// Compare file name
			else if (strcmp(FilePart(buf), name) == 0)
				return num;
		}
	}

	// Not found
	return -1;
}

// Unlock the locks in a WBArg array
void UnlockWBArg(struct WBArg *args, short count)
{
	short num;

	// Go through arguments
	for (num = 0; num < count; num++)
	{
		// Got a lock?
		if (args[num].wa_Lock)
		{
			// Don't have a name?
			if (!args[num].wa_Name || !*args[num].wa_Name)
			{
				char buf[256];
				char *ptr;

				// Get full path from lock
				DevNameFromLockDopus(args[num].wa_Lock, buf, 256);

				// Device?
				if (buf[strlen(buf) - 1] == ':')
					ptr = buf;

				// Use filename only
				else
					ptr = FilePart(buf);

				// Allocate copy
				if (args[num].wa_Name)
					FreeMemH(args[num].wa_Name);
				if ((args[num].wa_Name = AllocMemH(0, strlen(ptr) + 1)))
					strcpy(args[num].wa_Name, ptr);
			}

			// Free the lock
			UnLock(args[num].wa_Lock);
			args[num].wa_Lock = 0;
		}
	}
}

// Build an arg array
struct ArgArray *BuildArgArrayA(char **args)
{
	struct ArgArray *array;
	short num;

	// Allocate arg array
	if ((array = NewArgArray()))
	{
		// Go through arguments
		for (num = 0; args[num]; num++)
		{
			struct ArgArrayEntry *entry;

			// Allocate this argument
			if ((entry = AllocMemH(array->aa_Memory, sizeof(struct ArgArrayEntry) + strlen(args[num]))))
			{
				// Copy path
				strcpy(entry->ae_String, args[num]);

				// Increment array index
				++array->aa_Count;

				// Add to list
				AddTail((struct List *)&array->aa_List, (struct Node *)entry);
			}
		}
	}

	return array;
}

// Allocate a new ArgArray
struct ArgArray *NewArgArray(void)
{
	struct ArgArray *array;

	// Allocate arg array
	if ((array = AllocVec(sizeof(struct ArgArray), MEMF_CLEAR)))
	{
		// Initialise arg array
		NewList((struct List *)&array->aa_List);

		// Allocate memory handle
		if (!(array->aa_Memory = NewMemHandle(2048, 512, MEMF_CLEAR)))
		{
			// Failed
			FreeVec(array);
			return 0;
		}
	}

	return array;
}

struct ArgArrayEntry *NewArgArrayEntry(struct ArgArray *array, char *string)
{
	struct ArgArrayEntry *entry;

	// Allocate entry
	if ((entry = AllocMemH(array->aa_Memory, sizeof(struct ArgArrayEntry) + strlen(string))))
	{
		// Copy path
		strcpy(entry->ae_String, string);

		// Add to list
		AddTail((struct List *)&array->aa_List, (struct Node *)entry);
	}

	return entry;
}
