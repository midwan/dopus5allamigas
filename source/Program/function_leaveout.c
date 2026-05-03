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

// Leave some objects out
DOPUS_FUNC(function_leaveout)
{
	PathNode *path;
	FunctionEntry *entry;
	short ret = 1;
	short count = 0;

	// Get current path
	if (!(path = function_path_current(&handle->source_paths)))
		return 0;

	// Go through entries
	while ((entry = function_get_entry(handle)))
	{
		BOOL ok = 0;

		// Check for abort
		if (function_check_abort(handle))
		{
			ret = 0;
			break;
		}

		// Build source name
		function_build_source(handle, entry, handle->work_buffer);

		// Ignore if this is an icon
		if (!(isicon(handle->work_buffer)))
		{
			// Leave object out
			if (backdrop_leave_out(GUI->backdrop, handle->work_buffer, BLOF_PERMANENT | BLOF_REFRESH, -1, -1))
			{
				ok = 1;
				++count;
			}
		}

		// Get next entry
		function_end_entry(handle, entry, ok);
	}

	// Update leftout list
	if (count)
		backdrop_save_leftouts(GUI->backdrop);
	return ret;
}
