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

// RUN internal function
DOPUS_FUNC(function_runprog)
{
	FunctionEntry *entry;
	BPTR lock, old;

	// Lock source path
	if (!(lock = Lock(handle->source_path, ACCESS_READ)))
		return 1;

	// Change directory
	old = CurrentDir(lock);

	// Build list of sources
	while ((entry = function_get_entry(handle)))
	{
		struct DiskObject *icon;
		BPTR file;

		// Does file have an icon? (don't check if not allowed to)
		if (!(handle->flags & FUNCF_RUN_NO_ICONS))
    	{
      		icon = GetDiskObject(entry->name);
      		if (!icon)
      		{
          		if (IconBase->lib_Version >= 44)
          		{
             		//attempt to get deficons default tool
             		icon = GetIconTags(entry->name, ICONGETA_FailIfUnavailable, FALSE, TAG_DONE);
          		}
      		}
      		if (icon)
      		{
        		Lister *lister;
        		char *tool = 0;
        		BOOL done = 0;

        		// Does icon have a tool?
        		if (icon->do_Type == WBPROJECT && icon->do_DefaultTool && icon->do_DefaultTool[0])
        		{
          			// Test 'more' trap
          			if (file_trap_more(entry->name, icon->do_DefaultTool))
            			done = 1;

          			// Get pointer to tool name
          			else
	            		tool = FilePart(icon->do_DefaultTool);
        		}

    		    // Get pointer to file name
        		else
          			tool = FilePart(entry->name);

        		// Not run trapped more?
        		if (!done)
        		{
          			// Try to get current lister
          			if ((lister = function_lister_current(&handle->source_paths)))
          			{
            			// Set screen title
            			if (lister->window)
            			{
            				char buf[80];

              				if (tool)
                				lsprintf(buf, GetString(&locale, MSG_LAUNCHING_PROGRAM), tool);
              				title_error(buf, 0);
            			}
          			}

          			// Build name with quotes
          			lsprintf(handle->work_buffer, "\"%s\"", entry->name);

          			// Launch program
          			WB_LaunchNew(handle->work_buffer, GUI->screen_pointer, 0, environment->env->default_stack, tool);
        		}

        		// Free icon
        		FreeDiskObject(icon);
      		}
		}

		// No; try to open file
		else if ((file = Open(entry->name, MODE_OLDFILE)))
		{
			char *command, *output, *args;
			long test;

			// Read first long
			Read(file, (char *)&test, sizeof(long));
			Close(file);
#ifdef __AROS__
			test = AROS_BE2LONG(test);
#endif

			// Pointer to command buffer
			command = handle->work_buffer;
			*command = 0;

			// Pointer to argument buffer
			args = handle->work_buffer + 824;
			*args = 0;

			// Pointer to output buffer
			output = handle->work_buffer + 904;
			lsprintf(output,
					 "%s%s/AUTO/CLOSE/WAIT/SCREEN %s",
					 environment->env->output_device,
					 environment->env->output_window,
					 get_our_pubscreen());

			// Is file executable?
#ifdef __AROS__
			if (test == 0x7f454c46)
#else
			if ((test == 0x000003f3) || (test == 0x7f454c46))
#endif
			{
				// Build requester text
				lsprintf(handle->work_buffer + 512, GetString(&locale, MSG_ENTER_ARGUMENTS_FOR), entry->name);

				// Display requester
				if (function_request(handle,
									 handle->work_buffer + 512,
									 SRF_BUFFER,
									 args,
									 79,
									 GetString(&locale, MSG_OKAY),
									 GetString(&locale, MSG_CANCEL),
									 0))
				{
					// Build command command
					strcpy(command, "\"");
					strcat(command, entry->name);
					strcat(command, "\"");

					// Add arguments
					if (*args)
					{
						*(args - 1) = ' ';
						strcat(command, args - 1);
					}
				}
			}

			// Otherwise try to lock file
			else if ((file = Lock(entry->name, ACCESS_READ)))
			{
				// Examine file
				Examine(file, handle->s_info);
				UnLock(file);

				// Is S bit set?
				if (handle->s_info->fib_Protection & FIBF_SCRIPT)
				{
					// Execute script
					strcpy(command, "execute \"");
					strcat(command, entry->name);
					strcat(command, "\"");
				}
			}

			// Have we a command?
			if (*command)
			{
				// Open output
				if (!(file = Open(output, MODE_OLDFILE)))
					file = Open("nil:", MODE_OLDFILE);

				// Run command
				CLI_Launch(
					command, (struct Screen *)-1, 0, file, 0, LAUNCHF_USE_STACK, environment->env->default_stack);
			}
		}

		// Get next entry
		function_end_entry(handle, entry, 1);
	}

	// Restore directory
	CurrentDir(old);

#ifdef __amigaos4__
	// On FAT32 USB devices, the filesystem handler may invalidate locks
	// if the device is removed or the handler encounters an error.
	// Calling UnLock() on a stale lock causes a DSI crash in dos.library.
	{
		char test_buf[4];
		if (NameFromLock(lock, test_buf, sizeof(test_buf)) || IoErr() != ERROR_INVALID_LOCK)
			UnLock(lock);
	}
#else
	UnLock(lock);
#endif

	return 1;
}
