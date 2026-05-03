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

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	short valid;
	short confirm_each;
	ULONG flags;
	ULONG mask;
	short change;
	/*
		char	*source_file;
		short	savepos;
	*/
} AddIconData;

#ifndef __amigaos3__
	#pragma pack()
#endif

enum {
	ARG_FILE,
	ARG_BORDER,
	ARG_NOBORDER,
	ARG_LABEL,
	ARG_NOLABEL,
	ARG_CHANGE,
	/*
		ARG_SOURCE,
		ARG_SAVEPOS,
	*/
};

// ADDICON internal function
DOPUS_FUNC(function_addicon)
{
	FunctionEntry *entry;
	PathNode *path;
	long count = 1;
	short ret = 1;
	AddIconData *data;

	// Get data
	data = (AddIconData *)handle->inst_data;

	// First time through?
	if (!data->valid)
	{
		// Start by always confirming replace
		data->confirm_each = 1;
		data->valid = 1;
		data->flags = 0;
		data->mask = 0;
		/*
				data->source_file=0;
				data->savepos=0;
		*/
		// Parsed arguments?
		if (instruction->funcargs)
		{
			// Only change flags?
			if (instruction->funcargs->FA_Arguments[ARG_CHANGE])
			{
				data->change = 1;
				data->confirm_each = 0;
			}

			// Border/No border?
			if (instruction->funcargs->FA_Arguments[ARG_BORDER])
			{
				data->mask |= ICONF_BORDER_OFF | ICONF_BORDER_ON;
				data->flags |= ICONF_BORDER_ON;
			}
			else if (instruction->funcargs->FA_Arguments[ARG_NOBORDER])
			{
				data->mask |= ICONF_BORDER_OFF | ICONF_BORDER_ON;
				data->flags |= ICONF_BORDER_OFF;
			}

			// Label/No label?
			if (instruction->funcargs->FA_Arguments[ARG_LABEL])
			{
				data->mask |= ICONF_NO_LABEL;
			}
			else if (instruction->funcargs->FA_Arguments[ARG_NOLABEL])
			{
				data->flags |= ICONF_NO_LABEL;
				data->mask |= ICONF_NO_LABEL;
			}

			/*
						// Source file supplied?
						if (instruction->funcargs->FA_Arguments[ARG_SOURCE])
							data->source_file=(char *)instruction->funcargs->FA_Arguments[ARG_SOURCE];

						// Save position?
						if (instruction->funcargs->FA_Arguments[ARG_SAVEPOS])
							data->savepos=1;
			*/
		}
	}

	// Get current path
	if (!(path = function_path_current(&handle->source_paths)))
		return 0;

	// Tell this path to update it's datestamp at the end
	path->flags |= LISTNF_UPDATE_STAMP;

	// Turn progress indicator on
	function_progress_on(
		handle, GetString(&locale, MSG_PROGRESS_ADDING_ICONS), handle->entry_count, PWF_FILENAME | PWF_GRAPH);

	// Go through entries
	while ((entry = function_get_entry(handle)))
	{
		BOOL file_ok = 0, replace_image = 0;

		// Update progress indicator
		if (function_progress_update(handle, entry, count))
		{
			function_abort(handle);
			ret = 0;
			break;
		}

		// Ignore icons
		if (!(isicon(entry->name)))
		{
			// Build source and icon name
			function_build_source(handle, entry, handle->work_buffer);
			strcpy(handle->work_buffer + 384, handle->work_buffer);
			strcat(handle->work_buffer + 384, ".info");

			// Need confirmation?
			if (data->confirm_each)
			{
				BPTR lock;

				// Does icon exist?
				if ((lock = Lock(handle->work_buffer + 384, ACCESS_READ)))
				{
					UnLock(lock);

					// Build message
					lsprintf(handle->work_buffer + 800, GetString(&locale, MSG_ICON_ALREADY_EXISTS), entry->name);

					// Display requester
					if (!(ret = function_request(handle,
												 handle->work_buffer + 800,
												 0,
												 GetString(&locale, MSG_REPLACE),
												 GetString(&locale, MSG_ALL),
												 GetString(&locale, MSG_ICON_REPLACE_IMAGE),
												 GetString(&locale, MSG_SKIP),
												 GetString(&locale, MSG_CANCEL),
												 0)))
					{
						// Aborted?
						function_abort(handle);
						break;
					}

					// Skip?
					else if (ret == 4)
						ret = 0;

					// Replace all?
					else if (ret == 2)
						data->confirm_each = 0;

					// Replace image?
					else if (ret == 3)
						replace_image = 1;
				}
			}

			// Ok to write?
			if (ret)
			{
				// Changing flags?
				if (data->change && data->mask)
				{
					struct DiskObject *icon;

					// Get icon

					// gjp V44 icn stuff
					if (IconBase->lib_Version >= 44)
					{
						// icon=GetIconTags(handle->work_buffer,
						//	ICONGETA_FailIfUnavailable,TRUE,
						//	ICONGETA_RemapIcon,FALSE,
						//	TAG_DONE);

						struct Screen *screen = LockPubScreen(0);

						icon = GetIconTags(
							handle->work_buffer, ICONGETA_FailIfUnavailable, TRUE, ICONGETA_Screen, screen, TAG_DONE);
						UnlockPubScreen(NULL, screen);
					}
					else
						icon = GetDiskObject(handle->work_buffer);

					if (icon)
					{
						unsigned long flags;

						// Get current flags
						flags = GetIconFlags(icon);

						// Clear mask
						flags &= ~data->mask;

						// Set new flags
						flags |= data->flags;

						// Set flags
						SetIconFlags(icon, flags);

						// Write icon

						if (PutDiskObject(handle->work_buffer, icon))
						{
							// Load icon file into listers
							function_filechange_loadfile(handle, path->path, entry->name, FFLF_ICON);
							file_ok = 1;
						}

						// Free icon
						FreeDiskObject(icon);
					}
				}

				// Write icon
				else
				{
					short ok, err;
					ret = 1;
					while (!(ok = icon_write((entry->type > 0) ? ICONTYPE_DRAWER : ICONTYPE_PROJECT,
											 handle->work_buffer,
											 replace_image,
											 data->flags,
											 data->mask,
											 &err)))
					{
						// Display error requester
						if ((ret = function_error(handle, entry->name, MSG_ADDICONING, err)) == -1 || ret == 0)
							break;
						function_build_source(handle, entry, handle->work_buffer);
					}

					// Success?
					if (ok)
					{
						// Load icon file into listers
						function_filechange_loadfile(handle, path->path, entry->name, FFLF_ICON);
						file_ok = 1;
					}
					else if (ret == -1)
						break;
				}
			}
		}

		// Get next entry, increment count
		count += function_end_entry(handle, entry, file_ok);

		// Reset result code
		ret = 1;
	}

	return ret;
}
