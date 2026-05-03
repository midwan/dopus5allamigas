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

// Update the lister sliders
// Called from the LISTER PROCESS
void lister_update_slider(Lister *lister, int which)
{
	DirBuffer *buffer;

	// Iconified or icon view?
	if (!lister_valid_window(lister) || lister->flags & LISTERF_VIEW_ICONS || lister->more_flags & LISTERF_TITLEBARRED)
		return;

	// Get buffer
	buffer = lister->cur_buffer;

	// Vertical slider
	if (which & SLIDER_VERT)
	{
		// Check offset
		if (buffer->buf_VertOffset + lister->text_height > buffer->buf_TotalEntries[0])
			buffer->buf_VertOffset = buffer->buf_TotalEntries[0] - lister->text_height;
		if (buffer->buf_VertOffset < 0)
			buffer->buf_VertOffset = 0;

		// Refresh slider
		SetGadgetAttrs(lister->backdrop_info->vert_scroller,
					   lister->window,
					   0,
					   PGA_Top,
					   buffer->buf_VertOffset,
					   (which & UPDATE_POS_ONLY) ? TAG_IGNORE : PGA_Total,
					   buffer->buf_TotalEntries[0],
					   (which & UPDATE_POS_ONLY) ? TAG_IGNORE : PGA_Visible,
					   lister->text_height,
					   TAG_END);
	}

	// Horizontal slider
	if (which & SLIDER_HORZ)
	{
		// Round offset for non-proportional fonts
		if (!(lister->more_flags & LISTERF_PROP_FONT))
			buffer->buf_HorizOffset =
				(buffer->buf_HorizOffset / lister->window->RPort->TxWidth) * lister->window->RPort->TxWidth;

		// Check offset
		if (buffer->buf_HorizOffset >= (buffer->buf_HorizLength - lister->text_width))
			buffer->buf_HorizOffset = buffer->buf_HorizLength - lister->text_width;
		if (buffer->buf_HorizOffset < 0)
			buffer->buf_HorizOffset = 0;

		// Update slider settings
		SetGadgetAttrs(lister->backdrop_info->horiz_scroller,
					   lister->window,
					   0,
					   PGA_Top,
					   lister->cur_buffer->buf_HorizOffset,
					   (which & UPDATE_POS_ONLY) ? TAG_IGNORE : PGA_Total,
					   lister->cur_buffer->buf_HorizLength,
					   (which & UPDATE_POS_ONLY) ? TAG_IGNORE : PGA_Visible,
					   lister->text_width,
					   TAG_END);
	}
}

// Redraw a window to reflect vertical slider's position
// Called from the LISTER PROCESS
int lister_pos_slider(Lister *lister, short which)
{
	unsigned short pos;

	// Iconified or icon view?
	if (!lister_valid_window(lister) || lister->flags & LISTERF_VIEW_ICONS || lister->more_flags & LISTERF_TITLEBARRED)
		return 0;

	// Vertical?
	if (which & SLIDER_VERT)
	{
		// Get slider position
		pos = scroller_top(((struct PropInfo *)lister->backdrop_info->vert_scroller->SpecialInfo)->VertPot,
						   lister->cur_buffer->buf_TotalEntries[0],
						   lister->text_height);

		// If offset hasn't changed, return
		if (lister->cur_buffer->buf_VertOffset == pos)
			return 0;

		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Erase old selector
			show_list_selector(lister, lister->selector_pos, 1);
		}

		// Set new position
		lister->cur_buffer->buf_VertOffset = pos;

		// Refresh window
		lister_display_dir(lister);

		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Show new selector
			show_list_selector(lister, lister->selector_pos, 0);
		}
	}

	// Horizontal
	else if (which & SLIDER_HORZ)
	{
		// Get slider position
		pos = scroller_top(((struct PropInfo *)lister->backdrop_info->horiz_scroller->SpecialInfo)->HorizPot,
						   lister->cur_buffer->buf_HorizLength,
						   lister->text_width);

		// Round to charachter width if non-proportional
		if (!(lister->more_flags & LISTERF_PROP_FONT))
			pos = (pos / lister->text_area.rast.TxWidth) * lister->text_area.rast.TxWidth;

		// If position hasn't changed, return
		if (lister->cur_buffer->buf_HorizOffset == pos)
			return 0;

		// Set new position
		lister->cur_buffer->buf_HorizOffset = pos;
		lister->cur_buffer->buf_OldVertOffset = -1;

		// Refresh window
		lister_display_dir(lister);
	}

	return 1;
}

unsigned short scroller_top(unsigned short pot, unsigned short total, unsigned short visible)
{
	unsigned short hidden;

	// Get number of hidden lines
	if (total > visible)
		hidden = total - visible;
	else
		hidden = 0;

	// Return top value
	return (unsigned short)((((unsigned long)hidden * pot) + (MAXPOT >> 1)) >> 16);
}

// Scroll a lister
// Called from the LISTER PROCESS
void lister_scroll(Lister *lister, int x_dir, int y_dir)
{
	// Iconified or icon view?
	if (!lister_valid_window(lister) || lister->flags & LISTERF_VIEW_ICONS)
		return;

	// Scroll in y axis?
	if (y_dir)
	{
		// Check it's ok to scroll
		if ((y_dir < 0 && lister->cur_buffer->buf_VertOffset > 0) ||
			(y_dir > 0 &&
			 lister->cur_buffer->buf_VertOffset + lister->text_height < lister->cur_buffer->buf_TotalEntries[0]))
		{
			// In key selection mode?
			if (lister->flags & LISTERF_KEY_SELECTION)
			{
				// Erase old selector
				show_list_selector(lister, lister->selector_pos, 1);
			}

			// Add line delta
			lister->cur_buffer->buf_VertOffset += y_dir;

			// Refresh vertical slider
			lister_update_slider(lister, SLIDER_VERT | UPDATE_POS_ONLY);
			lister_display_dir(lister);
		}

		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Show new selector
			show_list_selector(lister, lister->selector_pos, 0);
		}
	}

	// Scroll in x axis
	else if (x_dir)
	{
		// Multiply scroll amount by text width
		x_dir *= lister->text_area.rast.TxWidth;

		// Scroll left?
		if (x_dir < 0)
		{
			// Check if we're full left
			if (lister->cur_buffer->buf_HorizOffset == 0)
				return;
		}

		// Scroll right
		else
		{
			// Check if we're full right
			if (lister->cur_buffer->buf_HorizOffset >= lister->cur_buffer->buf_HorizLength - lister->text_width)
				return;
		}

		// Add scroll delta
		lister->cur_buffer->buf_HorizOffset += x_dir;
		lister->cur_buffer->buf_OldVertOffset = -1;

		// Refresh horizontal slider
		lister_update_slider(lister, SLIDER_HORZ | UPDATE_POS_ONLY);

		// Display window
		lister_display_dir(lister);
	}
}

// Scroll to show the first selected entry
// Called from the LISTER PROCESS
void lister_show_selected(Lister *lister, int type)
{
	DirEntry *entry;
	int count;

	// Invalid window or not full?
	if (!lister || lister->cur_buffer->buf_TotalEntries[0] <= lister->text_height)
		return;

	// Lock buffer
	buffer_lock(lister->cur_buffer, FALSE);

	// Go through entries
	for (entry = (DirEntry *)lister->cur_buffer->entry_list.mlh_Head, count = 0; entry->de_Node.dn_Succ;
		 entry = (DirEntry *)entry->de_Node.dn_Succ, count++)
	{
		// If entry is selected
		if (entry->de_Flags & ENTF_SELECTED)
		{
			// Check entry is the right type
			if (type == -2 || ENTRYTYPE(entry->de_Node.dn_Type) == type)
				break;
		}
	}

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);

	// Did we get an entry?
	if (entry->de_Node.dn_Succ)
	{
		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Erase old selector
			show_list_selector(lister, lister->selector_pos, 1);
		}

		// Set window offset
		lister->cur_buffer->buf_VertOffset = count;

		// Fix vertical slider
		lister_update_slider(lister, SLIDER_VERT);

		// Refresh window
		lister_display_dir(lister);

		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Show new selector
			show_list_selector(lister, lister->selector_pos, 0);
		}
	}

	return;
}

// Scroll to the first entry beginning with a string
// Called from the LISTER PROCESS
void lister_show_char(Lister *lister, char *str)
{
	DirEntry *entry = 0;
	long count = 0;
	short len, type = ENTRY_FILE;

	// If invalid or empty window, return
	if (!lister || lister->cur_buffer->buf_TotalEntries[0] <= lister->text_height)
		return;

	// Invalid string?
	if (!*str)
		return;

	// Get type
	if (str[0] >= 'A' && str[0] <= 'Z')
		type = ENTRY_DIRECTORY;

	// Lock buffer
	buffer_lock(lister->cur_buffer, FALSE);

	// Go through length of string
	for (len = strlen(str); len >= 0; len--)
	{
		// Go through entries
		for (entry = (DirEntry *)lister->cur_buffer->entry_list.mlh_Head, count = 0; entry->de_Node.dn_Succ;
			 entry = (DirEntry *)entry->de_Node.dn_Succ, count++)
		{
			BOOL ok = 0;

			// End of the road?
			if (len == 0)
			{
				// Test last letter
				if (tolower(entry->de_Node.dn_Name[0]) >= tolower(str[0]))
					ok = 1;
			}

			// See if this entry starts with our string
			else if (strnicmp(entry->de_Node.dn_Name, str, len) == 0)
				ok = 1;

			// Matched?
			if (ok)
			{
				// Correct type?
				if ((entry->de_Node.dn_Type >= 0 && type == ENTRY_DIRECTORY) ||
					(entry->de_Node.dn_Type < 0 && type == ENTRY_FILE))
				{
					len = -1;
					break;
				}
			}
		}

		// No match on last loop?
		if (len == 0 && !entry->de_Node.dn_Succ)
		{
			// Jump to end of the list
			entry = (DirEntry *)lister->cur_buffer->entry_list.mlh_TailPred;
		}
	}

	// Unlock buffer
	buffer_unlock(lister->cur_buffer);

	// If we found one, set new offset
	if (entry && entry->de_Node.dn_Succ)
	{
		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Erase old selector
			show_list_selector(lister, lister->selector_pos, 1);
		}

		// Store new position
		lister->cur_buffer->buf_VertOffset = count;

		// Fix slider and refresh window
		lister_update_slider(lister, SLIDER_VERT);
		lister_display_dir(lister);

		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Show new selector
			show_list_selector(lister, lister->selector_pos, 0);
		}
	}
}

// Select an entry and make it visible
void lister_sel_show(Lister *lister, char *name)
{
	DirEntry *entry;
	DirBuffer *buffer;

	// Get buffer
	buffer = lister->cur_buffer;

	// Lock buffer
	buffer_lock(buffer, FALSE);

	// Find entry
	if ((entry = find_entry(&buffer->entry_list, name, 0, buffer->more_flags & DWF_CASE)))
	{
		// Not already selected?
		if (!(entry->de_Flags & ENTF_SELECTED))
		{
			// Select it and update selection info
			entry->de_Flags |= ENTF_SELECTED;
			select_update_info(entry, buffer);
		}

		// Make entry visible
		lister_show_entry(lister, entry);
	}

	// Unlock buffer
	buffer_unlock(buffer);
}

// Scroll a window to make a given entry visible
// Called from the LISTER PROCESS
void lister_show_entry(Lister *lister, DirEntry *entry)
{
	DirBuffer *buffer;
	DirEntry *temp;
	long count;

	// If invalid window return
	if (!lister)
		return;

	// Get buffer
	buffer = lister->cur_buffer;

	// Lock buffer
	buffer_lock(buffer, FALSE);

	// Find entry's offset in list
	for (temp = (DirEntry *)buffer->entry_list.mlh_Head, count = 0; temp->de_Node.dn_Succ && temp != entry;
		 temp = (DirEntry *)temp->de_Node.dn_Succ, count++)
		;

	// Unlock buffer
	buffer_unlock(buffer);

	// If we didn't find entry, fail
	if (!temp)
		return;

	// Set new offset
	buffer->buf_VertOffset = count;

	// If this file was already visible, don't scroll
	if (buffer->buf_OldVertOffset > -1 && buffer->buf_VertOffset >= buffer->buf_OldVertOffset &&
		buffer->buf_VertOffset < buffer->buf_OldVertOffset + lister->text_height)
	{
		// Restore old offset and return
		buffer->buf_VertOffset = buffer->buf_OldVertOffset;
		return;
	}

	// If we've moved
	if (buffer->buf_VertOffset != buffer->buf_OldVertOffset)
	{
		// Fix slider
		lister_update_slider(lister, SLIDER_VERT);

		// Refresh window
		lister_display_dir(lister);
	}
}

// Get the display width of a window
BOOL lister_fix_horiz_len(Lister *lister)
{
	short width = 0, old_len;
	DirBuffer *buffer;

	// Get buffer
	buffer = lister->cur_buffer;

	// Lock buffer
	buffer_lock(buffer, FALSE);

	// Store old length
	old_len = buffer->buf_HorizLength;

	// Clear 'recalc' flag
	lister->more_flags &= ~LISTERF_NEED_RECALC;

	// Is window empty?
	if (IsListEmpty((struct List *)&buffer->entry_list))
		;

	// Device list?
	else if (((DirEntry *)buffer->entry_list.mlh_Head)->de_Node.dn_Type == ENTRY_DEVICE)
	{
		DirEntry *entry;
		short len;
		char *ptr;

		// Initialise name width
		buffer->buf_FieldWidth[DISPLAY_NAME] = 0;

		// Go through entries
		for (entry = (DirEntry *)buffer->entry_list.mlh_Head; entry->de_Node.dn_Succ;
			 entry = (DirEntry *)entry->de_Node.dn_Succ)
		{
			// Check longest display string length
			if ((ptr = (char *)GetTagData(DE_DisplayString, 0, entry->de_Tags)))
				lister_check_max_length_tabs(lister, ptr, &len, DISPLAY_NAME);
		}

		// Get size from name field
		width = buffer->buf_FieldWidth[DISPLAY_NAME];
	}

	// Normal entries
	else
	{
		// Is window valid?
		if (buffer->flags & DWF_VALID)
		{
			long item;
			DirEntry *entry;
			short a;

			// Reset sizes
			buffer->name_length = 0;
			buffer->comment_length = 0;
			buffer->version_length = 0;
			buffer->type_length = 0;
			buffer->owner_length = 0;
			buffer->group_length = 0;
			buffer->size_length = 0;
			buffer->date_length = 0;

			// Reset field widths
			for (a = 0; a < DISPLAY_LAST; a++)
				if (!(buffer->buf_CustomWidthFlags & (1 << a)))
					buffer->buf_FieldWidth[a] = 0;

			// Check network field sizes
			lister_check_max_length(lister, string_no_owner, &buffer->owner_length, DISPLAY_OWNER);
			lister_check_max_length(lister, string_no_group, &buffer->group_length, DISPLAY_GROUP);

			// Go through entries
			for (entry = (DirEntry *)buffer->entry_list.mlh_Head; entry->de_Node.dn_Succ;
				 entry = (DirEntry *)entry->de_Node.dn_Succ)
			{
				NetworkInfo *network = 0;
				VersionInfo *info;
				char *ptr;
				short length;

				// Get NetworkInfo
				if (entry->de_Flags & ENTF_NETWORK)
					network = (NetworkInfo *)GetTagData(DE_NetworkInfo, 0, entry->de_Tags);

				// Check name size
				lister_check_max_length(lister, entry->de_Node.dn_Name, &buffer->name_length, DISPLAY_NAME);

				// Check protection size
				lister_check_max_length(lister, entry->de_ProtBuf, &length, DISPLAY_PROTECT);

				// Check comment size
				if ((ptr = (char *)GetTagData(DE_Comment, 0, entry->de_Tags)))
					lister_check_max_length(lister, ptr, &buffer->comment_length, DISPLAY_COMMENT);

				// Check date size
				length = buffer->date_length;
				lister_check_max_length(lister, entry->de_DateBuf, &length, DISPLAY_DATE);
				buffer->date_length = length;

				// Check filetype size
				if ((ptr = (char *)GetTagData(DE_Filetype, 0, entry->de_Tags)))
					lister_check_max_length(lister, ptr, &buffer->type_length, DISPLAY_FILETYPE);

				// Network?
				if (network)
				{
					// Check network field sizes
					lister_check_max_length(lister, network->owner, &buffer->owner_length, DISPLAY_OWNER);
					lister_check_max_length(lister, network->group, &buffer->group_length, DISPLAY_GROUP);
					lister_check_max_length(lister, network->net_protbuf, &length, DISPLAY_NETPROT);
				}

				// Got version info?
				if (entry->de_Flags & ENTF_VERSION &&
					(info = (VersionInfo *)GetTagData(DE_VersionInfo, 0, entry->de_Tags)))
				{
					char buf[40];

					// Build version string
					build_version_string(buf, info->vi_Version, info->vi_Revision, info->vi_Days, -1);

					// Check length
					lister_check_max_length(lister, buf, &buffer->version_length, DISPLAY_VERSION);
				}

				// Valid size?
				if (entry->de_Node.dn_Type <= ENTRY_FILE ||
					(entry->de_Node.dn_Type >= ENTRY_DIRECTORY && !(entry->de_Flags & ENTF_NO_SIZE)))
				{
					char buf[27];

					// Convert to string
					if (entry->de_Node.dn_Type <= ENTRY_FILE && entry->de_Size == 0)
						strcpy(buf, string_empty);
					else
#ifdef USE_64BIT
						ItoaU64(&entry->de_Size,
								buf,
								sizeof(buf),
								(environment->env->settings.date_flags & DATE_1000SEP) ? GUI->decimal_sep : 0);
#else
						Itoa(entry->de_Size,
							 buf,
							 (environment->env->settings.date_flags & DATE_1000SEP) ? GUI->decimal_sep : 0);
#endif

					// Check length
					lister_check_max_length(lister, buf, &buffer->size_length, DISPLAY_SIZE);
				}
			}

			// Go through display items
			for (item = 0; item < DISPLAY_LAST && buffer->buf_ListFormat.display_pos[item] >= 0 &&
						   buffer->buf_ListFormat.display_pos[item] < DISPLAY_LAST;
				 item++)
			{
				// Add width of this field to total
				width += buffer->buf_FieldWidth[buffer->buf_ListFormat.display_pos[item]];
			}
		}
	}

	// Check that horizontal offset isn't too far for width
	if (buffer->buf_HorizOffset >= (width - lister->text_width))
		buffer->buf_HorizOffset = width - lister->text_width;
	if (buffer->buf_HorizOffset < 0)
		buffer->buf_HorizOffset = 0;

	// Store width
	buffer->buf_HorizLength = width;

	// Unlock buffer
	buffer_unlock(buffer);

	// Return to indicate change
	return (BOOL)(old_len != width);
}

// See if a length is the longest for a field
void lister_check_max_length(Lister *lister, char *string, short *storage, short field)
{
	short len;

	// Get length
	len = (string) ? strlen(string) : 0;

	// Longest string?
	if (storage && len > *storage)
		*storage = len;

	// If a custom width is set, don't change the width
	if (!(lister->cur_buffer->buf_CustomWidthFlags & (1 << field)))
	{
		// Proportional font in lister?
		if (lister->more_flags & LISTERF_PROP_FONT)
		{
			// Get length in pixels
			len = (string) ? TextLength(&lister->text_area.rast, string, len) : 0;
		}

		// Otherwise, calculate pixel width from font size
		else
			len *= lister->text_area.rast.TxWidth;

		// Add gap?
		if (len > 0)
			len += lister->text_area.rast.TxWidth;

		// Longest pixel length?
		if (lister->cur_buffer->buf_FieldWidth[field] < len)
			lister->cur_buffer->buf_FieldWidth[field] = len;
	}
}

// See if a length is the longest for a field (supports tabs)
void lister_check_max_length_tabs(Lister *lister, char *string, short *storage, short field)
{
	short len;

	// Get length
	len = strlen(string);

	// Longest string?
	if (storage && len > *storage)
		*storage = len;

	// If a custom width is set, don't change the width
	if (!(lister->cur_buffer->buf_CustomWidthFlags & (1 << field)))
	{
		// Get length in pixels
		if ((len = TabbedTextExtent(&lister->text_area.rast, string, len, LISTER_TABSIZE, 0)) > 0)
			len += lister->text_area.rast.TxWidth;

		// Longest pixel length?
		if (lister->cur_buffer->buf_FieldWidth[field] < len)
			lister->cur_buffer->buf_FieldWidth[field] = len;
	}
}

// Get string length for a lister
short lister_get_length(Lister *lister, char *string)
{
	short len;

	// Proportional font in lister?
	if (lister->more_flags & LISTERF_PROP_FONT)
	{
		// Get length in pixels
		len = TextLength(&lister->text_area.rast, string, strlen(string));
	}

	// Otherwise, calculate pixel width from font size
	else
		len = strlen(string) * lister->text_area.rast.TxWidth;

	return len;
}
