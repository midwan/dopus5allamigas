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

DirEntry *ASM SAVEDS HookCreateFileEntry(REG(a0, Lister *lister),
										 REG(a1, struct FileInfoBlock *fib),
										 REG(d0, BPTR lock))
{
#ifdef USE_64BIT
	#warning detect if there is a valid fib_Size64 in the FIB
	((FileInfoBlock64 *)fib)->fib_Size64 = fib->fib_Size;
#endif

	// Create entry
	return create_file_entry_fib(lister->cur_buffer, lock, fib, 0, 0, 0, 0);
}

void ASM SAVEDS HookLockFileList(REG(a0, Lister *lister), REG(d0, BOOL exclusive))
{
	buffer_lock(lister->cur_buffer, exclusive);
}

void ASM SAVEDS HookUnlockFileList(REG(a0, Lister *lister))
{
	buffer_unlock(lister->cur_buffer);
}

DirEntry *ASM SAVEDS HookFindFileEntry(REG(a0, Lister *lister), REG(a1, char *name))
{
	DirEntry *entry;

	// Find entry
	if ((entry = find_entry(&lister->cur_buffer->entry_list, name, 0, lister->cur_buffer->more_flags & DWF_CASE)))
		return entry;
	return find_entry(&lister->cur_buffer->reject_list, name, 0, lister->cur_buffer->more_flags & DWF_CASE);
}

void ASM SAVEDS HookFileSet(REG(a0, Lister *lister), REG(a1, DirEntry *entry), REG(a2, struct TagItem *tags))
{
	struct TagItem *tag, *tstate;

	// Go through tags
	tstate = tags;
	while ((tag = NextTagItem(&tstate)))
	{
		// Look at tag
		switch (tag->ti_Tag)
		{
		// Name
		case HFFS_NAME:
			if (tag->ti_Data)
				stccpy(entry->de_Node.dn_Name, (char *)tag->ti_Data, entry->de_NameLen - 1);
			lister->more_flags |= LISTERF_NEED_RECALC;
			break;

		// Size
		case HFFS_SIZE:
#ifdef USE_64BIT
	#warning What about the high 32-bits?
			entry->de_Size = (UQUAD)tag->ti_Data;
#else
			entry->de_Size = tag->ti_Data;
#endif
			lister->more_flags |= LISTERF_NEED_RECALC;
			break;

		// Protection
		case HFFS_PROTECTION:
			entry->de_Protection = tag->ti_Data;
			protect_get_string(entry->de_Protection, entry->de_ProtBuf);
			break;

		// Date
		case HFFS_DATE:
			entry->de_Date = *((struct DateStamp *)tag->ti_Data);
			date_build_string(&entry->de_Date, entry->de_DateBuf, 1);
			break;

		// Comment
		case HFFS_COMMENT:
			direntry_add_string(lister->cur_buffer, entry, DE_Comment, (char *)tag->ti_Data);
			lister->more_flags |= LISTERF_NEED_RECALC;
			break;

		// Selected (flags)
		case HFFS_SELECTED:
			if (tag->ti_Data)
				entry->de_Flags |= ENTF_SELECTED;
			else
				entry->de_Flags &= ~ENTF_SELECTED;
			break;

		// Link (flags)
		case HFFS_LINK:
			if (tag->ti_Data)
				entry->de_Flags |= ENTF_LINK;
			else
				entry->de_Flags &= ~ENTF_SELECTED;
			break;

		// Colour
		case HFFS_COLOUR:
			entry->de_Flags &= ~(ENTF_COLOUR_DEVICE | ENTF_COLOUR_ASSIGN);
			if (tag->ti_Data == 1)
				entry->de_Flags |= ENTF_COLOUR_DEVICE;
			else if (tag->ti_Data == 2)
				entry->de_Flags |= ENTF_COLOUR_ASSIGN;
			break;

		// Userdata
		case HFFS_USERDATA:
			entry->de_UserData = tag->ti_Data;
			break;

		// Filetype
		case HFFS_FILETYPE:
			direntry_add_string(lister->cur_buffer, entry, DE_Filetype, (char *)tag->ti_Data);
			lister->more_flags |= LISTERF_NEED_RECALC;
			break;

		// Display string
		case HFFS_DISPLAY:
			direntry_add_string(lister->cur_buffer, entry, DE_DisplayString, (char *)tag->ti_Data);
			lister->more_flags |= LISTERF_NEED_RECALC;
			break;

		// Version info
		case HFFS_VERSION: {
			VersionInfo *info;

			// Get info
			if ((info = (VersionInfo *)tag->ti_Data))
			{
				// Add version
				direntry_add_version(lister->cur_buffer, entry, info->vi_Version, info->vi_Revision, info->vi_Days);
				lister->more_flags |= LISTERF_NEED_RECALC;
			}
		}
		break;

		// Popup menu
		case HFFS_MENU: {
			struct List *list, *from_list;
			struct Node *node, *from_node;
			struct TagItem *tags;

			// Get supplied list
			if ((from_list = (struct List *)tag->ti_Data))
			{
				// Allocate list copy and tags
				if ((list = AllocMemH(lister->cur_buffer->memory, sizeof(PopUpMenu))) &&
					(tags = AllocMemH(lister->cur_buffer->memory, sizeof(struct TagItem) * 2)))
				{
					// Set entry flag
					entry->de_Flags |= ENTF_USER_MENU;

					// Fill out tags
					tags[0].ti_Tag = DE_PopupMenu;
					tags[0].ti_Data = (ULONG)list;
					tags[1].ti_Tag = TAG_END;

					// Add tags to entry
					ChainTagItems(&entry->de_Tags, tags);

					// Initialise new list
					NewList((struct List *)list);

					// Go through list
					for (from_node = from_list->lh_Head; from_node->ln_Succ; from_node = from_node->ln_Succ)
					{
						// Allocate node copy
						if ((node = AllocMemH(lister->cur_buffer->memory, sizeof(struct Node))))
						{
							// Store name pointer
							node->ln_Name = from_node->ln_Name;

							// Add to list
							AddTail((struct List *)list, (struct Node *)node);
						}
					}
				}
			}

			// Remove menus
			else
			{
				struct TagItem *tag;

				// Clear flag
				entry->de_Flags &= ~ENTF_USER_MENU;

				// Find tag, change to ignore
				if ((tag = FindTagItem(DE_PopupMenu, entry->de_Tags)))
					tag->ti_Tag = TAG_IGNORE;
			}
		}
		break;
		}
	}
}

BOOL ASM SAVEDS HookFileQuery(REG(a0, Lister *lister), REG(a1, DirEntry *entry), REG(a2, struct TagItem *tags))
{
	struct TagItem *tag, *tstate;
	BOOL ret = 1;

	// Go through tags
	tstate = tags;
	while ((tag = NextTagItem(&tstate)))
	{
		// Look at tag
		switch (tag->ti_Tag)
		{
		// Name
		case HFFS_NAME:
			if (tag->ti_Data)
				strcpy((char *)tag->ti_Data, entry->de_Node.dn_Name);
			break;

		// Size
		case HFFS_SIZE:
			if (tag->ti_Data)
#ifdef USE_64BIT
	#warning What about the high 32-bits?
				*((ULONG *)tag->ti_Data) = (ULONG)(entry->de_Size & 0xffffffff);
#else
				*((ULONG *)tag->ti_Data) = entry->de_Size;
#endif
			break;

		// Protection
		case HFFS_PROTECTION:
			if (tag->ti_Data)
				*((ULONG *)tag->ti_Data) = entry->de_Protection;
			break;

		// Date
		case HFFS_DATE:
			if (tag->ti_Data)
				*((struct DateStamp *)tag->ti_Data) = entry->de_Date;
			break;

		// Comment
		case HFFS_COMMENT:
			if (tag->ti_Data)
			{
				char *ptr;

				if ((ptr = (char *)GetTagData(DE_Comment, 0, entry->de_Tags)))
					strcpy((char *)tag->ti_Data, ptr);
				else
					*((char *)tag->ti_Data) = 0;
			}
			break;

		// Selected (flags)
		case HFFS_SELECTED:
			if (tag->ti_Data)
				*((ULONG *)tag->ti_Data) = (entry->de_Flags & ENTF_SELECTED) ? 1 : 0;
			break;

		// Link (flags)
		case HFFS_LINK:
			if (tag->ti_Data)
				*((ULONG *)tag->ti_Data) = (entry->de_Flags & ENTF_LINK) ? 1 : 0;
			break;

		// Userdata
		case HFFS_USERDATA:
			if (tag->ti_Data)
				*((ULONG *)tag->ti_Data) = entry->de_UserData;
			break;

		// Filetype
		case HFFS_FILETYPE:
			if (tag->ti_Data)
			{
				char *ptr;

				if ((ptr = (char *)GetTagData(DE_Filetype, 0, entry->de_Tags)))
					strcpy((char *)tag->ti_Data, ptr);
				else
					*((char *)tag->ti_Data) = 0;
			}
			break;

		// Display string
		case HFFS_DISPLAY:
			if (tag->ti_Data)
			{
				char *ptr;

				if ((ptr = (char *)GetTagData(DE_DisplayString, 0, entry->de_Tags)))
					strcpy((char *)tag->ti_Data, ptr);
				else
					*((char *)tag->ti_Data) = 0;
			}
			break;

		// Version info
		case HFFS_VERSION:
			if (tag->ti_Data)
			{
				VersionInfo *ptr;

				// Clear pointer
				*((ULONG *)tag->ti_Data) = 0;

				// Get version info
				if ((ptr = (VersionInfo *)GetTagData(DE_VersionInfo, 0, entry->de_Tags)))
				{
					VersionInfo *copy;
					short size;

					// Copy version info
					if ((copy = AllocVec((size = sizeof(VersionInfo) + strlen(ptr->vi_String)), MEMF_CLEAR)))
					{
						// Copy it
						CopyMem((char *)ptr, (char *)copy, size);

						// Store pointer
						*((ULONG *)tag->ti_Data) = (ULONG)copy;
					}
				}
			}
			else
				ret = 0;
			break;

		// Popup menu
		case HFFS_MENU:
			ret = 0;
			break;

		// Something else
		default:
			ret = 0;
			break;
		}
	}

	return ret;
}

BOOL ASM SAVEDS HookSetFileComment(REG(a0, Lister *lister), REG(a1, char *name), REG(a2, char *comment))
{
	DirEntry *entry;

	// Lock list
	buffer_lock(lister->cur_buffer, TRUE);

	// Find entry
	if ((entry = HookFindFileEntry(lister, name)))
	{
		struct TagItem tags[2];

		// Fill out tags
		tags[0].ti_Tag = HFFS_COMMENT;
		tags[0].ti_Data = (ULONG)comment;
		tags[1].ti_Tag = TAG_DONE;

		// Set comment
		HookFileSet(lister, entry, tags);
	}

	// Unlock list
	buffer_unlock(lister->cur_buffer);

	// Return success/failure
	return (BOOL)((entry) ? 1 : 0);
}

void ASM SAVEDS HookSortFileList(REG(a0, Lister *lister),
								 REG(a1, struct List *list),
								 REG(d0, long file_count),
								 REG(d1, long dir_count))
{
	// Lock buffer
	buffer_lock(lister->cur_buffer, TRUE);

	// Sort directory
	buffer_sort_list(lister->cur_buffer, (struct MinList *)list, file_count, dir_count);

	// Set valid flag
	lister->cur_buffer->flags |= DWF_VALID;

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);
}

DirEntry *ASM SAVEDS HookAddFileEntry(REG(a0, Lister *lister), REG(a1, DirEntry *entry), REG(d0, BOOL sort))
{
	// Lock buffer
	buffer_lock(lister->cur_buffer, TRUE);

	// Add entry
	entry = add_file_entry(
		lister->cur_buffer, entry, (sort) ? 0 : ((DirEntry *)lister->cur_buffer->entry_list.mlh_TailPred));

	// Set valid flag
	lister->cur_buffer->flags |= DWF_VALID;

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);
	return entry;
}

void ASM SAVEDS HookResortLister(REG(a0, Lister *lister), REG(a1, ListFormat *format))
{
	// Format supplied?
	if (format)
	{
		// Store format in lister
		lister->format = *format;
	}

	// Resort the list
	lister_resort(lister, RESORT_SORT);
}

void ASM SAVEDS HookRefreshLister(REG(a0, Lister *lister), REG(d0, ULONG flags))
{
	ULONG refresh;

	// Default refresh
	refresh = REFRESHF_SLIDERS | REFRESHF_RESORT;

	// Full?
	if (flags & HOOKREFRESH_FULL)
		refresh |= REFRESHF_UPDATE_NAME | REFRESHF_STATUS;

	// Datestamp?
	else if (flags & HOOKREFRESH_DATE)
		refresh = REFRESHF_DATESTAMP;

	// Refresh the lister
	IPC_Command(lister->ipc, LISTER_REFRESH_WINDOW, refresh, 0, 0, 0);
}

void ASM SAVEDS HookRemoveFileEntry(REG(a0, Lister *lister), REG(a1, DirEntry *entry))
{
	removefile(lister->cur_buffer, entry);
}
