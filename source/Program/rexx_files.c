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

// Handle an AppMessage
void rexx_custom_app_msg(DOpusAppMessage *msg,
						 DirBuffer *buffer,
						 char *action,
						 Lister *dest,
						 char *dest_path,
						 unsigned short qual)
{
	char *string;

	// Build string
	if ((string = rexx_build_filestring(msg, 0, buffer->cust_flags)))
	{
		// Send message
		rexx_handler_msg(0,
						 buffer,
						 RXMF_WARN,
						 HA_String,
						 0,
						 action,
						 HA_Value,
						 1,
						 buffer->buf_CurrentLister,
						 HA_String,
						 2,
						 string,
						 HA_Value,
						 3,
						 dest,
						 HA_String,
						 5,
						 dest_path,
						 HA_Qualifier,
						 6,
						 qual,
						 TAG_END);

		// Free string
		FreeVec(string);
	}
}

// Build a string of filenames
char *rexx_build_filestring(DOpusAppMessage *msg, IPTR *lister_ptr, ULONG flags)
{
	long len = 0, num;
	char buf[256], *string;
	IPTR lister = 0;

	// Do files come from a lister?
	get_appmsg_data(msg, &lister, 0, 0);
	if (lister_ptr)
		*lister_ptr = lister;

	// Go through arguments
	for (num = 0; num < msg->da_Msg.am_NumArgs; num++)
	{
		// Get path name
		GetWBArgPath(&msg->da_Msg.am_ArgList[num], buf, 256);

		// If a directory and not from a lister, add trailing /
		if ((!msg->da_Msg.am_ArgList[num].wa_Name || !*msg->da_Msg.am_ArgList[num].wa_Name) && !lister)
		{
			AddPart(buf, "", 256);
		}

		// Do files come from a lister, and don't want full paths?
		if (lister && !(flags & CUSTF_WANT_FULLPATH))
		{
			// Store basename only
			len += strlen(FilePart(buf)) + ((flags & CUSTF_WANT_QUOTES) ? 3 : 1);
		}

		// Otherwise store full length
		else
			len += strlen(buf) + ((flags & CUSTF_WANT_QUOTES) ? 3 : 1);
	}

	// Allocate string
	if (!(string = AllocVec(len, MEMF_CLEAR)))
		return 0;

	// Go through arguments
	for (num = 0; num < msg->da_Msg.am_NumArgs; num++)
	{
		// Get path name
		GetWBArgPath(&msg->da_Msg.am_ArgList[num], buf, 256);

		// If a directory and not from a lister, add trailing /
		if ((!msg->da_Msg.am_ArgList[num].wa_Name || !*msg->da_Msg.am_ArgList[num].wa_Name) && !lister)
		{
			AddPart(buf, "", 256);
		}

		// Quote name
		if (flags & CUSTF_WANT_QUOTES)
			strcat(string, "\"");

		// Do files come from a lister, and don't want full paths?
		if (lister && !(flags & CUSTF_WANT_FULLPATH))
		{
			// Store basename only
			strcat(string, FilePart(buf));
		}

		// Otherwise store full name length
		else
			strcat(string, buf);

		// End quote
		if (flags & CUSTF_WANT_QUOTES)
			strcat(string, "\"");

		// Space
		if (num < msg->da_Msg.am_NumArgs - 1)
			strcat(string, " ");
	}

	return string;
}

// Add a file to a lister
short rexx_lister_add_file(Lister *lister, char *args, struct RexxMsg *msg)
{
	char *data;
	char *name, *type_buf, *seconds_buf, *prot_buf, *comment_buf;
	char *description, *display_string, *menu_stem;
	long protection = 0;
	DirEntry *entry;
	unsigned short flags = 0;
	long size = 0;
	short type, selected = 0;
	struct DateStamp date;
	long version = -1, revision = 0, ver_days = 0;
	unsigned long userdata = 0;
	BOOL got_date = 0;

	// Allocate data
	if (!(data = AllocVec(1024, MEMF_CLEAR)))
		return RXERR_NO_MEMORY;

	// Get pointers
	name = data;
	type_buf = name + 256;
	seconds_buf = type_buf + 20;
	prot_buf = seconds_buf + 20;
	comment_buf = prot_buf + 20;
	description = comment_buf + 256;
	display_string = description + 80;
	menu_stem = display_string + 256;

	// Message supplied?
	if (msg)
	{
		char buffer[40], stem[30];

		// Get stem name
		rexx_parse_word(&args, stem, 30);

		// Check stem has a period
		if (stem[0] && stem[strlen(stem) - 1] != '.')
			strcat(stem, ".");

		// Get name
		rexx_get_var(msg, stem, "NAME", name, 256);

		// Get size
		if (rexx_get_var(msg, stem, "SIZE", buffer, 20))
			size = atoi(buffer);

		// Get type
		rexx_get_var(msg, stem, "TYPE", type_buf, 20);

		// Protection string?
		if (!(rexx_get_var(msg, stem, "PROTSTRING", prot_buf, 20)))
		{
			// Protection value?
			if (rexx_get_var(msg, stem, "PROTECT", buffer, 20))
			{
				// Store value
				protection = atoi(buffer);
			}
		}

		// Get comment
		rexx_get_var(msg, stem, "COMMENT", comment_buf, 256);

		// Got a date string?
		if (rexx_get_var(msg, stem, "DATESTRING", buffer, 40))
		{
			char date_b[30], time_b[30];

			// Current date?
			if (buffer[0] == '0' && buffer[1] == 0)
			{
				DateStamp(&date);
				got_date = 1;
			}
			else
			{
				// Convert to separate strings
				ParseDateStrings(buffer, date_b, time_b, 0);

				// Convert to datestamp
				if (DateFromStrings(date_b, time_b, &date))
					got_date = 1;
			}
		}

		// Get seconds
		else
			rexx_get_var(msg, stem, "DATE", seconds_buf, 20);

		// Get description (filetype)
		rexx_get_var(msg, stem, "FILETYPE", description, 80);

		// Get display string
		rexx_get_var(msg, stem, "DISPLAY", display_string, 256);

		// Get version and revision
		if (rexx_get_var(msg, stem, "VERSION", buffer, 10))
		{
			// Get version
			version = atoi(buffer);

			// Get revision
			if (rexx_get_var(msg, stem, "REVISION", buffer, 10))
				revision = atoi(buffer);

			// Get version date
			if (rexx_get_var(msg, stem, "VERDATE", buffer, 40))
			{
				struct DateTime datetime;

				// Initialist DateTime structure
				datetime.dat_Format = FORMAT_DOS;
				datetime.dat_Flags = 0;
				datetime.dat_StrDate = buffer;
				datetime.dat_StrTime = 0;

				// Convert to datestamp
				if (StrToDate(&datetime))
					ver_days = datetime.dat_Stamp.ds_Days;
			}
		}

		// Get user data
		if (rexx_get_var(msg, stem, "USERDATA", buffer, 15))
			userdata = atoi(buffer);

		// Get menu stem
		if (rexx_get_var(msg, stem, "MENU", menu_stem, 40))
		{
			// Check stem has a period
			if (menu_stem[0] && menu_stem[strlen(menu_stem) - 1] != '.')
				strcat(menu_stem, ".");
		}

		// Selected?
		if (rexx_get_var(msg, stem, "SELECTED", buffer, 5))
			selected = atoi(buffer);
	}

	// Parse from string
	else
	{
		char buffer[40];

		// Get name
		rexx_parse_word(&args, name, 256);

		// Get size
		rexx_parse_word(&args, buffer, 20);
		size = atoi(buffer);

		// Get type
		rexx_parse_word(&args, type_buf, 20);

		// Get seconds
		rexx_parse_word(&args, seconds_buf, 20);

		// Get protection
		rexx_parse_word(&args, prot_buf, 20);

		// Get comment
		rexx_skip_space(&args);
		stccpy(comment_buf, args, 256);

		// Clear buffers
		description[0] = 0;
	}

	// Get type
	if (type_buf[0])
	{
		// Get value
		type = atoi(type_buf);

		// -2 = device colour
		if (type == RXENTRY_FILE_COLOUR || type == RXENTRY_FILE_COLOUR_LINK)
		{
			if (type == RXENTRY_FILE_COLOUR_LINK)
				flags |= ENTF_LINK;
			type = RXENTRY_FILE;
			flags |= ENTF_COLOUR_DEVICE;
		}

		// 2 = assign colour
		else if (type == RXENTRY_DIR_COLOUR || type == RXENTRY_DIR_COLOUR_LINK)
		{
			if (type == RXENTRY_DIR_COLOUR_LINK)
				flags |= ENTF_LINK;
			type = RXENTRY_DIR;
			flags |= ENTF_COLOUR_ASSIGN;
		}

		// -3 = link
		else if (type == RXENTRY_FILE_LINK)
		{
			type = RXENTRY_FILE;
			flags |= ENTF_LINK;
		}

		// 3 = link
		else if (type == RXENTRY_DIR_LINK)
		{
			type = RXENTRY_DIR;
			flags |= ENTF_LINK;
		}
	}
	else
		type = RXENTRY_FILE;

	// Don't already have date?
	if (!got_date)
	{
		long seconds;

		// Get seconds
		if ((seconds = (seconds_buf[0]) ? atoi(seconds_buf) : ((struct IntuitionBase *)IntuitionBase)->Seconds) == 0)
			DateStamp(&date);
		else
		{
			// Convert seconds to datestamp
			date.ds_Days = seconds / 86400;
			seconds %= 86400;
			date.ds_Minute = seconds / 60;
			seconds %= 60;
			date.ds_Tick = seconds * TICKS_PER_SECOND;
		}
	}

	// Get protection
	if (prot_buf[0])
		protection = prot_from_string(prot_buf);

	// Lock current buffer
	buffer_lock(lister->cur_buffer, TRUE);

	// Create entry
	if (!(entry = create_file_entry(lister->cur_buffer,
									0,
									name,
									size,
									type,
									&date,
									comment_buf,
									protection,
									0,
									(*display_string) ? display_string : 0,
									(*description) ? description : 0,
									0)))
	{
		// Failed
		buffer_unlock(lister->cur_buffer);
		FreeVec(data);
		return RXERR_NO_MEMORY;
	}

	// Set user data?
	if (userdata)
	{
		struct TagItem *tags;

		// Create tags
		if ((tags = AllocMemH(lister->cur_buffer->memory, sizeof(struct TagItem) * 2)))
		{
			// Fill out tags
			tags[0].ti_Tag = DE_UserData;
			tags[0].ti_Data = userdata;
			tags[1].ti_Tag = TAG_END;

			// Chain tags
			ChainTagItems(&entry->de_Tags, tags);
		}
	}

	// Set additional flags
	entry->de_Flags |= flags;

	// Got version?
	if (version > -1)
	{
		// Set version info
		direntry_add_version(lister->cur_buffer, entry, version, revision, ver_days);
	}

	// User menu?
	if (*menu_stem && msg)
	{
		struct TagItem *tags;
		PopUpMenu *list;
		short count, num;
		long base = 0;
		char buffer[40];

		// Set entry flag
		entry->de_Flags |= ENTF_USER_MENU;

		// Get count
		rexx_get_var(msg, menu_stem, "COUNT", buffer, 40);
		count = atoi(buffer);

		// Get base ID
		if (rexx_get_var(msg, menu_stem, "BASE", buffer, 40))
			base = atoi(buffer);

		// Create tags and list
		if (count > 0 && (tags = AllocMemH(lister->cur_buffer->memory, sizeof(struct TagItem) * 2)) &&
			(list = AllocMemH(lister->cur_buffer->memory, sizeof(PopUpMenu))))
		{
			// Fill out menu
			list->userdata = base;

			// Fill out tags
			tags[0].ti_Tag = DE_PopupMenu;
			tags[0].ti_Data = (IPTR)list;
			tags[1].ti_Tag = TAG_END;

			// Chain tags
			ChainTagItems(&entry->de_Tags, tags);

			// Initialise list
			NewList((struct List *)list);

			// Go through items
			for (num = 0; num < count; num++)
			{
				char buf[6];
				struct Node *node;

				// Build number
				lsprintf(buf, "%ld", num);

				// Get menu name
				if (!(rexx_get_var(msg, menu_stem, buf, buffer, 40)))
					break;

				// Allocate node
				if ((node = AllocMemH(lister->cur_buffer->memory, sizeof(struct Node) + strlen(buffer) + 1)))
				{
					// Fill out node
					node->ln_Name = (char *)(node + 1);
					strcpy(node->ln_Name, buffer);

					// Add to list
					AddTail((struct List *)list, node);
				}
			}
		}
	}

	// Selected?
	if (selected)
		entry->de_Flags |= ENTF_SELECTED;

	// Add to list
	add_file_entry(lister->cur_buffer, entry, 0);

	// Set valid flag
	lister->cur_buffer->flags |= DWF_VALID;

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);

	// Free data
	FreeVec(data);

	// Was entry rejected?
	if (entry->de_Flags & ENTF_REJECTED)
		return RXERR_FILE_REJECTED;

	// Success
	return 0;
}

// Find a file from name/number
DirEntry *rexx_lister_get_file(DirBuffer *buffer, char **args)
{
	char *name;
	DirEntry *entry = 0;

	// Allocate buffer
	if (!(name = AllocVec(256, 0)))
		return 0;

	// Get name
	if (!(rexx_parse_word(args, name, 256)))
	{
		FreeVec(name);
		return 0;
	}

	// Name is a number?
	if (name[0] == '#')
	{
		long num;

		// Get number
		num = atoi(name + 1);
		entry = get_entry_ord(&buffer->entry_list, num);
	}

	// No entry yet?
	if (!entry)
	{
		// Look for entry by name
		if (!(entry = find_entry(&buffer->entry_list, name, 0, buffer->more_flags & DWF_CASE)))
			entry = find_entry(&buffer->reject_list, name, 0, buffer->more_flags & DWF_CASE);
	}

	// Free buffer
	FreeVec(name);

	return entry;
}

// Remove a file from a lister
short rexx_lister_remove_file(Lister *lister, char *args)
{
	DirEntry *entry;

	// Lock buffer
	buffer_lock(lister->cur_buffer, TRUE);

	// Get entry
	rexx_skip_space(&args);
	if (!(entry = rexx_lister_get_file(lister->cur_buffer, &args)))
	{
		buffer_unlock(lister->cur_buffer);
		return RXERR_INVALID_NAME;
	}

	// Remove entry
	removefile(lister->cur_buffer, entry);

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);
	return 0;
}

// Select a file in a lister
short rexx_lister_select_file(Lister *lister, char *args, char *result)
{
	DirEntry *entry;
	short state;
	unsigned short oldflags;

	// Lock buffer
	buffer_lock(lister->cur_buffer, TRUE);

	// Get entry
	rexx_skip_space(&args);
	if (!(entry = rexx_lister_get_file(lister->cur_buffer, &args)))
	{
		buffer_unlock(lister->cur_buffer);
		return RXERR_INVALID_NAME;
	}

	// Get old selection status
	if (entry->de_Flags & ENTF_SELECTED)
		strcpy(result, "on");
	else
		strcpy(result, "off");

	// Match on/off keyword
	rexx_skip_space(&args);
	state = rexx_match_keyword(&args, on_off_strings, 0);
	oldflags = entry->de_Flags;

	// Toggle?
	if (state == -1)
		entry->de_Flags ^= ENTF_SELECTED;

	// On?
	else if (state & 1)
		entry->de_Flags |= ENTF_SELECTED;

	// Off
	else
		entry->de_Flags &= ~ENTF_SELECTED;

	// Has state changed?
	if (oldflags != entry->de_Flags)
	{
		// Update selection info
		select_update_info(entry, lister->cur_buffer);
	}

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);
	return 0;
}

// Reload a file in a lister
short rexx_lister_reload_file(Lister *lister, char *args)
{
	char name[40];
	BPTR lock, cd;
	D_S(struct FileInfoBlock, fib)
	DirEntry *entry, *old;
	DirBuffer *buf = lister->cur_buffer;
	BOOL update = 0;

	// Get name
	rexx_parse_word(&args, name, 40);

	// Update?
	if (rexx_match_keyword(&args, update_key, 0) == 0)
		update = 1;

	// Lock lister path
	if (!(lock = Lock(buf->buf_Path, ACCESS_READ)))
		return RXERR_INVALID_PATH;

	// Change to this directory
	cd = CurrentDir(lock);

	// Lock file
	if (!(lock = Lock(name, ACCESS_READ)))
	{
		// Failed
		UnLock(CurrentDir(cd));
		return RXERR_INVALID_FILE;
	}

	// Examine the file
#ifdef USE_64BIT
	ExamineLock64(lock, (FileInfoBlock64 *)fib);
#else
	Examine(lock, fib);
#endif
	UnLock(lock);

	// Restore CD
	UnLock(CurrentDir(cd));

	// Lock current buffer
	buffer_lock(buf, TRUE);

	// Find entry
	if (!(old = find_entry(&buf->entry_list, fib->fib_FileName, 0, buf->more_flags & DWF_CASE)))
		old = find_entry(&buf->reject_list, fib->fib_FileName, 0, buf->more_flags & DWF_CASE);

	// Create entry
	if (!(entry = create_file_entry_fib(buf, 0, fib, 0, 0, 0, 0)))
	{
		// Failed
		buffer_unlock(buf);
		return RXERR_NO_MEMORY;
	}

	// Got an old entry?
	if (old)
	{
		struct TagItem *tags, *tag;
		VersionInfo *version;

		// Find old user data
		if ((tags = FindTagItem(DE_UserData, old->de_Tags)))
		{
			ULONG user;

			// Save value
			user = tags->ti_Data;

			// Create tags copy
			if ((tags = AllocMemH(buf->memory, sizeof(struct TagItem) * 2)))
			{
				// Fill out tags
				tags[0].ti_Tag = DE_UserData;
				tags[0].ti_Data = user;
				tags[1].ti_Tag = TAG_END;

				// Chain tags
				ChainTagItems(&entry->de_Tags, tags);
			}
		}

		// Copy version info
		if ((version = (VersionInfo *)GetTagData(DE_VersionInfo, 0, old->de_Tags)))
			direntry_add_version(buf, entry, version->vi_Version, version->vi_Revision, version->vi_Days);

		// Old menu?
		if (old->de_Flags & ENTF_USER_MENU && (tag = FindTagItem(DE_PopupMenu, old->de_Tags)))
		{
			PopUpMenu *menu, *oldmenu = (PopUpMenu *)tag->ti_Data;
			struct Node *node;

			// Create tags and menu
			if ((tags = AllocMemH(buf->memory, sizeof(struct TagItem) * 2)) &&
				(menu = AllocMemH(buf->memory, sizeof(PopUpMenu))))
			{
				// Fill out menu
				menu->userdata = oldmenu->userdata;

				// Fill out tags
				tags[0].ti_Tag = DE_PopupMenu;
				tags[0].ti_Data = (IPTR)menu;
				tags[1].ti_Tag = TAG_END;

				// Chain tags
				ChainTagItems(&entry->de_Tags, tags);

				// Initialise list
				NewList((struct List *)menu);

				// Go through items
				for (node = ((struct List *)oldmenu)->lh_Head; node->ln_Succ; node = node->ln_Succ)
				{
					struct Node *nodecopy;

					// Allocate node copy
					if ((nodecopy = AllocMemH(buf->memory, sizeof(struct Node) + strlen(node->ln_Name) + 1)))
					{
						// Fill out node
						nodecopy->ln_Name = (char *)(nodecopy + 1);
						strcpy(nodecopy->ln_Name, node->ln_Name);

						// Add to list
						AddTail((struct List *)menu, node);
					}
				}
			}
		}

		// Selected?
		if (old->de_Flags & ENTF_SELECTED)
			entry->de_Flags |= ENTF_SELECTED;

		// Remove and free old entry
		removefile(buf, old);
	}

	// Add to list
	add_file_entry(buf, entry, 0);

	// Set valid flag
	buf->flags |= DWF_VALID;

	// Unlock buffer
	buffer_unlock(buf);

	// Refresh datestamp?
	if (update)
	{
		// Send refresh command
		IPC_Command(lister->ipc, LISTER_REFRESH_WINDOW, REFRESHF_DATESTAMP, 0, 0, 0);
	}

	// Was entry rejected?
	if (entry->de_Flags & ENTF_REJECTED)
		return RXERR_FILE_REJECTED;

	// Success
	return 0;
}
