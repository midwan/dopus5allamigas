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

// CLEARSIZES internal function
DOPUS_FUNC(function_clearsizes)
{
	FunctionEntry *entry;
	Lister *lister;
	DirBuffer *buffer = 0;

	// Get directories
	handle->instruction_flags = INSTF_WANT_DIRS;

	// Get current lister
	if (lister = function_lister_current(&handle->source_paths))
		buffer = lister->cur_buffer;

	// Go through files
	while (entry = function_get_entry(handle))
	{
		// Directory?
		if (entry->type > ENTRY_DIRECTORY && entry->entry && !(entry->flags & FUNCENTF_ICON_ACTION) &&
			entry->entry->size > 0)
		{
			// Got a buffer?
			if (buffer)
			{
				// Remove from buffer counts
				buffer->buf_TotalBytes[0] -= entry->entry->size;

				// Selected?
				if (entry->entry->flags & ENTF_SELECTED)
					buffer->buf_SelectedBytes[0] -= entry->entry->size;
			}

			// Clear size
			entry->entry->size = -1;
		}

		// Get next entry
		function_end_entry(handle, entry, 0);
	}

	return 1;
}
