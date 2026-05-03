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

// Get devices for backdrop (list must be locked)
short backdrop_get_devices(BackdropInfo *info)
{
	struct List dos_list;
	DosListEntry *dos_entry;
	BackdropObject *object;
	short change = 0;
	APTR memory;

	// Get memory handle
	if (!(memory = NewMemHandle(1024, 256, MEMF_CLEAR)))
		return 0;

	// Get a copy of the dos list
	GetDosListCopy(&dos_list, memory);

	// Go through the dos list
	for (dos_entry = (DosListEntry *)dos_list.lh_Head; dos_entry->dle_Node.ln_Succ;
		 dos_entry = (DosListEntry *)dos_entry->dle_Node.ln_Succ)
	{
		struct List *search;
		D_S(struct InfoData, infodata)
		char device_name[80];
		ULONG disk_type = 0;

		// If this is a device, skip it if the 'hide bad' flag is set
		if (dos_entry->dle_Node.ln_Type == DLT_DEVICE && environment->env->display_options & DISPOPTF_HIDE_BAD)
			continue;

		// Invalid task (means it's PAR, SER, etc)?
		if (!dos_entry->dle_DosList.dol_Task)
			continue;

		// Is this a device?
		if (dos_entry->dle_Node.ln_Type == DLT_DEVICE)
		{
			DosListEntry *entry;
			struct DosList *dos;

			// Go through dos list
			for (entry = (DosListEntry *)dos_list.lh_Head; entry->dle_Node.ln_Succ;
				 entry = (DosListEntry *)entry->dle_Node.ln_Succ)
			{
				// Is there a volume matching this task?
				if (entry->dle_Node.ln_Type == DLT_VOLUME &&
					entry->dle_DosList.dol_Task == dos_entry->dle_DosList.dol_Task)
					break;
			}

			// If we found a volume, it will be picked up anyway
			if (entry->dle_Node.ln_Succ)
				continue;

			// Add colon to device name
			lsprintf(device_name, "%b:", dos_entry->dle_DosList.dol_Name);

			// Get disk info, check there's a disk in the drive
			if (!(GetDiskInfo(device_name, infodata)) || infodata->id_DiskType == ID_NO_DISK_PRESENT)
				continue;

			// Skip this device if it's in the hidden list, or the 'bad' hidden list
			if (desktop_find_hidden(device_name, FALSE) || desktop_find_hidden(device_name, TRUE))
				continue;

			// Skip this device if there's a good disk using the unit by another name
			if (!backdrop_goodbad_check(device_name, &dos_list))
				continue;

			// Get type from DOS list if set, otherwise from Info structure
			if ((dos = (struct DosList *)BADDR(infodata->id_VolumeNode)) && dos->dol_misc.dol_volume.dol_DiskType)
				disk_type = dos->dol_misc.dol_volume.dol_DiskType;
			else
				disk_type = infodata->id_DiskType;

			// See if disk is already in list
			search = &info->objects.list;
			while ((object = (BackdropObject *)FindName(search, device_name)))
			{
				// Bad disk object?
				if (object->type == BDO_BAD_DISK)
				{
					// Compare types; if it exists, we won't get it again later
					if (disk_type == object->misc_data)
					{
						// Mark entry as ok
						object->flags |= BDOF_OK;
					}
					break;
				}

				// Keep searching from this object
				search = (struct List *)object;
			}
		}

		// Must be a volume
		else
		{
			// Get device name
			DeviceFromHandler(dos_entry->dle_DosList.dol_Task, device_name);

			// No device name given?
			if (!*device_name)
			{
				// Build device name from volume name
				lsprintf(device_name, "%b:", dos_entry->dle_DosList.dol_Name);
			}

			// Skip this volume if it's in the hidden list
			if (*device_name && desktop_find_hidden(device_name, FALSE))
				continue;

			// See if volume is in list
			search = &info->objects.list;
			while ((object = (BackdropObject *)FindName(search, dos_entry->dle_Node.ln_Name)))
			{
				// Disk object?
				if (object->type == BDO_DISK)
				{
					// Compare date stamps
					if (CompareDates(&object->date, &dos_entry->dle_DosList.dol_misc.dol_volume.dol_VolumeDate) == 0)
					{
						// Mark entry as ok
						object->flags |= BDOF_OK;
						break;
					}
				}

				// Keep searching from this object
				search = (struct List *)object;
			}

			// Get type from DOS list if set, otherwise from Info structure
			if (dos_entry->dle_DosList.dol_misc.dol_volume.dol_DiskType)
				disk_type = dos_entry->dle_DosList.dol_misc.dol_volume.dol_DiskType;

			// Otherwise
			else
			{
				D_S(struct InfoData, infodata)

				// Get disk info and type from that
				if (GetDiskInfo(device_name, infodata))
					disk_type = infodata->id_DiskType;
			}
		}

		// If it was already in the list, continue with next entry
		if (object)
			continue;

		// Allocate new object
		if (!(object = backdrop_new_object(
				  info,
				  (dos_entry->dle_Node.ln_Type == DLT_DEVICE) ? device_name : dos_entry->dle_Node.ln_Name,
				  device_name,
				  (dos_entry->dle_Node.ln_Type == DLT_DEVICE) ? BDO_BAD_DISK : BDO_DISK)))
			return 0;

		// Get object date
		if (dos_entry->dle_Node.ln_Type == DLT_VOLUME)
			object->date = dos_entry->dle_DosList.dol_misc.dol_volume.dol_VolumeDate;

		// Store disk type
		object->misc_data = disk_type;

		// Mark object as ok
		object->flags = BDOF_OK | BDOF_NO_POSITION;

		// If this is the ram disk, add it to the start of the list
		if (object->device_name && strcmp(object->device_name, "RAM:") == 0)
		{
			AddHead(&info->objects.list, &object->node);
		}

		// Add to backdrop list
		else
			AddTail(&info->objects.list, &object->node);

		// Set change flag
		change |= BDEVF_DISK_ADDED;
	}

	// Free dos list memory
	FreeMemHandle(memory);

	// Go through backdrop list
	for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;)
	{
		BackdropObject *next = (BackdropObject *)object->node.ln_Succ;

		// Is this a disk?
		if (object->type == BDO_DISK || object->type == BDO_BAD_DISK)
		{
			// Is object marked ok?
			if (object->flags & BDOF_OK)
			{
				// Clear ok flag
				object->flags &= ~BDOF_OK;

				// Does object not have an icon?
				if (!object->icon)
				{
					// Get icon for object
					backdrop_get_icon(info, object, GETICON_CD);

					// If there's still no icon, remove it
					if (!object->icon)
					{
						// Remove and free object
						backdrop_remove_object(info, object);
					}
				}
			}

			// Any disks not marked as OK get removed
			else
			{
				// Erase object
				if (object->icon)
					backdrop_erase_icon(info, object, 0);

				// Remove and free object
				backdrop_remove_object(info, object);

				// Set change flag
				change |= BDEVF_DISK_REMOVED;
			}
		}

		// Get next object
		object = next;
	}

	return change;
}

// Refresh drives
void backdrop_refresh_drives(BackdropInfo *info, short flags)
{
	short change;

	// Lock backdrop list
	lock_listlock(&info->objects, 1);

	// Refresh the device list
	change = backdrop_get_devices(info);

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Update display?
	if (info->window && flags & BDEVF_SHOW && change > 0)
	{
		// Rubber-banding?
		if (info->flags & (BDIF_DRAGGING | BDIF_RUBBERBAND))
		{
			struct IntuiMessage msg;

			// Fake right button
			msg.Class = IDCMP_MOUSEBUTTONS;
			msg.Code = MENUDOWN;
			backdrop_handle_button(info, &msg, 0);
		}

		// New icons need rearranging
		if (change & BDEVF_DISK_ADDED)
			backdrop_arrange_objects(info);

		// Show icons
		backdrop_show_objects(info, BDSF_RECALC);
	}
}

static char *drive_equiv_name[] = {"DF", "PC", "MC", "DS", "FS", 0};

// Check for PC0/DF0 combination, etc
BOOL backdrop_goodbad_check(char *device, struct List *dos_list)
{
	DosListEntry *entry;
	short equiv;

	// Find the device we're testing against
	for (equiv = 0; drive_equiv_name[equiv]; equiv++)
		if (device[0] == drive_equiv_name[equiv][0] && device[1] == drive_equiv_name[equiv][1] && device[3] == ':')
			break;

	// Not found?
	if (!drive_equiv_name[equiv])
		return TRUE;

	// Go through dos list
	for (entry = (DosListEntry *)dos_list->lh_Head; entry->dle_Node.ln_Succ;
		 entry = (DosListEntry *)entry->dle_Node.ln_Succ)
	{
		short match;

		// Skip devices, we're only looking for volumes
		if (entry->dle_Node.ln_Type == DLT_DEVICE)
			continue;

		// See if this device matches one we know about
		for (match = 0; drive_equiv_name[match]; match++)
			if (entry->dle_DeviceName[0] == drive_equiv_name[match][0] &&
				entry->dle_DeviceName[1] == drive_equiv_name[match][1] && entry->dle_DeviceName[3] == ':')
				break;

		// No match, or the same as us?
		if (!drive_equiv_name[match] || match == equiv)
			continue;

		// Compare the unit number
		if (entry->dle_DeviceName[2] == device[2])
		{
			// Unit is the same; this is a valid disk so we don't show a bad icon
			return FALSE;
		}
	}

	return TRUE;
}
