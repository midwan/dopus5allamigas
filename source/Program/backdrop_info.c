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

// Show info on backdrop objects
void backdrop_info(BackdropInfo *info, BOOL disk, BackdropObject *icon)
{
	BackdropObject *object;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
		 object = (BackdropObject *)object->node.ln_Succ)
	{
		// Is object selected, or one supplied?
		if ((!icon && object->state) || (icon == object))
		{
			// Bad disk?
			if (object->type == BDO_BAD_DISK)
				DisplayBeep(info->window->WScreen);

			// AppIcon?
			else if (object->type == BDO_APP_ICON)
			{
				// Does it support Info?
				if ((WB_AppIconFlags((struct AppIcon *)object->misc_data)) & APPENTF_INFO)
				{
					// Send info message
					backdrop_appicon_message(object, BAPPF_INFO);
				}

				// Fail
				else
					DisplayBeep(info->window->WScreen);
			}

			// Other
			else if (!disk || object->type == BDO_DISK)
			{
				BPTR old, lock;

				// Get icon lock
				if ((lock = backdrop_icon_lock(object)))
				{
					// Change directory
					old = CurrentDir(lock);

					// Show info
					misc_startup((disk) ? "dopus_disk_info" : "dopus_icon_info",
								 (disk) ? MENU_ICON_DISKINFO : MENU_ICON_INFO,
								 info->window,
								 (object->type == BDO_DISK) ? ":" : object->name,
								 0);

					// Restore directory
					CurrentDir(old);
					UnLock(lock);
				}
			}

			// Icon supplied?
			if (icon)
				break;
		}
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);
}
