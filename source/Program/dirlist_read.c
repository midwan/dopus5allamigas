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

// Launches read directory function for a lister
void read_directory(Lister *lister, char *path, int flags)
{
	// Check path for FTP
	if (!(lister_check_ftp(lister, path)))
	{
		// Launch read function
		function_launch(FUNCTION_READ_DIRECTORY, 0, 0, flags, lister, 0, path, 0, 0, 0, 0);
	}
}

// Create a new lister and read a directory into it
Lister *read_new_lister(char *path, Lister *parent, UWORD qual)
{
	Cfg_Lister *cfg;
	Lister *new_lister = 0;

	// If shift is not down, look for open lister first
	if (!(qual & IEQUAL_ANYSHIFT))
	{
		IPCData *ipc;
		Lister *lister = 0;

		// Lock lister list
		lock_listlock(&GUI->lister_list, FALSE);

		// Go through listers
		for (ipc = (IPCData *)GUI->lister_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
		{
			// Get lister
			lister = IPCDATA(ipc);

			// Is this lister what we're after?
			if (stricmp(lister->cur_buffer->buf_Path, path) == 0 && !(lister->flags & LISTERF_BUSY))
			{
				// Activate this window
				IPC_Command(ipc, IPC_ACTIVATE, 0, (APTR)1, 0, 0);
				break;
			}
		}

		// Unlock lister list
		unlock_listlock(&GUI->lister_list);

		// Get one?
		if (ipc->node.mln_Succ)
			return lister;
	}

	// Get a new lister definition
	if ((cfg = NewLister(path)))
	{
		char fullpath[512];
		BPTR lock;
		short mode;
		position_rec *pos;

		// Expand path just in case
		if ((lock = Lock(path, ACCESS_READ)))
		{
			NameFromLock(lock, fullpath, 512);
			AddPart(fullpath, "", 512);
			UnLock(lock);
		}
		else
			strcpy(fullpath, path);

		// Get position
		pos = GetListerPosition(
			fullpath, 0, 0, &cfg->lister.pos[0], &mode, &cfg->lister.format, (parent) ? parent->window : 0, parent, 0);

		// Split display?
		if (qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
		{
			// Fill out lister
			lister_init_new(cfg, parent);
		}

		// Create a new lister
		if ((new_lister = lister_new(cfg)))
		{
			// Store position record
			new_lister->pos_rec = pos;

			// Split display?
			if (qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
			{
				IPC_Command(new_lister->ipc, LISTER_SPLIT, 0, parent, 0, 0);
			}

			// Open in icon mode?
			if (mode & LISTERMODE_ICON)
			{
				// Force new lister into icon view mode
				cfg->lister.flags |= DLSTF_ICON;
				if (mode & LISTERMODE_ICON_ACTION)
					cfg->lister.flags |= DLSTF_ICON_ACTION;

				// Show all?
				if (mode & LISTERMODE_SHOW_ALL)
					cfg->lister.flags |= DLSTF_SHOW_ALL;
			}

			// Initialise lister
			{
				ULONG init_flags = LISTERF_MAKE_SOURCE;

				if ((environment->env->lister_options & LISTEROPTF_DUAL_DEFAULT) &&
					!(mode & LISTERMODE_ICON) && !(qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT)))
					init_flags |= LISTERF_DUAL;

				IPC_Command(new_lister->ipc, LISTER_INIT, init_flags, GUI->screen_pointer, 0, 0);
			}
		}
	}

	return new_lister;
}
