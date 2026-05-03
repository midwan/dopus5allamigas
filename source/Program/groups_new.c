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

void groups_new(BackdropInfo *info, IPCData *ipc)
{
	BackdropObject *object;
	char name[32], path[80];
	BPTR dir;
	short a;

	// Get group name
	name[0] = 0;
	if (!(super_request_args(info->window->WScreen,
							 GetString(&locale, MSG_GROUP_ENTER_NAME),
							 SRF_SCREEN_PARENT | SRF_BUFFER | SRF_IPC,
							 name,
							 30,
							 ipc,
							 GetString(&locale, MSG_OKAY),
							 GetString(&locale, MSG_CANCEL),
							 0)))
		return;

	// Strip and / or :
	for (a = 0; name[a]; a++)
	{
		if (name[a] == '/' || name[a] == ':')
		{
			name[a] = 0;
			break;
		}
	}

	// Invalid name?
	if (!name[0])
		return;

	// Build full path
	strcpy(path, "dopus5:groups");
	AddPart(path, name, 80);

	// Create directory
	if (!(dir = CreateDir(path)))
	{
		DisplayBeep(info->window->WScreen);
		return;
	}

	// Unlock directory
	UnLock(dir);

	// Write icon
	if (!(icon_write(ICONTYPE_GROUP, path, 0, 0, 0, 0)))
	{
		// Failed
		DeleteFile(path);
		return;
	}

	// Lock backdrop list
	lock_listlock(&info->objects, 1);

	// Create new entry
	if ((object = backdrop_leftout_new(info, path, 0, 0)))
	{
		// Change type to group
		object->type = BDO_GROUP;

		// Add object
		backdrop_new_group_object(info, object, BDNF_CD | BDNF_RECALC);
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Open group
	if (object)
		backdrop_open_group(info, object, 1);
}

// Add a new object
BOOL backdrop_new_group_object(BackdropInfo *info, BackdropObject *object, unsigned short flags)
{
	BOOL show = 1;

	// Get icon
	backdrop_get_icon(info, object, flags & (GETICON_CD | GETICON_KEEP));

	// If it's a disk, fail
	if (!object->icon || (flags & BDNF_NO_DISKS && object->icon->do_Type == WBDISK))
		return 0;

	// Arrange new icon if necessary
	if (object->flags & BDOF_NO_POSITION)
	{
		// Allowed to arrange?
		if (!(flags & BDNF_NO_ARRANGE))
		{
			// Position new icon
			backdrop_place_object(info, object);
		}

		// Don't show, will be arranged later
		else
			show = 0;
	}

	// Show icon
	if (show)
		backdrop_render_object(info, object, BRENDERF_CLIP);

	// Recalc?
	if (flags & BDNF_RECALC)
		backdrop_calc_virtual(info);
	return 1;
}

// Dereference a group object
BOOL group_dereference(BPTR lock, char *name, char *path, Point *position, ULONG *flags)
{
	BPTR old = 0;
	APTR file;
	ULONG data;
	BOOL ok = 0;
	char *buffer;

	// Allocate buffer
	if (!(buffer = AllocVec(512, MEMF_CLEAR)))
		return 0;

	// Change directory?
	if (lock)
		old = CurrentDir(lock);

	// Open file
	if ((file = OpenBuf(name, MODE_OLDFILE, 512)))
	{
		// Read header
		data = 0;
		ReadBuf(file, (char *)&data, sizeof(data));

		// Valid group object?
		if (data == MAKE_ID('G', 'R', 'P', '\0'))
		{
			short len, pos;

			// Clear stuff
			if (path)
				*path = 0;
			if (position)
			{
				position->x = -1;
				position->y = -1;
			}
			if (flags)
				*flags = 0;

			// Read data
			len = ReadBuf(file, buffer, 400);

			// Look for newline
			for (pos = 0; pos < len; pos++)
				if (buffer[pos] == '\n')
					break;

			// Got newline?
			if (buffer[pos] == '\n')
			{
				// Null-terminate
				if (pos > 255)
					buffer[255] = 0;
				else
					buffer[pos] = 0;

				// Copy path
				if (path)
					strcpy(path, buffer);

				// Get position
				if (position && len > pos + 4)
					CopyMem((char *)buffer + pos + 1, (char *)position, sizeof(Point));

				// Get flags
				if (flags && len > pos + 8)
					CopyMem((char *)buffer + pos + 5, (char *)flags, sizeof(ULONG));

				// Set ok flag
				ok = 1;
			}
		}

		// Close file
		CloseBuf(file);
	}

	// Free buffer
	FreeVec(buffer);

	// Restore directory
	if (lock)
		CurrentDir(old);
	return ok;
}

// Write group data file
BOOL group_write_data(char *name, char *path, short x, short y, ULONG flags)
{
	APTR file;
	ULONG data[2];

	// Create file
	if (!(file = OpenBuf(name, MODE_NEWFILE, 384)))
		return 0;

	// Write pointer info
	WriteBuf(file, "GRP\0", 4);
	WriteBuf(file, path, -1);
	WriteBuf(file, "\n", 1);

	// Store position and flags
	data[0] = (x << 16) | y;
	data[1] = flags;

	// Write data
	WriteBuf(file, (char *)data, sizeof(ULONG) * 2);

	// Close file
	CloseBuf(file);
	return 1;
}

// Snapshot position in group
void group_snapshot_icon(BackdropInfo *info, BackdropObject *icon, short x, short y)
{
	GroupData *group;
	char buffer[256];
	BPTR lock, old;
	ULONG flags;

	// Get group pointer
	group = (GroupData *)IPCDATA(info->ipc);

	// Build group path
	lsprintf(buffer, "dopus5:groups/%s", group->name);

	// Change into group directory
	if (!(lock = Lock(buffer, ACCESS_READ)))
		return;
	old = CurrentDir(lock);

	// Dereference object
	if (group_dereference(0, icon->device_name, buffer, 0, &flags))
	{
		// Write new data file
		group_write_data(icon->device_name, buffer, x, y, flags);
	}

	// Restore directory
	UnLock(CurrentDir(old));
}
