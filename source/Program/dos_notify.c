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

// Handle notification of DOS events
void handle_dos_notify(DOpusNotify *notify, FunctionHandle *handle)
{
	// Not DOS action?
	if (!notify || !(notify->dn_Type & DN_DOS_ACTION))
		return;

	// CreateDir?
	if (notify->dn_Flags & DNF_DOS_CREATEDIR)
	{
		// Got valid FileInfoBlock?
		if (notify->dn_Fib)
		{
			// Add change to add file
			function_filechange_addfile(handle, notify->dn_Name, notify->dn_Fib, 0, 0);
		}
	}

	// Delete file?
	else if (notify->dn_Flags & DNF_DOS_DELETEFILE)
	{
		char *ptr;

		// Copy path to buffer
		strcpy(handle->work_buffer, notify->dn_Name);

		// Get pointer to filename
		if ((ptr = FilePart(handle->work_buffer)))
		{
			// Copy to separate buffer and break connection
			strcpy(handle->recurse_path, ptr);
			*ptr = 0;
		}

		// Got valid file?
		if (ptr)
		{
			// Add change to remove file
			function_filechange_delfile(handle, handle->work_buffer, handle->recurse_path, 0, 1);
		}
	}

	// Modify?
	else if (notify->dn_Flags & (DNF_DOS_SETFILEDATE | DNF_DOS_SETCOMMENT | DNF_DOS_SETPROTECTION | DNF_DOS_RENAME |
								 DNF_DOS_CREATE | DNF_DOS_CLOSE))
	{
		// Got valid FileInfoBlock?
		if (notify->dn_Fib)
		{
			struct TagItem tag;
#ifdef USE_64BIT
			UQUAD tmp;
#endif

			// Initialise tag
			tag.ti_Tag = TAG_END;
			tag.ti_Data = 0;

			// Create?
			if (notify->dn_Flags & DNF_DOS_CREATE)
			{
				// Add change to add file
				function_filechange_addfile(handle, notify->dn_Name, notify->dn_Fib, 0, 0);
			}

			// SetFileDate
			else if (notify->dn_Flags & DNF_DOS_SETFILEDATE)
			{
				// Fill out tag
				tag.ti_Tag = FM_Date;
				tag.ti_Data = (IPTR)&notify->dn_Fib->fib_Date;
			}

			// SetComment
			else if (notify->dn_Flags & DNF_DOS_SETCOMMENT)
			{
				// Fill out tag
				tag.ti_Tag = FM_Comment;
				tag.ti_Data = (IPTR)notify->dn_Fib->fib_Comment;
			}

			// SetProtection
			else if (notify->dn_Flags & DNF_DOS_SETPROTECTION)
			{
				// Fill out tag
				tag.ti_Tag = FM_Protect;
				tag.ti_Data = (ULONG)notify->dn_Fib->fib_Protection;
			}

			// Rename
			else if (notify->dn_Flags & DNF_DOS_RENAME)
			{
				// Fill out tag
				tag.ti_Tag = FM_Name;
				tag.ti_Data = (IPTR)notify->dn_Fib->fib_Comment;

				// Is this a directory?
				if (notify->dn_Fib->fib_DirEntryType > 0)
				{
					// Build full pathname for old...
					strcpy(handle->work_buffer, notify->dn_Name);
					AddPart(handle->work_buffer, notify->dn_Fib->fib_FileName, 512);
					AddPart(handle->work_buffer, "", 512);

					// And for new...
					strcpy(handle->work_buffer + 512, notify->dn_Name);
					AddPart(handle->work_buffer + 512, notify->dn_Fib->fib_Comment, 512);
					AddPart(handle->work_buffer + 512, "", 512);

					// Add directory rename
					function_filechange_rename(handle, handle->work_buffer, handle->work_buffer + 512);
				}
			}

			// Size (ie Close)
			else if (notify->dn_Flags & DNF_DOS_CLOSE)
			{
				// Fill out tag
#ifdef USE_64BIT
				tag.ti_Tag = FM_Size64;
				tmp = GETFIBSIZE(notify->dn_Fib);
				tag.ti_Data = (IPTR)&tmp;
#else
				tag.ti_Tag = FM_Size;
				tag.ti_Data = (ULONG)notify->dn_Fib->fib_Size;
#endif
			}

			// Valid tag?
			if (tag.ti_Tag != TAG_END)
			{
				// Add change to modify file
				function_filechange_modify(
					handle, notify->dn_Name, notify->dn_Fib->fib_FileName, tag.ti_Tag, tag.ti_Data, TAG_END);
			}
		}
	}

	// Relabel?
	else if (notify->dn_Flags & DNF_DOS_RELABEL)
	{
		// Pass name change through to things
		notify_disk_name_change(0, notify->dn_Name, notify->dn_Fib->fib_FileName);
	}
}
