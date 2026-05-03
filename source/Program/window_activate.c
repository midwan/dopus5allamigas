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

// Remove sizes from directories in a window
// Called from LISTER PROCESS
void remove_dir_sizes(Lister *lister)
{
	DirEntry *entry;

	// Lock buffer
	buffer_lock(lister->cur_buffer, FALSE);

	// Go through entries
	for (entry = (DirEntry *)lister->cur_buffer->entry_list.mlh_Head; entry->de_Node.dn_Succ;
		 entry = (DirEntry *)entry->de_Node.dn_Succ)
	{
		// Is entry a selected directory with a known size?
		if (entry->de_Flags & ENTF_SELECTED && entry->de_Node.dn_Type >= ENTRY_DIRECTORY && entry->de_Size > 0)
		{
			// Remove size
			lister->cur_buffer->buf_TotalBytes[0] -= entry->de_Size;
			if (entry->de_Flags & ENTF_SELECTED)
				lister->cur_buffer->buf_SelectedBytes[0] -= entry->de_Size;
			entry->de_Size = 0;
			entry->de_Flags |= ENTF_NO_SIZE;
		}
	}

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);

	// Refresh window
	lister_refresh_display(lister, 0);

	// Update selection information
	select_show_info(lister, 1);
}
