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

DOPUS_FUNC(function_assign)
{
	char *ptr, name[35];
	short suc = 0, len;

	// Valid path?
	if (!*handle->source_path)
		return 0;

	// Strip trailing /
	if (handle->source_path[(len = strlen(handle->source_path) - 1)] == '/')
		handle->source_path[len] = 0;

	// Get pointer to object name
	if (!(ptr = FilePart(handle->source_path)) || !*ptr)
		ptr = handle->source_path;

	// Get truncated file name
	get_trunc_filename(ptr, name);

	// Loop while unsuccessful
	while (!suc)
	{
		short ret;
		BPTR lock = 0;

		// Build requester text
		lsprintf(handle->work_buffer, GetString(&locale, MSG_ENTER_ASSIGN_NAME), name);

		// Ask for name
		if (!(ret = function_request(handle,
									 handle->work_buffer,
									 SRF_BUFFER | SRF_PATH_FILTER,
									 handle->work_buffer + 512,
									 30,
									 GetString(&locale, MSG_ASSIGN),
									 GetString(&locale, MSG_ADD),
									 GetString(&locale, MSG_PATH),
									 GetString(&locale, MSG_DEFER),
									 GetString(&locale, MSG_ABORT),
									 0)) ||
			!*(handle->work_buffer + 512))
			return 0;

		// Path assign?
		if (ret == 3)
			suc = AssignPath(handle->work_buffer + 512, handle->source_path);

		// Late assign?
		else if (ret == 4)
			suc = AssignLate(handle->work_buffer + 512, handle->source_path);

		// Try to lock path
		else if ((lock = Lock(handle->source_path, ACCESS_READ)))
		{
			// Add?
			if (ret == 2)
				suc = AssignAdd(handle->work_buffer + 512, lock);

			// Normal assign
			else
				suc = AssignLock(handle->work_buffer + 512, lock);

			// Successful?
			if (suc)
				lock = 0;
		}

		// Error?
		if (!suc)
		{
			// Show error
			suc = !(function_error(handle, handle->work_buffer + 512, MSG_ASSIGNING, -IoErr()));
		}

		// Lock to free?
		if (lock)
			UnLock(lock);
	}

	return 0;
}
