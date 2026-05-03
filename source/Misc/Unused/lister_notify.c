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

// Handle a notify message
void lister_handle_notify(Lister *lister, DOpusNotify *notify, char *name)
{
	BOOL show = 0;

	// Lock buffer
	buffer_lock(lister->cur_buffer, TRUE);

	// Create dir?
	if (notify->dn_Flags & DNF_DOS_CREATEDIR)
	{
		DirEntry *entry;
		BPTR lock;
		D_S(struct FileInfoBlock, fib)

		// Lock directory
		if (lock = Lock(name, ACCESS_READ))
		{
			// Examine it
			Examine(lock, fib);
			UnLock(lock);

			// Create entry
			if (entry = create_file_entry(lister->cur_buffer,
										  0,
										  &fib->fib_FileName,
										  fib->fib_Size,
										  fib->fib_DirEntryType,
										  &fib->fib_Date,
										  &fib->fib_Comment,
										  fib->fib_Protection,
										  0,
										  0,
										  0,
										  0))
			{
				// Add to buffer
				add_file_entry(lister->cur_buffer, entry, 0);

				// Save date in buffer (assume this is the latest thing!)
				lister->cur_buffer->buf_DirectoryDate = fib->fib_Date;

				// Mark for refresh
				show = 1;
			}
		}
	}

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);

	// Refresh?
	if (show)
		lister_refresh_display(lister, REFRESHF_SLIDERS | REFRESHF_STATUS);
}
