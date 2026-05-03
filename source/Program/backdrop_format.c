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

// Format disks
void backdrop_format(BackdropInfo *info, BackdropObject *icon)
{
	BackdropObject *object;
	short count = 0;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Disk?
		if (object->type != BDO_DISK && object->type != BDO_BAD_DISK)
			continue;

		// Selected or supplied?
		if ((!icon && object->state) || icon == object)
		{
			// Got a device name?
			if (object->device_name)
			{
				// Launch format
				function_launch(FUNCTION_RUN_FUNCTION,
								def_function_format,
								0,
								0,
								0,
								0,
								0,
								0,
								BuildArgArray((IPTR)object->device_name, 0),
								0,
								0);

				// Increment count
				++count;
			}

			// Icon supplied?
			if (icon)
				break;
		}
	}

	// Unlock backdrop
	unlock_listlock(&info->objects);

	// If we didn't launch any formats, run the format program
	if (count == 0)
	{
		// Launch format
		function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_format, 0);
	}
}
