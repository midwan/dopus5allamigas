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

// Get an entry
DirEntry *get_entry(struct MinList *list, BOOL selected, short type)
{
	DirEntry *entry;

	// Valid list?
	if (list)
	{
		// Go through list
		for (entry = (DirEntry *)list->mlh_Head; entry->de_Node.dn_Succ; entry = (DirEntry *)entry->de_Node.dn_Succ)
		{
			// See if entry matches selection requirements
			if (!selected || (entry->de_Flags & ENTF_SELECTED))
			{
				// See if entry matches type requirements
				if (type == ENTRY_ANYTHING || (type == 0 && entry->de_Node.dn_Type == 0) ||
					(type < 0 && entry->de_Node.dn_Type < 0) || (type > 0 && entry->de_Node.dn_Type > 0))
				{
					// Return entry
					return entry;
				}
			}
		}
	}

	// Not found
	return 0;
}

// Return first selected file
DirEntry *checktot(DirBuffer *list)
{
	return get_entry(&list->entry_list, 1, ENTRY_FILE);
}

// Return first selected directory
DirEntry *checkdirtot(DirBuffer *list)
{
	return get_entry(&list->entry_list, 1, ENTRY_DIRECTORY);
}

// Return first selected device
DirEntry *checkdevtot(DirBuffer *list)
{
	return get_entry(&list->entry_list, 1, ENTRY_DEVICE);
}

// First selected file/directory
DirEntry *checkalltot(DirBuffer *list)
{
	DirEntry *entry;

	// Valid list?
	if (!list)
		return 0;

	// Go through list
	for (entry = (DirEntry *)list->entry_list.mlh_Head; entry->de_Node.dn_Succ;
		 entry = (DirEntry *)entry->de_Node.dn_Succ)
	{
		// Selected file/directory?
		if (entry->de_Flags & ENTF_SELECTED && entry->de_Node.dn_Type != ENTRY_DEVICE)
			return entry;
	}

	return 0;
}

// Find an entry in a list
DirEntry *find_entry(struct MinList *list, char *name, long *offset, short case_sens)
{
	DirEntry *entry = 0;

	// Valid list?
	if (list && !(IsListEmpty((struct List *)list)))
	{
		char *icon_name = 0;

		// Look for icon?
		if (case_sens & FINDENTRY_ICON)
		{
			// Don't already have .info?
			if (!isicon(name))
			{
				// Allocate icon name
				if ((icon_name = AllocVec(strlen(name) + 6, 0)))
				{
					// Get name
					strcpy(icon_name, name);

					// Add .info suffix
					strcat(icon_name, ".info");

					// Use as new name pointer
					name = icon_name;
				}
			}
		}

		// Zero offset
		if (offset)
			*offset = 0;

		// Go through list
		for (entry = (DirEntry *)list->mlh_Head; entry->de_Node.dn_Succ; entry = (DirEntry *)entry->de_Node.dn_Succ)
		{
			// See if name matches
			if (case_sens & DWF_CASE)
			{
				if (strcmp(name, entry->de_Node.dn_Name) == 0)
					break;
			}
			else if (stricmp(name, entry->de_Node.dn_Name) == 0)
				break;

			// Increment offset
			if (offset)
				++(*offset);
		}

		// Invalid?
		if (!entry->de_Node.dn_Succ)
			entry = 0;

		// Free icon name
		if (icon_name)
			FreeVec(icon_name);
	}

	// Return entry
	return entry;
}

// Find an entry's offset in a list
long find_entry_offset(struct MinList *list, DirEntry *find)
{
	DirEntry *entry;
	long offset;

	// Valid list?
	if (!list || IsListEmpty((struct List *)list))
		return -1;

	// Go through list
	for (entry = (DirEntry *)list->mlh_Head, offset = 0; entry->de_Node.dn_Succ;
		 entry = (DirEntry *)entry->de_Node.dn_Succ, offset++)
	{
		// Is this the entry?
		if (entry == find)
			return offset;
	}

	// Not found
	return -1;
}

// Get an ordinal number in a list
DirEntry *get_entry_ord(struct MinList *list, long offset)
{
	DirEntry *entry;

	// Valid list or offset?
	if (offset < 0)
		return 0;

	// Get ordinal number in list
	for (entry = (DirEntry *)list->mlh_Head; offset > 0 && entry->de_Node.dn_Succ;
		 offset--, entry = (DirEntry *)entry->de_Node.dn_Succ)
		;

	// Return entry
	if (entry->de_Node.dn_Succ)
		return entry;
	return 0;
}

// See if a list contains "user-custom" entries
BOOL list_is_custom(DirBuffer *buffer)
{
	// Valid buffer?
	if (!buffer || (IsListEmpty((struct List *)&buffer->entry_list)))
		return 0;

	// Device list entry?
	if (((DirEntry *)buffer->entry_list.mlh_Head)->de_Node.dn_Type == ENTRY_DEVICE)
		return 1;

	// Normal entry
	return 0;
}

// Read parent/root of a window
// Called from the LISTER PROCESS
void do_parent_root(Lister *lister, char *path)
{
	// Read directory
	read_directory(lister, path, GETDIRF_CANMOVEEMPTY | GETDIRF_CANCHECKBUFS);
}
