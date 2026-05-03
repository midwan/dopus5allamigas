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

BOOL ReadSoftLinkDopus(BPTR parent, char *path, char *name, SoftLinkInfo *info)
{
	BPTR lock = 0;
	BOOL ok = FALSE;

#ifdef __amigaos4__
	strlcpy(info->sli_Path, path, 256);
	AddPart(info->sli_Path, name, 256);
#else
	// Not got a path lock?
	if (!parent)
	{
		// Try to lock path
		if (!(lock = Lock(path, ACCESS_READ)))
			return 0;
		parent = lock;
	}

	// Read soft link
	if (ReadLink(((struct FileLock *)BADDR(parent))->fl_Task, parent, name, info->sli_Path, 256) > 0)
#endif
	{
		// Unlock path lock if we have one
		if (lock)
			UnLock(lock);

		// Try to lock soft-linked path
		if ((lock = Lock(info->sli_Path, ACCESS_READ)))
		{
			char *ptr;

			// Get real file information
#ifdef USE_64BIT
			ExamineLock64(lock, (FileInfoBlock64 *)&info->sli_Fib);
#else
			Examine(lock, &info->sli_Fib);
#endif

			// Cut filename out of path string
			if ((ptr = PathPart(info->sli_Path)))
				*ptr = 0;

			// Flag success
			ok = TRUE;
		}
	}

	// something went wrong
	if (!ok)
	{
		char buffer[80];
		Fault(IoErr(), NULL, buffer, sizeof(buffer));
		D(bug("couldn't resolve link %s: %s\n", name, buffer));
	}

	// Unlock lock if we have one
	if (lock)
		UnLock(lock);

	return ok;
}
