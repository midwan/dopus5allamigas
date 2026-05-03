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

// Initialise reselection list
void InitReselect(ReselectionData *data)
{
	data->buffer = 0;
	data->memory = 0;
	NewList((struct List *)&data->files);
}

// Make entry reselection list
void MakeReselect(ReselectionData *data, DirBuffer *buffer, ULONG flags)
{
	DirEntry *entry, *top_entry = 0;
	long count;

	// Free existing reselection list
	ClearMemHandle(data->memory);
	NewList((struct List *)&data->files);

	// Invalid buffer, or a custom list?
	if (!buffer || list_is_custom(buffer))
		return;

	// Store buffer settings
	data->buffer = buffer;
	data->v_offset = buffer->buf_VertOffset;
	data->h_offset = buffer->buf_HorizOffset;
	data->flags = flags;

	// Allocate memory pool
	if (!data->memory && !(data->memory = NewMemHandle(2048, 300, MEMF_CLEAR)))
		return;

	// Go through file list
	for (entry = (DirEntry *)buffer->entry_list.mlh_Head, count = 0; entry->de_Node.dn_Succ;)
	{
		char *ptr = 0;

		// If we're saving filetypes we want all files with filetypes or selected,
		// otherwise just the selected entries
		if (entry->de_Flags & ENTF_SELECTED ||
			(data->flags & RESELF_SAVE_FILETYPES && (ptr = (char *)GetTagData(DE_Filetype, 0, entry->de_Tags))))
		{
			ReselectionFile *file;

			// Allocate new entry
			if ((file = AllocMemH(data->memory, sizeof(ReselectionFile) + strlen(entry->de_Node.dn_Name))))
			{
				// Fill out entry
				file->flags = entry->de_Flags;
				strcpy(file->name, entry->de_Node.dn_Name);
				file->date = entry->de_Date;

				// Save filetype?
				if (data->flags & RESELF_SAVE_FILETYPES && ptr)
				{
					// Copy filetype
					if ((file->filetype = AllocMemH(data->memory, strlen(ptr) + 1)))
						strcpy(file->filetype, ptr);
				}

				// Add to reselection list
				AddTail((struct List *)&data->files, (struct Node *)file);
			}
		}

		// Initial loop?
		if (count > -1)
		{
			// Is this the top entry?
			if (count == buffer->buf_VertOffset)
				top_entry = entry;

			// Last entry in list?
			if (!entry->de_Node.dn_Succ->dn_Succ)
			{
				// If we're saving filetypes, go through reject list too
				if (data->flags & RESELF_SAVE_FILETYPES)
				{
					entry = (DirEntry *)buffer->reject_list.mlh_Head;
					count = -1;
					continue;
				}
			}

			// Increment count
			++count;
		}

		// Get next entry
		entry = (DirEntry *)entry->de_Node.dn_Succ;
	}

	// Did we get a top entry?
	if (top_entry && (data->top_name = AllocMemH(data->memory, strlen(top_entry->de_Node.dn_Name) + 1)))
		strcpy(data->top_name, top_entry->de_Node.dn_Name);
	else
		data->top_name = 0;
}

// Reselect entries from a selection list
void DoReselect(ReselectionData *data, Lister *lister, BOOL no_move)
{
	long count = 0, top_pos = -1;
	DirBuffer *buffer;
	DirEntry *entry;
	ReselectionFile *file;

	// Check we have a valid buffer
	if (!(buffer = data->buffer) || lister->cur_buffer != buffer)
		return;

	// Go through directory
	for (entry = (DirEntry *)buffer->entry_list.mlh_Head; entry->de_Node.dn_Succ;
		 entry = (DirEntry *)entry->de_Node.dn_Succ, count++)
	{
		// If entry is selected, deselect it
		if (entry->de_Flags & ENTF_SELECTED)
		{
			entry->de_Flags &= ~ENTF_SELECTED;
			select_update_info(entry, buffer);
		}

		// Is this the file at the top?
		if (top_pos == -1 && data->top_name && (strcmp(entry->de_Node.dn_Name, data->top_name)) == 0)
			top_pos = count;
	}

	// Go through reselection list
	for (file = (ReselectionFile *)data->files.mlh_Head; file->node.mln_Succ;
		 file = (ReselectionFile *)file->node.mln_Succ)
	{
		// Find entry in list
		if ((entry = find_entry(&buffer->entry_list, file->name, 0, buffer->more_flags & DWF_CASE)))
		{
			// Is entry to be selected?
			if (file->flags & ENTF_SELECTED)
			{
				// Select entry
				entry->de_Flags |= ENTF_SELECTED;
				select_update_info(entry, buffer);
			}
		}
	}

	// Scroll to old position?
	if (!no_move)
	{
		if (top_pos < 0)
			top_pos = data->v_offset;
		buffer->buf_VertOffset = top_pos;
		buffer->buf_HorizOffset = data->h_offset;
	}
}

// Copy filetypes from reselection list
void GetReselectFiletypes(ReselectionData *reselect, struct MinList *file_list)
{
	ReselectionFile *file;
	DirEntry *entry;
	DirBuffer *buffer;

	// Get buffer
	buffer = reselect->buffer;

	// Go through reselection list
	for (file = (ReselectionFile *)reselect->files.mlh_Head; file->node.mln_Succ;
		 file = (ReselectionFile *)file->node.mln_Succ)
	{
		// Saved filetype?
		if (file->filetype)
		{
			// Look for entry in list
			if ((entry = find_entry(file_list, file->name, 0, buffer->more_flags & DWF_CASE)) &&
				!(GetTagData(DE_Filetype, 0, entry->de_Tags)))
			{
				// Check datestamps match
				if (CompareDates(&entry->de_Date, &file->date) == 0)
				{
					// Set filetype string in entry
					direntry_add_string(buffer, entry, DE_Filetype, file->filetype);
				}
			}
		}
	}
}

// Free reselection data
void FreeReselect(ReselectionData *data)
{
	// Free memory
	FreeMemHandle(data->memory);

	// Reinitialise
	InitReselect(data);
}
