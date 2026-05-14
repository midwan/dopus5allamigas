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

enum {
	MAKEFILE_NAME,
	MAKEFILE_NOICON,
	MAKEFILE_SELECT,
};

// MAKEFILE internal function
DOPUS_FUNC(function_makefile)
{
	char *file_path, *file_name;
	PathNode *path;
	short ret = 1, ask_flag = 1, lister_flag = 1, icon_flag = 0, def_icon = 0, select = 0;
	FileChange *change;

	// Allocate buffer for file path
	if (!(file_path = AllocVec(768, MEMF_CLEAR)))
		return 0;
	file_name = file_path + 512;

	// Get current lister
	path = function_path_current(&handle->source_paths);

	// Default to icons?
	if (GUI->flags & GUIF_SAVE_ICONS || handle->flags & FUNCF_ICONS ||
		(path && path->lister && path->lister->flags & LISTERF_VIEW_ICONS))
		def_icon = 1;
	icon_flag = def_icon;

	// Parsed arguments?
	if (instruction->funcargs)
	{
		// No icons?
		if (instruction->funcargs->FA_Arguments[MAKEFILE_NOICON])
			icon_flag = 0;

		// Select?
		if (instruction->funcargs->FA_Arguments[MAKEFILE_SELECT])
			select = 1;

		// File?
		if (instruction->funcargs->FA_Arguments[MAKEFILE_NAME])
		{
			char *path = (char *)instruction->funcargs->FA_Arguments[MAKEFILE_NAME];

			// Don't need to ask
			ask_flag = 0;

			// See if it's a full pathname
			if (strchr(path, '/') || strchr(path, ':'))
			{
				strcpy(file_path, path);
				lister_flag = 0;
			}

			// It's just a name
			else
			{
				// Build path
				strcpy(file_path, handle->source_path);
				AddPart(file_path, path, 512);
			}
		}
	}

	// If no path, and we need one, return
	if (!function_valid_path(path) && lister_flag)
	{
		FreeVec(file_path);
		return 0;
	}

	// Loop until successful or aborted
	FOREVER
	{
		BPTR file, lock;
		long error;

		// Do we need to ask?
		if (ask_flag)
		{
			// Get file name from user
			if (!(ret = function_request(handle,
										 GetString(&locale, MSG_ENTER_FILE_NAME),
										 SRF_BUFFER | SRF_PATH_FILTER,
										 file_name,
										 GUI->def_filename_length,
										 GetString(&locale, (def_icon) ? MSG_MAKEDIR_ICON : MSG_MAKEDIR_NOICON),
										 GetString(&locale, (def_icon) ? MSG_MAKEDIR_NOICON : MSG_MAKEDIR_ICON),
										 GetString(&locale, MSG_CANCEL),
										 0)))
			{
				ret = -1;
				break;
			}

			// Return pressed on string gadget
			else if (ret == -1)
			{
				ret = 1;
			}

			// If name is empty, abort
			if (!file_name[0])
			{
				ret = -1;
				break;
			}

			// Build path name
			strcpy(file_path, handle->source_path);
			AddPart(file_path, file_name, 512);

			// Icon?
			icon_flag = (def_icon != ret - 1);
		}

		// If we want an icon, check name isn't too long
		if (icon_flag)
		{
			char *name;

			// Get filename
			name = FilePart(file_path);

			// Maximum 25 characters
			if (strlen(name) > GUI->def_filename_length - 5)
				name[GUI->def_filename_length - 5] = 0;
		}

		// Don't overwrite an existing object
		if ((lock = Lock(file_path, ACCESS_READ)))
		{
			UnLock(lock);

			// Put up error requester
			if (!ask_flag || (function_error(handle, file_path, MSG_ERROR_CREATING_FILE, -ERROR_OBJECT_EXISTS)) == -1)
			{
				if (!ask_flag)
					ret = 0;
				else
					ret = -1;
				break;
			}

			continue;
		}
		else if ((error = IoErr()) != ERROR_OBJECT_NOT_FOUND)
		{
			// Put up error requester
			if (!ask_flag || (function_error(handle, file_path, MSG_ERROR_CREATING_FILE, -error)) == -1)
			{
				if (!ask_flag)
					ret = 0;
				else
					ret = -1;
				break;
			}

			continue;
		}

		// Create empty file
		if ((file = OriginalOpen(file_path, MODE_NEWFILE)))
		{
			OriginalClose(file);

			// Examine the new file
			if ((lock = Lock(file_path, ACCESS_READ)))
			{
#ifdef USE_64BIT
				ExamineLock64(lock, (FileInfoBlock64 *)handle->s_info);
#else
				Examine(lock, handle->s_info);
#endif
				UnLock(lock);
				break;
			}
		}

		// Put up error requester
		error = IoErr();
		if (!ask_flag || (function_error(handle, file_path, MSG_ERROR_CREATING_FILE, -error)) == -1)
		{
			if (!ask_flag)
				ret = 0;
			else
				ret = -1;
			break;
		}
	}

	// Aborted?
	if (ret < 1)
	{
		if (ret == -1)
			function_abort(handle);
		FreeVec(file_path);
		return 0;
	}

	// Write icon?
	if (icon_flag)
	{
		// Write icon
		if ((icon_write(ICONTYPE_PROJECT, file_path, 0, 0, 0, 0)) != ICONWRITE_OK)
			icon_flag = 0;
	}

	// Got a lister?
	if (lister_flag)
	{
		// Tell lister to update it's stamp
		path->flags |= LISTNF_UPDATE_STAMP;

		// Display status text
		function_text(handle, GetString(&locale, MSG_FILE_CREATED));
	}

	// No path supplied?
	if (!lister_flag)
	{
		BPTR lock;

		// Lock path
		if ((lock = Lock(file_path, ACCESS_READ)))
		{
			// Get full pathname
			DevNameFromLockDopus(lock, file_path, 512);
			UnLock(lock);
		}

		// Clear name pointer
		if ((file_name = FilePart(file_path)))
			*file_name = 0;

		// Use path pointer
		file_name = file_path;
	}

	// Use path in lister
	else
		file_name = path->path;

	// Add entry to the list
	if ((change = function_filechange_addfile(handle, file_name, handle->s_info, 0, 0)))
	{
		// Select/show it
		change->node.ln_Pri = (select) ? FCF_SHOW | FCF_SELECT : FCF_SHOW;
	}

	// Need to add icon?
	if (icon_flag && (change = function_filechange_loadfile(handle, file_name, handle->s_info->fib_FileName, FFLF_ICON)))
	{
		// Select/show it
		if (select && environment->env->settings.icon_flags & ICONFLAG_AUTOSELECT)
			change->node.ln_Pri = FCF_SELECT;
	}

	// Free buffer
	FreeVec(file_path);
	return 1;
}
