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

#define DELETE_UPDATE_TIME 2

typedef struct
{
	short valid;
	short confirm_each;
	short unprotect_all;
	short skip_all;
} DeleteData;

enum {
	DELARG_NAME,
	DELARG_QUIET,
};

static char *delfile = "DeleteFile";

// DELETE internal function
DOPUS_FUNC(function_delete)
{
	FunctionEntry *entry;
	short ret = 1;
	long count = 1;
	char *source_file;
	PathNode *path;
	DeleteData *data;
	BOOL change_info = 0, del_trash = 0;
	CommandList *delete_cmd;
	TimerHandle *timer = 0;
	ULONG side;
	char *ptr;

	// Find the delete command
	ptr = delfile;
	delete_cmd = function_find_internal(&ptr, 0);

	// Get data and initialise if necessary
	data = (DeleteData *)handle->inst_data;
	if (!data->valid)
	{
		data->valid = 1;
		data->confirm_each = 1;
	}

	// Quiet?
	if (instruction->funcargs && instruction->funcargs->FA_Arguments[DELARG_QUIET])
		data->confirm_each = 0;

	// Allocate memory for source path
	if (!(source_file = AllocVec(512, MEMF_CLEAR)))
		return 0;

	// Run from icon mode?
	if (handle->flags & FUNCF_ICONS)
	{
		// Get first entry
		if ((entry = function_get_entry(handle)))
		{
			// Is is the trashcan?
			if (stricmp(entry->name, "trashcan") == 0)
			{
				struct DiskObject *icon;

				// Build source name
				function_build_source(handle, entry, source_file);

				// Get icon
				if ((icon = GetDiskObject(source_file)))
				{
					// Is it really the trashcan?
					if (icon->do_Type == WBGARBAGE)
					{
						// Set flag
						del_trash = 1;
					}

					// Free icon
					FreeDiskObject(icon);
				}
			}
		}
	}

	// Ask before commencing?
	if (del_trash || (environment->env->settings.delete_flags & DELETE_ASK && data->confirm_each))
	{
		short ret;

		// Deleting trashcan?
		if (del_trash)
		{
			// Get requester text
			strcpy(handle->temp_buffer, GetString(&locale, MSG_REALLY_EMPTY_TRASH));
		}

		// Or, just deleting a shortcut?
		else if (handle->file_count == handle->link_file_count && handle->dir_count == handle->link_dir_count)
		{
			// Build text
			strcpy(handle->temp_buffer,
				   GetString(&locale,
							 (handle->file_count + handle->dir_count < 2) ? MSG_DELETE_LINK_CONFIRM_SINGLE
																		  : MSG_DELETE_LINK_CONFIRM_MULTI));
		}

		// Build requester text
		else
			lsprintf(
				handle->temp_buffer, GetString(&locale, MSG_DELETE_CONFIRM), handle->file_count, handle->dir_count);

		// Display requester
		if (!(ret = function_request(handle,
									 handle->temp_buffer,
									 0,
									 GetString(&locale, MSG_PROCEED),
									 GetString(&locale, MSG_DELETE_ALL),
									 GetString(&locale, MSG_CANCEL),
									 0)))
		{
			function_abort(handle);
			FreeVec(source_file);
			return 0;
		}

		// Delete all?
		if (ret == 2)
			data->confirm_each = 0;
	}

	// Get current path
	if ((path = function_path_current(&handle->source_paths)))
	{
		// Tell this lister to update it's datestamp at the end
		path->flags |= LISTNF_UPDATE_STAMP;

		// Check desktop
		if (HookMatchDesktop(path->path_buf))
			handle->flags |= FUNCF_RESCAN_DESKTOP;
	}

	// Set flags
	handle->instruction_flags = INSTF_RECURSE_DIRS | INSTF_WANT_DIRS | INSTF_WANT_DIRS_END | INSTF_DIR_CLEAR_SIZES;

	// Turn progress indicator on
	function_progress_on(handle,
						 GetString(&locale, MSG_PROGRESS_OPERATION_DELETING),
						 handle->entry_count,
						 PWF_FILENAME | PWF_GRAPH | PWF_INFO);

	// Build info string
	function_build_info(handle, 0, 0, 0);

	// Update free space?
	if (environment->env->settings.copy_flags)
	{
		// Valid lister?
		if (path && path->lister && lister_valid_window(path->lister))
		{
			// Create timer for regular updates
			if ((timer = AllocTimer(UNIT_VBLANK, 0)))
				StartTimer(timer, DELETE_UPDATE_TIME, 0);
		}
	}

	// Go through entries
	while ((entry = function_get_entry(handle)))
	{
		BOOL file_ok = 1;

		// Update?
		if (timer && CheckTimer(timer))
		{
			side = (path->flags & LISTNF_DUAL_SIDE) ? path->dual_side + 1 : 0;

			// Send update message
			IPC_Command(path->lister->ipc, LISTER_REFRESH_NAME, side, 0, 0, 0);

			// Restart timer
			StartTimer(timer, DELETE_UPDATE_TIME, 0);
		}

		// Update progress indicator
		if (function_progress_update(handle, entry, count))
		{
			function_abort(handle);
			ret = 0;
			break;
		}

		// Build source name
		function_build_source(handle, entry, source_file);

		// Change info string?
		if (change_info)
		{
			char *file, ch = 0;

			// Clear filename
			if ((file = FilePart(source_file)))
			{
				ch = *file;
				*file = 0;
			}

			// Build info string
			function_build_info(handle, source_file, 0, 0);
			change_info = 0;

			// Restore source filename
			if (file)
				*file = ch;
		}

		// Check this isn't an exited directory
		if (!(entry->flags & FUNCENTF_EXITED))
		{
			// Confirm each is on?
			if (data->confirm_each)
			{
				BOOL ask = 1;

				// Got a directory?
				if (entry->type > 0)
					change_info = 1;

				// Is entry a top-level directory?
				if (entry->flags & FUNCENTF_TOP_LEVEL && entry->type > 0 && !(entry->flags & FUNCENTF_LINK) &&
					environment->env->settings.delete_flags & DELETE_DIRS)
				{
					// Skip if the trashcan
					if (del_trash && stricmp(entry->name, "trashcan") == 0)
						ask = 0;

					// Build requester text
					else
						lsprintf(
							handle->work_buffer, GetString(&locale, MSG_DELETE_DIR_CONFIRM), FilePart(source_file));
				}

				// Ask for confirmation for normal file?
				else if ((entry->flags & FUNCENTF_LINK || entry->type < 0) && !(entry->flags & FUNCENTF_ICON) &&
						 environment->env->settings.delete_flags & DELETE_FILES)
				{
					// Build requester text
					lsprintf(handle->work_buffer, GetString(&locale, MSG_DELETE_FILE_CONFIRM), FilePart(source_file));
				}

				// Otherwise don't need to ask
				else
					ask = 0;

				// Display requester if necessary
				if (ask)
				{
					// Show requester
					if ((ret = function_request(handle,
												handle->work_buffer,
												0,
												GetString(&locale, MSG_DELETE),
												GetString(&locale, MSG_DELETE_ALL),
												GetString(&locale, MSG_SKIP),
												GetString(&locale, MSG_ABORT),
												0)) == 0 ||
						ret == -1)
					{
						// Abort
						ret = -1;
					}

					// Delete all?
					else if (ret == 2)
					{
						ret = 1;
						data->confirm_each = 0;
					}

					// Skip?
					else if (ret == 3)
						ret = 0;
				}
			}
		}

		// It was a directory
		else
		{
			// Set flag to change info display
			change_info = 1;

			// If this is the trashcan, don't really delete the directory
			if (del_trash && stricmp(entry->name, "trashcan") == 0)
				ret = 0;
		}

		// Ok to delete?
		if (ret == 1)
		{
			short
			try
				= 0;

			// Directory?
			if (entry->type > 0 && !(entry->flags & FUNCENTF_EXITED) && !(entry->flags & FUNCENTF_LINK))
			{
				// Skip this entry
				ret = 2;
				file_ok = 1;
			}

			// Loop while unsuccessful
			else
				while (ret)
				{
					short err_code;

					// Delete file
					ret = DOpusDeleteFile(source_file, handle, delete_cmd, (entry->entry) ? 1 : 0);

					// Increment try count
					++
					try
						;

					// Get error code
					err_code = IoErr();

					// Successful?
					if (ret || err_code == ERROR_OBJECT_NOT_FOUND)
					{
						ret = 1;
						break;
					}

					// Object in use, less than 3 tries?
					if (err_code == ERROR_OBJECT_IN_USE && try < 3)
					{
						// Wait for a tick and then try again
						Delay(15);
						ret = 1;
						continue;
					}

					// If we failed, skip all?
					if (data->skip_all)
						break;

					// Delete protected?
					if (err_code == ERROR_DELETE_PROTECTED)
					{
						// Ask before unprotecting?
						if (!data->unprotect_all)
						{
							// Build requester text
							lsprintf(
								handle->work_buffer, GetString(&locale, MSG_DELETE_PROTECTED), FilePart(source_file));

							// Display request
							if (!(ret = function_request(handle,
														 handle->work_buffer,
														 0,
														 GetString(&locale, MSG_UNPROTECT),
														 GetString(&locale, MSG_UNPROTECT_ALL),
														 GetString(&locale, MSG_SKIP),
														 GetString(&locale, MSG_SKIP_ALL),
														 GetString(&locale, MSG_ABORT),
														 0)) ||
								ret == -1)
							{
								// Abort
								ret = -1;
								break;
							}

							// Skip?
							else if (ret == 3)
							{
								ret = 0;
								break;
							}

							// Skip all
							else if (ret == 4)
							{
								ret = 0;
								data->skip_all = 1;
								break;
							}

							// Unprotect all?
							else if (ret == 2)
								data->unprotect_all = 1;
						}

						// Try to unprotect file
						if ((ret = SetProtection(source_file, 0)))
							ret = 1;
					}

					// Another error
					else
					{
						// Directory not empty with recursive filter on
						if (err_code == ERROR_DIRECTORY_NOT_EMPTY && handle->got_filter == 2)
						{
							// Skip
							ret = 0;
						}

						// Otherwise
						else if ((ret = function_error(handle, entry->name, MSG_DELETING, err_code)) == -1)
							break;
					}
				}

			// Successful?
			if (ret == 1)
				entry->flags |= FUNCENTF_REMOVE;
			else if (ret != 2)
				file_ok = 0;
		}

		// Skip
		else
			file_ok = 0;

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

	// Free timer
	FreeTimer(timer);

	// Free data
	FreeVec(source_file);

	return ret;
}

// Call the internal delete file function
long DOpusDeleteFile(char *name, FunctionHandle *handle, CommandList *cmd, BOOL orig)
{
	char *ptr;
	CommandList *command;
	ULONG flags = 0;
	short ret;

	// Got handle?
	if (handle)
	{
		// Backup flags
		flags = handle->instruction_flags;

		// Set 'original' flag
		if (orig)
			handle->special_flags |= 1;
		else
			handle->special_flags &= ~1;
	}

	// Find the command if needed
	ptr = delfile;
	if (!(command = cmd) && !(command = function_find_internal(&ptr, 0)))
	{
		// Failed; call internal function directory
		ret = function_deletefile(0, handle, name, 0);
	}

	// Got command
	else
	{
		// Run the command
		ret = function_internal_command(command, name, handle, 0);
	}

	// Restore flags if we have a handle
	if (handle)
		handle->instruction_flags = flags;
	return ret;
}

// Standard internal delete function
DOPUS_FUNC(function_deletefile)
{
	char *ptr;
	short ret;

	// .info file?
	if ((ptr = sufcmp(args, ".info")) && strlen(FilePart(args)) < 31)
	{
		// Strip .info
		*ptr = 0;

		// Delete icon
		ret = DeleteDiskObject(args);

		// Restore .info
		*ptr = '.';
	}

	// Call original function?
	else if (handle && handle->special_flags & 1)
	{
		// Delete with original function
		ret = OriginalDeleteFile(args);
	}

	// Delete normal file, allow patch to be used
	else
		ret = DeleteFile(args);

	return ret;
}
