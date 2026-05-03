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

#include "cleanup.h"

#include <Program/position.h>
#include <Program/main_commands.h>

char *version = "$VER: cleanup.module 68.0 (3.10.98)";

int LIBFUNC L_Module_Entry(REG(a0, struct List *files),
						   REG(a1, struct Screen *screen),
						   REG(a2, IPCData *ipc),
						   REG(a3, IPCData *main_ipc),
						   REG(d0, ULONG mod_id),
						   REG(d1, EXT_FUNC(func_callback)))
{
	struct Window *window;
	APTR memory;
	BOOL change = 0;

	// Open status window
	window = OpenStatusWindow(GetString(locale, MSG_CLEANUP_TITLE),
							  GetString(locale, MSG_PLEASE_WAIT),
							  screen,
							  0,
							  WINDOW_NO_CLOSE | WINDOW_NO_ACTIVATE);

	// Tell Opus to save positions out
	IPC_Command(main_ipc, MAINCMD_SAVE_POSITIONS, 0, 0, 0, REPLY_NO_PORT);

	// Allocate memory handle
	if ((memory = NewMemHandle(4096, 1024, MEMF_CLEAR)))
	{
		APTR iff;

		// Open position info file
		if ((iff = IFFOpen("dopus5:system/position-info", IFF_READ, ID_OPUS)))
		{
			ULONG id;
			BOOL ok = 1;
			char name[40], *ptr;
			struct DosList *dos;
			struct List list;

			// Initialise list
			NewList(&list);

			// Lock dos list
			dos = LockDosList(LDF_VOLUMES | LDF_DEVICES | LDF_READ);

			// Read file
			while ((id = IFFNextChunk(iff, 0)))
			{
				struct Node *entry;

				// Valid chunk?
				if (id != ID_POSI && id != ID_LOUT && id != ID_ICON && id != ID_STRT)
					continue;

				// Allocate record
				if (!(entry = AllocMemH(memory, IFFChunkSize(iff) + sizeof(struct Node))))
				{
					ok = 0;
					break;
				}

				// Read chunk data
				IFFReadChunkBytes(iff, (APTR)(entry + 1), -1);

				// Set type and name pointer
				if (id == ID_POSI)
				{
					entry->ln_Type = PTYPE_POSITION;
					entry->ln_Name = ((position_rec *)entry)->name;
				}
				else if (id == ID_LOUT)
				{
					entry->ln_Type = PTYPE_LEFTOUT;
					entry->ln_Name = ((leftout_record *)entry)->name;
				}
				else if (id == ID_ICON)
				{
					entry->ln_Type = PTYPE_APPICON;
					entry->ln_Name = ((leftout_record *)entry)->name;
				}

				// Only check if a position or leftout
				if (id == ID_POSI || id == ID_LOUT)
				{
					// Get device name
					stccpy(name, entry->ln_Name, 39);
					if ((ptr = strchr(name, ':')))
						*ptr = 0;

					// Find in dos list
					if (FindDosEntry(dos, name, (id == ID_POSI) ? LDF_VOLUMES : LDF_DEVICES))
					{
						BPTR lock;

						// Try to lock thing
						if (!(lock = Lock(entry->ln_Name, ACCESS_READ)))
						{
							// Failed to find, free node
							FreeMemH(entry);
							entry = 0;
							change = 1;
						}
						else
							UnLock(lock);
					}
				}

				// Add to list
				if (entry)
					AddTail(&list, entry);
			}

			// Unlock DOS list
			UnLockDosList(LDF_VOLUMES | LDF_DEVICES | LDF_READ);

			// Close file
			IFFClose(iff);

			// No changes?
			if (!change)
				ok = 0;

			// Re-open file
			if (ok && (iff = IFFOpen("dopus5:system/position-info", IFF_WRITE | IFF_SAFE, ID_OPUS)))
			{
				struct Node *node;

				// Go through entries
				for (node = list.lh_Head; node->ln_Succ; node = node->ln_Succ)
				{
					// Valid path, or AppIcon?
					if ((node->ln_Name && *node->ln_Name) || node->ln_Type == PTYPE_APPICON)
					{
						ULONG id = 0;
						short size = 0;

						// Position?
						if (node->ln_Type == PTYPE_POSITION)
						{
							// Get id and chunk size
							id = ID_POSI;
							size = sizeof(position_rec) - sizeof(struct Node);
						}

						// Left-out?
						else if (node->ln_Type == PTYPE_LEFTOUT)
						{
							// Get id and chunk size
							id = ID_LOUT;
							size = sizeof(leftout_record) - sizeof(struct Node);
						}

						// AppIcon?
						else if (node->ln_Type == PTYPE_APPICON)
						{
							// Get id and chunk size
							id = ID_ICON;
							size = sizeof(leftout_record) - sizeof(struct Node);
						}

						// Write chunk
						if (!(IFFWriteChunk(
								iff, (APTR)(node + 1), id, size + ((node->ln_Name) ? strlen(node->ln_Name) : 0))))
							break;
					}
				}

				// Close file
				IFFClose(iff);
			}
		}

		// Free memory
		FreeMemHandle(memory);
	}

	// Tell Opus to get new positions out
	if (change)
		IPC_Command(main_ipc, MAINCMD_LOAD_POSITIONS, 0, 0, 0, 0);

	// Close status window
	if (window)
		CloseConfigWindow(window);
	return 1;
}
