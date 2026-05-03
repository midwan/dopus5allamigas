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

// Get the parent of a path
int path_parent(char *path)
{
	short len;

	// Get length of path
	if ((len = strlen(path) - 1) < 0)
		return 0;

	// If last character is a :, can't go any further
	if (path[len] == ':')
		return 0;

	// If last character is a /, start at one before
	if (path[len] == '/')
		--len;

	// Go back to previous / or :
	while (len > 0 && path[len] != '/' && path[len] != ':')
		--len;

	// Got a separator?
	if (path[len] == '/' || path[len] == ':')
	{
		// Clear after this
		path[len + 1] = 0;
		return 1;
	}

	// Nothing
	return 0;
}

// Get the root of a path
int path_root(char *path)
{
	short count;

	// Keep doing parent until we reach the root
	for (count = 0; path_parent(path); count++)
		;

	return count;
}

// Get the root of a pathname
int get_path_root(char *path, char *root_name, struct DateStamp *date)
{
	struct DevProc *proc;
	D_S(struct InfoData, info)
	struct DosList *dos;

	// Get device process
	if (!(proc = GetDeviceProc(path, 0)))
		return 0;

		// Send info packet
#ifdef __AROS__
	if (((struct Library *)DOSBase)->lib_Version < 50)
	{
		// Info(proc->dvp_Lock,info);
		BPTR lock;
		if (!(lock = Lock(path, SHARED_LOCK)))
		{
			FreeDeviceProc(proc);
			return 0;
		}
		Info(lock, info);
		UnLock(lock);
	}
	else
#endif
		DoPkt(proc->dvp_Port, ACTION_DISK_INFO, MKBADDR(info), 0, 0, 0, 0);

	// Get DOS list pointer
	if (!(dos = (struct DosList *)BADDR(info->id_VolumeNode)))
	{
		FreeDeviceProc(proc);
		return 0;
	}

	// Get root device name
	if (root_name)
	{
		if (dos->dol_Name)
			BtoCStr(dos->dol_Name, root_name, 32);
		else
			*root_name = 0;
	}

	// Copy datestamp if buffer supplied
	if (date)
		*date = dos->dol_misc.dol_volume.dol_VolumeDate;

	// Free device process
	FreeDeviceProc(proc);

	// Return disk state
	return info->id_DiskState;
}
