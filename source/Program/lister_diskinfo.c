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

void lister_diskinfo(Lister *lister)
{
	// Do we have a device list?
	if (lister->cur_buffer->more_flags & DWF_DEVICE_LIST)
	{
		// In icon mode?
		if (lister->flags & LISTERF_VIEW_ICONS)
		{
			BackdropObject *object;

			// Lock icon list
			lock_listlock(&lister->backdrop_info->objects, FALSE);

			// Go through, look for selected disks
			for (object = (BackdropObject *)lister->backdrop_info->objects.list.lh_Head; object->node.ln_Succ;
				 object = (BackdropObject *)object->node.ln_Succ)
			{
				// Selected disk? :)
				if (object->type == BDO_DISK && object->state && !(object->flags & (BDOF_ASSIGN | BDOF_CACHE)))
				{
					// Launch info for this disk
					function_launch(
						FUNCTION_RUN_FUNCTION, def_function_diskinfo, 0, 0, 0, 0, object->device_name, 0, 0, 0, 0);
				}
			}

			// Unlock icon list
			unlock_listlock(&lister->backdrop_info->objects);
		}

		return;
	}

	// Launch on current lister
	function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_diskinfo, lister);
}
