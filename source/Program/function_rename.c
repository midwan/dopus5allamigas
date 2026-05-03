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

enum { RENAME_NAME, RENAME_NEWNAME };

typedef struct
{
	short wild_flag;
	short confirm_each;
	char new_name_edit[256];
	char old_name_edit[256];
} RenameData;

// RENAME internal function
DOPUS_FUNC(function_rename)
{
	FunctionEntry *entry;
	int ret = 1, count = 1;
	char *source_file, *dest_file, *dest_name;
	char *new_name;
	PathNode *path;
	RenameData *data;
	BOOL progress = 0;

	// Get current source path
	if (!(path = function_path_current(&handle->source_paths)))
		return 0;

	// Get pointer to data
	data = (RenameData *)handle->inst_data;

	// Tell this lister to update it's datestamp at the end
	path->flags |= LISTNF_UPDATE_STAMP;

	// Allocate memory for strings
	if (!(source_file = AllocVec(1024, MEMF_CLEAR)))
		return 0;
	dest_file = source_file + 256;
	dest_name = dest_file + 256;
	new_name = dest_name + 256;

	// Go through entries
	while ((entry = function_get_entry(handle)))
	{
		BOOL file_ok = 1, asked = 0;

		// Update progress indicator
		if (function_progress_update(handle, entry, count))
		{
			function_abort(handle);
			ret = 0;
			break;
		}

		// Build source name
		function_build_source(handle, entry, source_file);

		// Get destination filename
		strcpy(dest_name, entry->name);

		// An icon entry?
		if (entry->flags & FUNCENTF_ICON)
		{
			// Get new name, tack a .info to the end
			strcpy(dest_name, new_name);
			strcat(dest_name, ".info");
		}

		// Normal entry
		else
		{
			// Do we have to ask for a name?
			if (!data->wild_flag)
			{
				// Name supplied in arguments?
				if (instruction->funcargs && instruction->funcargs->FA_Arguments[RENAME_NAME] &&
					instruction->funcargs->FA_Arguments[RENAME_NEWNAME])
				{
					// Store names
					strcpy(data->old_name_edit, FilePart((char *)instruction->funcargs->FA_Arguments[RENAME_NAME]));
					strcpy(data->new_name_edit, FilePart((char *)instruction->funcargs->FA_Arguments[RENAME_NEWNAME]));
					ret = 1;
				}

				// Need to ask
				else
				{
					// Ask for a new name
					strcpy(data->new_name_edit, entry->name);
				ask_point:
					strcpy(data->old_name_edit, entry->name);
					asked = 1;
					if (!(ret = function_request(handle,
												 GetString(&locale, MSG_ENTER_NEW_NAME),
												 SRF_BUFFER | SRF_BUFFER2 | SRF_PATH_FILTER,
												 data->new_name_edit,
												 GUI->def_filename_length,
												 data->old_name_edit,
												 GUI->def_filename_length,
												 GetString(&locale, MSG_RENAME),
												 GetString(&locale, MSG_SKIP),
												 GetString(&locale, MSG_ABORT),
												 0)))
					{
						function_abort(handle);
						break;
					}

					// Skip?
					if (ret == 2)
					{
						file_ok = 0;
						ret = 0;
					}

					// Otherwise
					else
					{
						// Turn -1 into 1
						if (ret == -1)
							ret = 1;
					}
				}

				// Got file?
				if (file_ok)
				{
					// Does the new name contain wildcards?
					if (strchr(data->new_name_edit, '*'))
					{
						// If the old name doesn't, make it a *
						if (!(strchr(data->old_name_edit, '*')))
							strcpy(data->old_name_edit, "*");

						// Set wildcard flag
						data->wild_flag = 1;
					}

					// No wildcards
					else
					{
						// Store new name
						strcpy(dest_name, data->new_name_edit);
					}
				}
			}

			// Wildcard rename?
			if (data->wild_flag)
			{
				// Progress indicator not shown yet?
				if (!progress)
				{
					// Turn progress indicator on
					function_progress_on(handle,
										 GetString(&locale, MSG_PROGRESS_RENAMING),
										 handle->entry_count,
										 PWF_FILENAME | PWF_GRAPH);
					progress = 1;
				}

				// Get new name
				file_ok = rename_get_wild(data->old_name_edit, data->new_name_edit, entry->name, dest_name);
			}

			// Still ok?
			if (file_ok)
			{
				// Get new name
				strcpy(new_name, dest_name);
			}
		}

		// Ok to do this file?
		if (file_ok)
		{
			// Check names are different
			if (strcmp(dest_name, entry->name) != 0)
			{
				// Get destination path
				strcpy(dest_file, handle->source_path);
				AddPart(dest_file, dest_name, 256);

				// Check destination is ok
				if ((stricmp(dest_name, entry->name)) == 0 ||
					(ret = check_file_destination(handle, entry, dest_file, &data->confirm_each)) == 1)
				{
					short suc = 0;

					// Loop while unsuccessful
					while (!suc)
					{
						// Got a lister entry?
						if (entry->entry)
						{
							// Use original function
							suc = OriginalRename(source_file, dest_file);
						}

						// Otherwise, allow patched function to be used
						else
						{
							suc = Rename(source_file, dest_file);
						}

						// Successful?
						if (suc)
						{
							ret = 1;
							break;
						}

						// Display error
						if (!(ret = function_error(handle, entry->name, MSG_RENAMING, IoErr())) || ret == -1)
							break;

						if (asked)
							goto ask_point;
					}
				}

				// Successful?
				if (ret == 1)
				{
					BPTR lock;

					// Mark this entry for removal
					entry->flags |= FUNCENTF_REMOVE;

					// Lock the new file
					if ((lock = Lock(dest_file, ACCESS_READ)))
					{
						// Examine it
#ifdef USE_64BIT
						ExamineLock64(lock, (FileInfoBlock64 *)handle->s_info);
#else
						Examine(lock, handle->s_info);
#endif
						UnLock(lock);

						// Was it a link?
						if (entry->entry && entry->entry->de_Flags & ENTF_LINK)
						{
							// Get new name from user string
							strcpy(handle->s_info->fib_FileName, FilePart(dest_file));

							// Set type appropriately
							handle->s_info->fib_DirEntryType =
								(handle->s_info->fib_DirEntryType < 0) ? ST_LINKFILE : ST_LINKDIR;
						}

						// Add new file to listers
						function_filechange_addfile(handle, path->path, handle->s_info, 0, 0);

						// Is it a directory?
						if (handle->s_info->fib_DirEntryType > 0)
						{
							// Make sure paths are terminated
							AddPart(source_file, "", 256);
							AddPart(dest_file, "", 256);

							// Rename any buffers showing this
							function_filechange_rename(handle, source_file, dest_file);
						}
					}
				}

				// File has failed
				else
					file_ok = 0;
			}
		}

		// Aborted?
		if (ret == -1)
		{
			function_abort(handle);
			ret = 0;
			break;
		}

		// Get next entry, increment count
		count += function_end_entry(handle, entry, file_ok);

		// Reset result code
		ret = 1;
	}

	// Free data
	FreeVec(source_file);

	return ret;
}

// Given a source and destination "pattern" and an old name, build a new name
BOOL rename_get_wild(char *src_pattern, char *dst_pattern, char *old_name, char *new_name)
{
	char *buffer;
	char *src_prefix, *src_suffix;
	char *dst_prefix, *dst_suffix;
	char *keep_bit;
	int len, prefix_len = 0, suffix_len = 0;
	int a;

	if (!(buffer = AllocVec(700, 0)))
		return 0;
	src_prefix = buffer;
	src_suffix = src_prefix + 140;
	dst_prefix = src_suffix + 140;
	dst_suffix = dst_prefix + 140;
	keep_bit = dst_suffix + 140;

	// Initialise source prefix and suffix
	src_prefix[0] = 0;
	src_suffix[0] = 0;

	// Get length of source pattern
	len = strlen(src_pattern);

	// Look for * in source pattern
	for (a = 0; a < len; a++)
	{
		// Found a *
		if (src_pattern[a] == '*')
		{
			// Get prefix (bit before *)
			strcpy(src_prefix, src_pattern);
			prefix_len = a;
			src_prefix[prefix_len] = 0;

			// Get suffix (bit after *)
			strcpy(src_suffix, src_pattern + a + 1);
			suffix_len = len - a - 1;
			break;
		}
	}

	// Look for * in destination pattern
	for (a = 0; dst_pattern[a]; a++)
	{
		// Found a *
		if (dst_pattern[a] == '*')
		{
			// Get prefix
			strcpy(dst_prefix, dst_pattern);
			dst_prefix[a] = 0;

			// Get suffix
			strcpy(dst_suffix, dst_pattern + a + 1);
			break;
		}
	}

	// Get old name length
	len = strlen(old_name);

	// If no prefix, or the old name matches the prefix
	if ((prefix_len == 0 || (strnicmp(old_name, src_prefix, prefix_len)) == 0) &&

		// If no suffix, or old name matches the suffix
		(suffix_len == 0 || (len >= suffix_len && (stricmp(&old_name[len - suffix_len], src_suffix)) == 0)))
	{
		// Get length of bit to keep
		len -= prefix_len + suffix_len;

		// Copy bit we are keeping
		CopyMem(old_name + prefix_len, keep_bit, len);
		keep_bit[len] = 0;

		// Build new name
		strcpy(new_name, dst_prefix);
		strcat(new_name, keep_bit);
		strcat(new_name, dst_suffix);

		// New name is valid?
		if (new_name[0] != 0)
		{
			FreeVec(buffer);
			return 1;
		}
	}

	// Invalid (didn't match)
	FreeVec(buffer);
	return 0;
}
