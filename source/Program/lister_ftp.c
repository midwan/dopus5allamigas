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

// See if a path entered means go to FTP
BOOL lister_check_ftp(Lister *lister, char *path)
{
	char *ptr;
	short pos;
	Cfg_Function *function;

	// Look for ftp:// string
	if (strnicmp(path, "ftp://", 6) != 0)
		return 0;

	// Copy the string
	ptr = lister->work_buffer + 512;
	stccpy(ptr, path + 6, 450);

	// Refresh the path field
	lister_update_pathfield(lister);

	// Find first slash
	for (pos = 0; ptr[pos]; pos++)
		if (ptr[pos] == '/')
			break;

	// Got a slash?
	if (ptr[pos] == '/')
	{
		// Change to a null
		ptr[pos] = 0;
		++pos;
	}
	else
		pos = 0;

	// Build command
	stccpy(lister->work_buffer, "FTPConnect", 512);

	// Valid host?
	if (*ptr)
	{
		strcat(lister->work_buffer, " HOST=");
		strcat(lister->work_buffer, ptr);
	}

	// Got a directory?
	if (pos > 0 && ptr[pos])
	{
		BOOL suffix = 0, ok = 0;

		// Bump pointer to path
		ptr += pos;

		// Try and be smart; if path has a suffix it probably points to a file
		for (pos = strlen(ptr) - 1; pos > 0; pos--)
		{
			if (ptr[pos] == '/' || ptr[pos] == '\\')
				break;
			else if (ptr[pos] == '.')
			{
				suffix = 1;
			}
		}

		// Got a slash?
		if (ptr[pos] == '/' || ptr[pos] == '\\')
		{
			// If the path had a suffix, strip the last element
			if (suffix)
				ptr[pos] = 0;

			// Set flag
			if (pos > 0)
				ok = 1;
		}

		// Still got directory?
		if ((ok && pos > 0) || (!ok && !suffix))
		{
			// Add directory field
			strcat(lister->work_buffer, " DIR=\"");
			strcat(lister->work_buffer, ptr);
			strcat(lister->work_buffer, "\"");
		}
	}

	// Add lister
	strcat(lister->work_buffer, " LISTER=");
	lsprintf(lister->work_buffer + strlen(lister->work_buffer), "%lu", (unsigned long)(IPTR)lister);

	// Create new function
	if ((function = new_default_function(lister->work_buffer, global_memory_pool)))
	{
		// Set flag to free function
		function->function.flags2 |= FUNCF2_FREE_FUNCTION;

		// Execute function
		function_launch_quick(FUNCTION_RUN_FUNCTION, function, lister);
	}

	return 1;
}
