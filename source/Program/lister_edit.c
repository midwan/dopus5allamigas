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
#include <libraries/multiuser.h>
#include <proto/multiuser.h>

#define UNDO (GUI->global_undo_buffer + 256)

// End a direct edit
void lister_end_edit(Lister *lister, short flags)
{
	struct DevProc *proc;
	DirBuffer *buffer = lister->cur_buffer;

	// No window?
	if (!lister_valid_window(lister))
		return;

	// Cancelled?
	if (lister->edit_flags & EDITF_CANCEL)
	{
		// Clear flag
		flags &= ~EDITF_KEEP;
		lister->edit_flags &= ~EDITF_CANCEL;
	}

	// Reset offsets
	lister->edit_old_offset = -1;
	lister->edit_new_offset = -1;

	// Get device proc
	proc = GetDeviceProc("", 0);

	// Was editing?
	if (lister->cursor_line > -1)
	{
		long line;
		BOOL change = 0;
		char *command = 0;

		// Has entry not changed?
		if (strcmp(lister->edit_ptr, lister->edit_old_ptr) == 0)
			flags &= ~EDITF_KEEP;

		// If keeping changes, remove entry from list
		if (flags & EDITF_KEEP)
		{
			// If we want the offset, get current position
			if (flags & EDITF_OFFSET)
			{
				lister->edit_old_offset = find_entry_offset((struct MinList *)&buffer->entry_list, lister->edit_entry);
			}

			// Remove the entry
			remove_file_entry(buffer, lister->edit_entry);
		}

		// Look at the type
		switch (lister->edit_type)
		{
		// Name
		case DISPLAY_NAME:

			// Don't try to rename if name is empty
			if (*lister->edit_ptr)
			{
				// Keeping?
				if (flags & EDITF_KEEP)
				{
					// Custom handler?
					if (buffer->buf_CustomHandler[0])
					{
						// Get command
						command = "name";
						strcpy(UNDO, lister->edit_ptr);
					}

					// Otherwise
					else
					{
						// Try to do Rename
						if ((change = Rename(lister->edit_buffer, lister->edit_ptr)))
							break;

						// Failed
						DisplayBeep(lister->window->WScreen);

						// Show error
						lister_edit_error(lister, MSG_RENAME);
					}
				}
			}

			// Restore old string if not keeping
			strcpy(lister->edit_ptr, lister->edit_buffer);
			break;

		// Owner/group
		case DISPLAY_OWNER:
		case DISPLAY_GROUP:

			// Ok to use?
			if (flags & EDITF_KEEP)
			{
				NetworkInfo *network;

				// Custom handler?
				if (buffer->buf_CustomHandler[0])
				{
					// Get command
					command = (lister->edit_type == DISPLAY_OWNER) ? "owner" : "group";
					strcpy(UNDO, lister->edit_ptr);
				}

				// Get network info
				else if ((network = (NetworkInfo *)GetTagData(DE_NetworkInfo, 0, lister->edit_entry->de_Tags)))
				{
					ULONG owner;
					UWORD val = 0;
					char *ptr = lister->edit_ptr;
					BOOL ok = 1;

					// Get current owner/group
					owner = (network->owner_id << 16) | network->group_id;

					// Empty string
					if (!*lister->edit_ptr)
						val = 0;

					// Digit?
					else if (isdigit(*lister->edit_ptr))
					{
						long v;

						// Get value
						v = atoi(lister->edit_ptr);

						// Check for invalid
						if (v < 0 || v > 65535)
							ok = 0;

						// Get valid value
						else
						{
							// Print whole value into string
							lsprintf(lister->edit_ptr, "%ld", v);

							// Get value
							if ((val = (UWORD)v) == 0)
							{
								// No string
								*lister->edit_ptr = 0;
							}

							// User?
							else if (lister->edit_type == DISPLAY_OWNER)
							{
								// Try for packet
								if (buffer->user_info && (DoPkt(proc->dvp_Port,
																ACTION_UID_TO_USERINFO,
																val,
																(ULONG)buffer->user_info,
																0,
																0,
																0)))
								{
									// Copy user name to edit buffer
									strcpy(lister->edit_ptr, buffer->user_info->ui_UserName);
								}

								// MUFS volume
								else if (muBase && buffer->mu_user_info &&
										 environment->env->settings.general_flags & GENERALF_ENABLE_MUFS)
								{
									// Fill out MUFS stuff
									buffer->mu_user_info->uid = val;

									// Get user name?
									if (muGetUserInfo(buffer->mu_user_info, muKeyType_uid))
									{
										// Copy user name to edit buffer
										strcpy(lister->edit_ptr, buffer->mu_user_info->UserID);
									}
								}
							}

							// Group?
							else if (lister->edit_type == DISPLAY_GROUP)
							{
								// Try for packet
								if (buffer->group_info && (DoPkt(proc->dvp_Port,
																 ACTION_GID_TO_GROUPINFO,
																 val,
																 (ULONG)buffer->group_info,
																 0,
																 0,
																 0)))
								{
									// Copy group name to edit buffer
									strcpy(lister->edit_ptr, buffer->group_info->gi_GroupName);
								}

								// MUFS volume
								else if (muBase && buffer->mu_group_info &&
										 environment->env->settings.general_flags & GENERALF_ENABLE_MUFS)
								{
									// Fill out MUFS stuff
									buffer->mu_group_info->gid = val;

									// Get group name?
									if (muGetGroupInfo(buffer->mu_group_info, muKeyType_gid))
									{
										// Copy group name to edit buffer
										strcpy(lister->edit_ptr, buffer->mu_group_info->GroupID);
									}
								}
							}
						}
					}

					// Setting by name
					else
					{
						long res;

						// Owner?
						if (lister->edit_type == DISPLAY_OWNER)
						{
							// Try packet first
							res = DoPkt(proc->dvp_Port, ACTION_NAME_TO_UID, (ULONG)lister->edit_ptr, 0, 0, 0, 0);
							if (IoErr() == 0)
							{
								// Got name?
								if (res > 0)
								{
									// Get user ID
									val = res;
								}

								// Invalid user
								else
									ok = 0;
							}

							// Packet failed, try MUFS
							else if (muBase && buffer->mu_user_info &&
									 environment->env->settings.general_flags & GENERALF_ENABLE_MUFS)
							{
								// Fill things in
								strcpy(buffer->mu_user_info->UserID, lister->edit_ptr);

								// Get user by name?
								if (muGetUserInfo(buffer->mu_user_info, muKeyType_UserID))
								{
									// Get user ID
									val = buffer->mu_user_info->uid;
									ptr = buffer->mu_user_info->UserID;
								}

								// Invalid user
								else
									ok = 0;
							}
						}

						// Group?
						else if (lister->edit_type == DISPLAY_GROUP)
						{
							// Try packet first
							res = DoPkt(proc->dvp_Port, ACTION_NAME_TO_GID, (ULONG)lister->edit_ptr, 0, 0, 0, 0);
							if (IoErr() == 0)
							{
								// Got group?
								if (res > 0)
								{
									// Get group ID
									val = res;
								}

								// Invalid group
								else
									ok = 0;
							}

							// Packet failed, try MUFS
							else if (muBase && buffer->mu_group_info &&
									 environment->env->settings.general_flags & GENERALF_ENABLE_MUFS)
							{
								// Fill things in
								strcpy(buffer->mu_group_info->GroupID, lister->edit_ptr);

								// Get user by name?
								if (muGetGroupInfo(buffer->mu_group_info, muKeyType_GroupID))
								{
									// Get user ID
									val = buffer->mu_group_info->gid;
									ptr = buffer->mu_group_info->GroupID;
								}

								// Invalid group
								else
									ok = 0;
							}
						}
					}

					// Ok to continue?
					if (ok)
					{
						// No owner/group?
						if (!val)
							ptr =
								GetString(&locale, (lister->edit_type == DISPLAY_OWNER) ? MSG_NO_OWNER : MSG_NO_GROUP);

						// Get new owner/group value
						if (lister->edit_type == DISPLAY_OWNER)
						{
							owner &= 0x0000ffff;
							owner |= val << 16;
						}
						else
						{
							owner &= 0xffff0000;
							owner |= val;
						}

						// Try to set new owner
						if ((change = SetOwner(lister->edit_entry->de_Node.dn_Name, owner)))
						{
							// Copy new string to buffer for installation
							if (val && ptr != lister->edit_ptr)
								strcpy(lister->edit_ptr, ptr);
							strcpy(lister->edit_old_ptr, ptr);

							// Store new value
							if (lister->edit_type == DISPLAY_OWNER)
								network->owner_id = val;
							else
								network->group_id = val;

							// Success
							break;
						}
					}

					// Beep
					DisplayBeep(lister->window->WScreen);

					// Show error
					lister_edit_error(
						lister, (lister->edit_type == DISPLAY_OWNER) ? MSG_LISTER_TITLE_OWNER : MSG_LISTER_TITLE_GROUP);
				}
			}

			// Restore old string if not keeping
			strcpy(lister->edit_ptr, lister->edit_buffer);
			break;

		// Protection
		case DISPLAY_PROTECT:
		case DISPLAY_NETPROT:

			// Keeping?
			if (flags & EDITF_KEEP)
			{
				// Custom handler?
				if (buffer->buf_CustomHandler[0])
				{
					// Get command
					command = (lister->edit_type == DISPLAY_NETPROT) ? "netprot" : "protect";
					strcpy(UNDO, lister->edit_ptr);
				}

				// Otherwise
				else
				{
					long value, old;

					// Network protection?
					if (lister->edit_type == DISPLAY_NETPROT)
					{
						// Get protection value
						value = netprot_from_string(lister->edit_ptr);

						// Get old value, clear changeable bits
						old = lister->edit_entry->de_Protection;
						old &= ~(FIBF_GRP_READ | FIBF_GRP_WRITE | FIBF_GRP_EXECUTE | FIBF_GRP_DELETE | FIBF_OTR_READ |
								 FIBF_OTR_WRITE | FIBF_OTR_EXECUTE | FIBF_OTR_DELETE);
					}

					// Normal protection
					else
					{
						// Get protection value
						value = prot_from_string(lister->edit_ptr);

						// Get old value, clear changeable bits
						old = lister->edit_entry->de_Protection;
						old &= ~((1 << 7) | FIBF_SCRIPT | FIBF_PURE | FIBF_ARCHIVE | FIBF_READ | FIBF_WRITE |
								 FIBF_EXECUTE | FIBF_DELETE);
					}

					// Or into new value
					value |= old;

					// Set new protection
					if ((change = SetProtection(lister->edit_entry->de_Node.dn_Name, value)))
						break;

					// Failed
					DisplayBeep(lister->window->WScreen);

					// Show error
					lister_edit_error(lister, MSG_PROTECT_TITLE);
				}
			}

			// Restore old string if not keeping
			strcpy(lister->edit_ptr, lister->edit_buffer);
			break;

		// Date
		case DISPLAY_DATE:

			// Keeping?
			if (flags & EDITF_KEEP)
			{
				// Custom handler?
				if (buffer->buf_CustomHandler[0])
				{
					// Get command
					command = "date";
					strcpy(UNDO, lister->edit_ptr);
				}

				// Otherwise
				else
				{
					char date_b[30], time_b[30];
					struct DateStamp date;

					// Convert to separate strings
					ParseDateStrings(lister->edit_ptr, date_b, time_b, 0);

					// Convert to datestamp
					if (DateFromStringsNew(date_b, time_b, &date, environment->env->settings.date_format))
					{
						// Set new datestamp
						if ((change = SetFileDate(lister->edit_entry->de_Node.dn_Name, &date)))
						{
							// Get new date string
							date_build_string(&date, lister->edit_ptr, 1);
							break;
						}
					}

					// Failed
					DisplayBeep(lister->window->WScreen);

					// Show error
					lister_edit_error(lister, MSG_DATESTAMP_TITLE);
				}
			}

			// Restore old string if not keeping
			strcpy(lister->edit_ptr, lister->edit_buffer);
			break;

		// Comment
		case DISPLAY_COMMENT:

			// Ok to use?
			if (flags & EDITF_KEEP)
			{
				// Custom handler?
				if (buffer->buf_CustomHandler[0])
				{
					// Get command
					command = "comment";
					strcpy(UNDO, lister->edit_ptr);
				}

				// Otherwise
				else
				{
					// Try to set new comment
					if ((change = SetComment(lister->edit_entry->de_Node.dn_Name, lister->edit_ptr)))
					{
						// Copy new comment to buffer for installation
						strcpy(lister->edit_old_ptr, lister->edit_ptr);
					}

					// Failed
					else
					{
						// Beep
						DisplayBeep(lister->window->WScreen);

						// Show error
						lister_edit_error(lister, MSG_COMMENT_TITLE);
					}
				}
			}

			// Install new or old string
			direntry_add_string(buffer, lister->edit_entry, DE_Comment, lister->edit_old_ptr);
			break;
		}

		// Made changes?
		if (flags & EDITF_KEEP)
		{
			// Add entry back
			add_file_entry(buffer, lister->edit_entry, 0);

			// Get new line
			if ((line = find_entry_offset(&buffer->entry_list, lister->edit_entry)) > -1)
			{
				// Want to keep offset?
				if (flags & EDITF_OFFSET)
				{
					// Store offset
					lister->edit_new_offset = line;
				}

				// Convert to display offset
				line -= buffer->buf_VertOffset;

				// If a different line we'll have to redisplay completely
				if (line != lister->cursor_line)
					line = -1;
			}

			// Not found at all
			else
				line = -2;

			// Got a command to send to a custom handler
			if (command)
			{
				// Send the message
				rexx_handler_msg(0,
								 buffer,
								 RXMF_WARN,
								 HA_String,
								 0,
								 "edit",
								 HA_Value,
								 1,
								 lister,
								 HA_String,
								 2,
								 lister->edit_entry->de_Node.dn_Name,
								 HA_String,
								 3,
								 command,
								 HA_String,
								 4,
								 UNDO,
								 HA_Value,
								 5,
								 GetTagData(DE_UserData, 0, lister->edit_entry->de_Tags),
								 TAG_END);

				// Clear flag so sliders will be refreshed
				flags &= ~EDITF_KEEP;
			}

			// Update buffer?
			else if (change)
				update_buffer_stamp(lister);
		}

		// Cache line
		else
			line = lister->cursor_line;

		// End edit
		lister->cursor_line = -1;
		lister->edit_type = -1;

		// Need to recalculate lister width?
		if (lister->edit_flags & EDITF_LONGEST && !(flags & EDITF_KEEP))
			line = -2;

		// Allowed to refresh?
		if (!(flags & EDITF_NOREFRESH))
		{
			// Need to redisplay?
			if (line < 0)
			{
				// Do refresh
				lister_refresh_display(lister, (line == -2) ? REFRESHF_SLIDERS : 0);
			}

			// Otherwise
			else
			{
				// Redisplay edit line
				display_entry(lister->edit_entry, lister, line);

				// Refresh title
				if (lister->more_flags & LISTERF_TITLE && !(flags & EDITF_NO_TITLE))
					lister_show_title(lister, 1);
			}
		}
	}

	// Free device process
	FreeDeviceProc(proc);

	// Clear RMBTRAP flag
	lister->window->Flags &= ~WFLG_RMBTRAP;
}

// Set edit position
BOOL lister_start_edit(Lister *lister, short x, short y, short item)
{
	short line, pos = 0;
	BOOL ok = 0, supplied = 0;
	short cursor_off = 0;

	// No window?
	if (!lister_valid_window(lister))
		return 0;

	// Clear refresh flags
	lister->edit_flags &= ~(EDITF_REFRESH | EDITF_RECALC | EDITF_LONGEST);

	// Custom handler?
	if (lister->cur_buffer->buf_CustomHandler[0])
	{
		// Can only edit if flag is set
		if (!(lister->cur_buffer->cust_flags & CUSTF_EDITING))
			return 0;
	}

	// If item is supplied, y will be the line, and x will be the cursor position
	if (item > -1 || item == -2)
	{
		// Get line
		line = y;

		// Is this at the end of the current string?
		if (x < 0 || (lister->edit_type > -1 && x == lister->edit_pos && x == lister->edit_length && x > 0))
		{
			// Special flag to move to end of new string
			cursor_off = -1;
		}

		// Get cursor offset
		else
			cursor_off = x;

		// Set supplied flag
		supplied = 1;
	}

	// Get line clicked on
	else
		line = (y - lister->text_area.rect.MinY) / lister->text_area.font->tf_YSize;

	// Different line?
	if (line != lister->cursor_line)
	{
		DirEntry *entry;

		// End old edit
		lister_end_edit(lister, EDITF_KEEP | EDITF_OFFSET | EDITF_NO_TITLE);

		// Has position of entry changed?
		if (lister->edit_old_offset != lister->edit_new_offset)
		{
			// Convert offsets to window relative
			lister->edit_old_offset -= lister->cur_buffer->buf_VertOffset;
			lister->edit_new_offset -= lister->cur_buffer->buf_VertOffset;

			// If old offset was below the new line, and new offset is above it
			if (lister->edit_old_offset > line && lister->edit_new_offset <= line)
			{
				// Shift new edit line down by one
				++line;
			}

			// Or, if old offset was above the new line, and new offset is below
			else if (lister->edit_old_offset < line && lister->edit_new_offset >= line)
			{
				// Shift new edit line up by one
				--line;
			}
		}

		// Find entry
		if (!(entry = get_entry_ord(&lister->cur_buffer->entry_list, line + lister->cur_buffer->buf_VertOffset)))
			return 0;

		// Can't edit devices
		if (entry->de_Node.dn_Type == ENTRY_DEVICE)
			return 0;

		// Start editing this
		lister->edit_entry = entry;
	}

	// Item not supplied?
	if (item < 0)
	{
		// Build title array without showing it
		lister_show_title(lister, 0);

		// Want name?
		if (item == -2)
		{
			// Find which item is name
			for (item = 0; item < 16; item++)
				if (lister->title_boxes[item].tb_Item == DISPLAY_NAME)
					break;
		}

		// Get from coordinates
		else
		{
			// Get position clicked on
			pos = lister->cur_buffer->buf_HorizOffset + x - lister->text_area.rect.MinX;

			// Find which item
			for (item = 0; item < 16; item++)
			{
				// Last item?
				if (lister->title_boxes[item].tb_Item == -1)
				{
					if (ok)
					{
						item = 16;
						break;
					}
					continue;
				}

				// Skip an item?
				if (lister->title_boxes[item].tb_Left == -2)
					continue;

				// Set flag
				ok = 1;

				// Within item?
				if (pos >= lister->title_boxes[item].tb_Position &&
					pos < lister->title_boxes[item].tb_Position + lister->title_boxes[item].tb_Width)
					break;
			}
		}
	}

	// Didn't get one?
	if (item > 15 || lister->title_boxes[item].tb_Item < 0)
		return 0;

	// Can only edit certain types of item
	if (lister->title_boxes[item].tb_Item != DISPLAY_NAME && lister->title_boxes[item].tb_Item != DISPLAY_PROTECT &&
		lister->title_boxes[item].tb_Item != DISPLAY_NETPROT && lister->title_boxes[item].tb_Item != DISPLAY_OWNER &&
		lister->title_boxes[item].tb_Item != DISPLAY_GROUP && lister->title_boxes[item].tb_Item != DISPLAY_DATE &&
		lister->title_boxes[item].tb_Item != DISPLAY_COMMENT)
		return 0;

	// Already editing, different thing?
	if (lister->edit_type > -1 && lister->edit_type != lister->title_boxes[item].tb_Item)
	{
		// End edit
		lister_end_edit(lister, EDITF_OFFSET | EDITF_KEEP | EDITF_NO_TITLE); /**************/

		// Has position of entry changed?
		if (lister->edit_old_offset != lister->edit_new_offset)
		{
			// Get new edit line
			line += lister->edit_new_offset - lister->edit_old_offset;
		}
	}

	// Or, editing same thing
	else if (lister->edit_type > -1)
	{
		short delta;

		// Get change in cursor position
		delta = pos - lister->cursor_pos;

		// Adjust cursor position
		lister->cursor_pos += delta;
		lister->edit_flags |= EDITF_REFRESH;

		// Calculate edit position
		lister_edit_calc_position(lister, 1);

		// Check cursor position
		lister_edit_check_cursor(lister, 1);

		// Update display ourselves
		display_entry(lister->edit_entry, lister, lister->cursor_line);
		return 1;
	}

	// Need to scroll?
	if (line < 0 || line >= lister->text_height)
	{
		// Scroll down?
		if (line > 0)
		{
			// Fix offset
			lister->cur_buffer->buf_VertOffset += line - lister->text_height + 1;
			line = lister->text_height - 1;
		}

		// Scroll up?
		else
		{
			// Fix offset
			lister->cur_buffer->buf_VertOffset += line;
			line = 0;
		}

		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Erase old selector
			show_list_selector(lister, lister->selector_pos, 1);
		}

		// Refresh scroller and window
		lister_update_slider(lister, SLIDER_VERT);
		lister_display_dir(lister);

		// In key selection mode?
		if (lister->flags & LISTERF_KEY_SELECTION)
		{
			// Show new selector
			show_list_selector(lister, lister->selector_pos, 0);
		}
	}

	// Save line
	lister->cursor_line = line;

	// Set edit type and offset
	lister->edit_type = lister->title_boxes[item].tb_Item;
	lister->edit_field_pos = lister->title_boxes[item].tb_Position;

	// Item supplied?
	if (supplied)
	{
		short off;

		// Get offset
		if ((off = cursor_off) < 0)
			off = 0;

		// Start at supplied position
		lister->edit_pos = off;
		//		lister_edit_calc_position(lister,0);
	}

	// Got coordinates supplied
	else
	{
		// Get cursor position
		lister->cursor_pos = pos;
	}

	// Initialise settings for this type
	if (!lister_edit_init(lister, !supplied))
	{
		// Failed
		lister->cursor_line = -1;
		lister->edit_type = -1;
		return 0;
	}

	// Item supplied?
	if (supplied)
	{
		// Set cursor to the end of the string?
		if (cursor_off == -1)
		{
			// Start at end of item
			lister->edit_pos = lister->edit_length;
		}

		// Calculate cursor position
		lister_edit_calc_position(lister, 0);

		// Check item position
		lister_edit_check_item(lister);
	}

	// Otherwise, just check cursor position
	else
		lister_edit_check_cursor(lister, 1);

	// If entry is selected, deselect it
	if (lister->edit_entry->de_Flags & ENTF_SELECTED)
	{
		// Deselect it
		select_default_state(lister, lister->cursor_line, 0);

		// Refresh title bar
		select_show_info(lister, 0);
	}

	// Show cursor in entry
	else
		display_entry(lister->edit_entry, lister, line);

	// Set RMBTRAP flag
	lister->window->Flags |= WFLG_RMBTRAP;
	return 1;
}

// Handle edit event
BOOL lister_edit_key(Lister *lister, struct IntuiMessage *msg)
{
	// Not editing?
	if (lister->cursor_line == -1)
		return 0;

	// Clear refresh flags
	lister->edit_flags &= ~(EDITF_REFRESH | EDITF_RECALC);

	// Mouse buttons?
	if (msg->Class == IDCMP_MOUSEBUTTONS)
	{
		// If a click outside of lister area, change to right button
		if (!(point_in_element(&lister->list_area, msg->MouseX, msg->MouseY)))
			msg->Code = MENUDOWN;

		// Button down?
		if (msg->Code == SELECTDOWN || msg->Code == MIDDLEDOWN)
		{
			// Left button click with middle button down?
			if (msg->Code == SELECTDOWN && msg->Qualifier & IEQUALIFIER_MIDBUTTON)
			{
				// End the edit
				lister->edit_flags |= EDITF_CANCEL;
				lister_end_edit(lister, 0);
				return 0;
			}

			// Start new edit
			lister_start_edit(lister, msg->MouseX, msg->MouseY, -1);
			return 1;
		}

		// Right button ends
		else if (msg->Code == MENUDOWN)
		{
			// End edit
			lister_end_edit(lister, EDITF_KEEP);
			return 1;
		}
	}

	// Key press?
	else if (msg->Class == IDCMP_RAWKEY)
	{
		short old_pos;
		unsigned char key = 0;

		// Save cursor position
		old_pos = lister->edit_pos;

		// Look at key
		switch (msg->Code)
		{
		// Cursor left
		case CURSORLEFT:

			// With shift?
			if (msg->Qualifier & IEQUAL_ANYSHIFT)
				lister->edit_pos = 0;

			// Otherwise, one space left
			else if (lister->edit_pos > 0)
				--lister->edit_pos;
			break;

		// Cursor right
		case CURSORRIGHT:

			// With shift?
			if (msg->Qualifier & IEQUAL_ANYSHIFT)
				lister->edit_pos = lister->edit_length;

			// Otherwise, one space right
			else if (lister->edit_pos < lister->edit_length)
				++lister->edit_pos;
			break;

		// Cursor up
		case CURSORUP: {
			short line, item;

			// Get current line
			line = lister->cur_buffer->buf_VertOffset + lister->cursor_line;

			// With shift?
			if (msg->Qualifier & IEQUAL_ANYSHIFT)
			{
				// Move to top of page?
				if (lister->cursor_line > 0)
					line -= lister->cursor_line;

				// Up a page
				else
					line -= lister->text_height - 1;
			}

			// With control?
			else if (msg->Qualifier & IEQUALIFIER_CONTROL)
				line = 0;

			// Otherwise, one line up
			else
				--line;

			// Bounds-check
			if (line < 0)
				line = 0;

			// Not changed?
			if (line == lister->cur_buffer->buf_VertOffset + lister->cursor_line)
				break;

			// Get edit line
			line -= lister->cur_buffer->buf_VertOffset;

			// Get current item
			item = lister_edit_next(lister, -1);

			// Start edit on new line
			lister_start_edit(lister, lister->edit_pos, line, item);

			// Get new cursor position
			old_pos = lister->edit_pos;
		}
		break;

		// Cursor down
		case CURSORDOWN: {
			short line, item;

			// Get current line
			line = lister->cur_buffer->buf_VertOffset + lister->cursor_line;

			// With shift?
			if (msg->Qualifier & IEQUAL_ANYSHIFT)
			{
				// Move to bottom of page?
				if (lister->cursor_line < lister->text_height - 1)
					line += lister->text_height - 1 - lister->cursor_line;

				// Down a page
				else
					line += lister->text_height - 1;
			}

			// With control?
			else if (msg->Qualifier & IEQUALIFIER_CONTROL)
				line = lister->cur_buffer->buf_TotalEntries[0];

			// Otherwise, one line down
			else
				++line;

			// Bounds-check
			if (line >= lister->cur_buffer->buf_TotalEntries[0])
				line = lister->cur_buffer->buf_TotalEntries[0] - 1;
			if (line < 0)
				line = 0;

			// Not changed?
			if (line == lister->cur_buffer->buf_VertOffset + lister->cursor_line)
				break;

			// Get edit line
			line -= lister->cur_buffer->buf_VertOffset;

			// Get current item
			item = lister_edit_next(lister, -1);

			// Start edit on new line
			lister_start_edit(lister, lister->edit_pos, line, item);

			// Get new cursor position
			old_pos = lister->edit_pos;
		}
		break;

		// Tab
		case 0x42: {
			short item;

			// Get new item
			if ((item = lister_edit_next(lister, (msg->Qualifier & IEQUAL_ANYSHIFT) ? 1 : 0)) == -1)
				break;

			// Start edit on new item
			lister_start_edit(lister, 0, lister->cursor_line, item);

			// Get new cursor position
			old_pos = lister->edit_pos;
		}
		break;

		// Something else
		default:

			// Ignore key up
			if (msg->Code & IECODE_UP_PREFIX)
				break;

			// Convert from rawkey
			ConvertRawKey(msg->Code, msg->Qualifier, &key);

			// Command key down?
			if (msg->Qualifier & IEQUALIFIER_RCOMMAND)
			{
				// Map a few things
				key = tolower(key);

				// Map command keys to control keys
				if (key == 'x')
					key = 24;
				else if (key == 'c')
					key = 3;
				else if (key == 'v')
					key = 22;
				else if (key == 'q' || key == 'u')
					key = 17;

				// Otherwise, invalid
				else
					break;
			}

			// Invalid left command
			else if (msg->Qualifier & IEQUALIFIER_LCOMMAND)
				break;

			// Valid key?
			if (key && (lister_edit_key_press(lister, key, msg->Qualifier)))
			{
				// Return pressed
				lister_end_edit(lister, EDITF_KEEP);
				return 1;
			}
			break;
		}

		// Cursor moved?
		if (lister->edit_pos != old_pos)
		{
			// Calculate real cursor position
			lister_edit_calc_position(lister, 0);
			lister->edit_flags |= EDITF_REFRESH;
		}

		// Recalculate?
		if (lister->edit_flags & EDITF_RECALC)
		{
			// Refresh
			lister_refresh_display(lister, REFRESHF_SLIDERS);

			// Check cursor position
			lister_edit_check_cursor(lister, 1);
		}

		// Refresh?
		else if (lister->edit_flags & EDITF_REFRESH)
		{
			// Check cursor position
			lister_edit_check_cursor(lister, 1);

			// Update display ourselves
			display_entry(lister->edit_entry, lister, lister->cursor_line);
		}
	}

	// Swallow event
	return 1;
}

// Initialise for editing a certain type
BOOL lister_edit_init(Lister *lister, BOOL need_pos)
{
	char *ptr;
	struct Node *after = 0;
	BOOL add = 1, change = 0;
	short offset;

	// Initialise buffer
	lister->edit_buffer[0] = 0;
	lister->edit_length = 0;
	lister->edit_old_width = 256;
	lister->edit_ptr = 0;

	// Backup offset
	offset = lister->cur_buffer->buf_HorizOffset;

	// Only item in list?
	if (Att_NodeCount((Att_List *)&lister->cur_buffer->entry_list) == 1)
		add = 0;

	// Otherwise, remove from list
	else
	{
		// Get previous node
		after = (struct Node *)lister->edit_entry->de_Node.dn_Pred;
		if (!after->ln_Pred)
			after = 0;

		// Remove entry temporarily
		Remove((struct Node *)lister->edit_entry);

		// Calculate horizontal length for window
		change = lister_fix_horiz_len(lister);
	}

	// Look at type
	switch (lister->edit_type)
	{
	// Name
	case DISPLAY_NAME:

		// Length of current name
		lister->edit_length = strlen(lister->edit_entry->de_Node.dn_Name);

		// Maximum length
		lister->edit_max_length = lister->cur_buffer->name_field_size - 1;
		lister->edit_old_width = lister->cur_buffer->name_length;

		// Set up edit buffer
		lister->edit_ptr = lister->edit_entry->de_Node.dn_Name;
		strcpy(lister->edit_buffer, lister->edit_entry->de_Node.dn_Name);
		lister->edit_old_ptr = lister->edit_buffer;
		break;

	// Protection
	case DISPLAY_PROTECT:

		// Fixed size
		lister->edit_length = 8;
		lister->edit_max_length = 8;
		lister->edit_ptr = lister->edit_entry->de_ProtBuf;
		strcpy(lister->edit_buffer, lister->edit_entry->de_ProtBuf);
		lister->edit_old_ptr = lister->edit_buffer;
		break;

	// Network protection, owner, group
	case DISPLAY_NETPROT:
	case DISPLAY_OWNER:
	case DISPLAY_GROUP:

		if (lister->edit_entry->de_Flags & ENTF_NETWORK)
		{
			NetworkInfo *network;

			// Get network info
			if ((network = (NetworkInfo *)GetTagData(DE_NetworkInfo, 0, lister->edit_entry->de_Tags)))
			{
				// Netprot?
				if (lister->edit_type == DISPLAY_NETPROT)
				{
					// Fixed size
					lister->edit_length = 8;
					lister->edit_max_length = 8;

					// Get edit thing
					lister->edit_ptr = network->net_protbuf;
					strcpy(lister->edit_buffer, network->net_protbuf);
					lister->edit_old_ptr = lister->edit_buffer;
				}

				// Owner
				else if (lister->edit_type == DISPLAY_OWNER)
				{
					// Get size
					lister->edit_length = strlen(network->owner);
					lister->edit_max_length = sizeof(network->owner) - 1;
					lister->edit_old_width = lister->cur_buffer->owner_length;

					// Get edit thing
					lister->edit_ptr = network->owner;
					strcpy(lister->edit_buffer, network->owner);
					lister->edit_old_ptr = lister->edit_buffer;
				}

				// Group
				else if (lister->edit_type == DISPLAY_GROUP)
				{
					// Get size
					lister->edit_length = strlen(network->group);
					lister->edit_max_length = sizeof(network->group) - 1;
					lister->edit_old_width = lister->cur_buffer->group_length;

					// Get edit thing
					lister->edit_ptr = network->group;
					strcpy(lister->edit_buffer, network->group);
					lister->edit_old_ptr = lister->edit_buffer;
				}
			}
		}
		break;

	// Date
	case DISPLAY_DATE:

		// Length of current date
		lister->edit_length = strlen(lister->edit_entry->de_DateBuf);

		// Maximum length
		lister->edit_max_length = lister->cur_buffer->date_length;
		lister->edit_old_width = lister->cur_buffer->date_length;

		// Set up edit buffer
		lister->edit_ptr = lister->edit_entry->de_DateBuf;
		strcpy(lister->edit_buffer, lister->edit_entry->de_DateBuf);
		lister->edit_old_ptr = lister->edit_buffer;
		break;

	// Comment
	case DISPLAY_COMMENT:

		// Valid comment?
		if ((ptr = (char *)GetTagData(DE_Comment, 0, lister->edit_entry->de_Tags)))
		{
			// Get length
			lister->edit_length = strlen(ptr);
			strcpy(lister->edit_buffer, ptr);
		}

		// Install edit comment
		if (!(direntry_add_string(lister->cur_buffer, lister->edit_entry, DE_Comment, str_space_string)))
		{
			// Failed, oops
			break;
		}

		// Get new comment pointer
		ptr = (char *)GetTagData(DE_Comment, 0, lister->edit_entry->de_Tags);

		// Copy comment into it and set edit pointer
		strcpy(ptr, lister->edit_buffer);
		lister->edit_ptr = ptr;

		// Save old comment
		strcpy(lister->edit_buffer + 128, lister->edit_buffer);
		lister->edit_old_ptr = lister->edit_buffer + 128;

		// Fixed size
		lister->edit_max_length = lister->cur_buffer->comment_field_size - 1;
		lister->edit_old_width = lister->cur_buffer->comment_length;
		break;
	}

	// Need to add back?
	if (add)
	{
		// Add entry back to list
		Insert((struct List *)&lister->cur_buffer->entry_list, (struct Node *)lister->edit_entry, after);

		// Calculate horizontal length for window
		if (change)
			lister_fix_horiz_len(lister);
	}

	// Restore offset
	lister->cur_buffer->buf_HorizOffset = offset;

	// Copy string to undo buffer
	if (lister->edit_ptr)
		strcpy(UNDO, lister->edit_ptr);

	// Need to get position from cursor?
	if (need_pos)
		lister_edit_calc_position(lister, 1);

	// Check position is not outside of maximum size
	if (lister->edit_pos > lister->edit_length)
	{
		// Adjust cursor position
		lister->edit_pos = lister->edit_length;
		lister_edit_calc_position(lister, 0);
	}

	// Success?
	return (BOOL)((lister->edit_ptr) ? TRUE : FALSE);
}

// Get the next item
short lister_edit_next(Lister *lister, short want_prev)
{
	short item, prev = -1, next = 0, first = -1, last = -1;
	BOOL ok = 0, last_flag = 0;

	// Go through titles
	for (item = 0; item < 16; item++)
	{
		BOOL valid = 0;

		// Last item?
		if (lister->title_boxes[item].tb_Item == -1)
		{
			if (ok)
			{
				item = 16;
				break;
			}
			continue;
		}

		// Skip an item?
		if (lister->title_boxes[item].tb_Left == -2)
			continue;

		// Set flag
		ok = 1;

		// Is this item valid?
		if (lister->title_boxes[item].tb_Item == DISPLAY_NAME || lister->title_boxes[item].tb_Item == DISPLAY_PROTECT ||
			lister->title_boxes[item].tb_Item == DISPLAY_NETPROT ||
			lister->title_boxes[item].tb_Item == DISPLAY_OWNER || lister->title_boxes[item].tb_Item == DISPLAY_GROUP ||
			lister->title_boxes[item].tb_Item == DISPLAY_DATE || lister->title_boxes[item].tb_Item == DISPLAY_COMMENT)
		{
			// Remember if the first
			if (first == -1)
				first = item;

			// Set valid flag
			valid = 1;
		}

		// Looking for the last?
		if (last_flag)
		{
			// Save as last if valid
			if (valid)
				last = item;
		}

		// Need next?
		else if (next)
		{
			// Was item valid?
			if (valid)
			{
				// Use it and break
				prev = item;
				break;
			}
		}

		// Is this the one we're editing now?
		else if (lister->title_boxes[item].tb_Item == lister->edit_type)
		{
			// Want item number?
			if (want_prev == -1)
				return item;

			// Want next?
			if (want_prev == 0)
				next = 1;

			// At the first one? Need the last
			else if (first == item)
				last_flag = 1;

			// Use previous
			else
				break;
		}

		// Save item if valid
		if (valid)
			prev = item;
	}

	// Didn't find it?
	if (item > 15 || lister->title_boxes[prev].tb_Item < 0)
	{
		// Needed last?
		if (last_flag)
			prev = last;

		// Or first
		else if (!want_prev)
			prev = first;

		// Failed
		else
			prev = -1;
	}

	// The same?
	if (prev > 0 && lister->title_boxes[prev].tb_Item == lister->edit_type)
		prev = -1;
	return prev;
}

// Make sure the current edit item and cursor are visible
BOOL lister_edit_check_item(Lister *lister)
{
	short item, start, end;
	DirBuffer *buffer;

	// Find the item
	for (item = 0; item < 16; item++)
		if (lister->title_boxes[item].tb_Item == lister->edit_type)
			break;

	// Not found?
	if (item > 15)
		return 0;

	// Get the start and end position of the current edit item
	start = lister->title_boxes[item].tb_Position;
	end = start + lister->title_boxes[item].tb_Width - 1;

	// Pointer to buffer
	buffer = lister->cur_buffer;

	// Is item wholly within the display?
	if (start >= buffer->buf_HorizOffset && end < buffer->buf_HorizOffset + lister->text_width)
		return 1;

	// Is start of item to the left?
	if (start < buffer->buf_HorizOffset)
	{
		// Shift offset to the left
		buffer->buf_HorizOffset = start;
	}

	// Is end of item to the right?
	else if (end >= buffer->buf_HorizOffset + lister->text_width)
	{
		// Can item fit in the display?
		if (lister->title_boxes[item].tb_Width <= lister->text_width)
		{
			// Shift to end of item
			buffer->buf_HorizOffset = end - lister->text_width;
		}

		// If not, shift to beginning of item
		else
		{
			buffer->buf_HorizOffset = start;
		}
	}

	// Make sure cursor is visible
	lister_edit_check_cursor(lister, 0);

	// Update the sliders
	lister_update_slider(lister, SLIDER_HORZ);

	// Refresh window
	buffer->buf_OldVertOffset = -1;
	lister_display_dir(lister);
	return 0;
}

// Make sure the cursor is visible
BOOL lister_edit_check_cursor(Lister *lister, short show)
{
	DirBuffer *buffer;

	// Pointer to buffer
	buffer = lister->cur_buffer;

	// Is cursor within visible range?
	if (lister->cursor_pos >= buffer->buf_HorizOffset &&
		lister->cursor_pos < buffer->buf_HorizOffset + lister->text_width)
		return 1;

	// To the left of the display?
	if (lister->cursor_pos < buffer->buf_HorizOffset)
	{
		// Shift display left
		buffer->buf_HorizOffset = lister->cursor_pos;
	}

	// To the right of the display?
	else if (lister->cursor_pos >= buffer->buf_HorizOffset + lister->text_width)
	{
		// Shift display right
		buffer->buf_HorizOffset = lister->cursor_pos - lister->text_width + 1;
	}

	// Want to show changes?
	if (show)
	{
		// Update the sliders
		lister_update_slider(lister, SLIDER_HORZ);

		// Refresh window
		buffer->buf_OldVertOffset = -1;
		lister_display_dir(lister);
	}

	return 0;
}

// Key pressed in edit
BOOL lister_edit_key_press(Lister *lister, unsigned char key, UWORD qual)
{
	BOOL ret = 0;

	// Look at edit type
	switch (lister->edit_type)
	{
	// Name/Comment/Date
	case DISPLAY_NAME:
	case DISPLAY_COMMENT:
	case DISPLAY_OWNER:
	case DISPLAY_GROUP:
	case DISPLAY_DATE:

		// Normal string edit
		ret = lister_edit_key_string(lister, key, qual);
		break;

	// Protect
	case DISPLAY_PROTECT:

		// Special edit
		ret = lister_edit_key_protect(lister, key);
		break;

	// Net protection
	case DISPLAY_NETPROT:

		// Special edit
		ret = lister_edit_key_netprotect(lister, key);
		break;
	}

	// Dynamic length fields?
	if (lister->edit_type == DISPLAY_NAME || lister->edit_type == DISPLAY_COMMENT ||
		lister->edit_type == DISPLAY_OWNER || lister->edit_type == DISPLAY_GROUP)
	{
		BOOL longest = 0;

		// Was string the longest, but now isn't?
		if (lister->edit_length <= lister->edit_old_width && lister->edit_flags & EDITF_LONGEST)
		{
			longest = 1;
			lister->edit_flags &= ~EDITF_LONGEST;
		}

		// Is string the longest one?
		else
		{
			short length = 0;

			// Get length
			if (lister->edit_type == DISPLAY_NAME)
				length = lister->cur_buffer->name_length;
			else if (lister->edit_type == DISPLAY_COMMENT)
				length = lister->cur_buffer->comment_length;
			else if (lister->edit_type == DISPLAY_OWNER)
				length = lister->cur_buffer->owner_length;
			else if (lister->edit_type == DISPLAY_GROUP)
				length = lister->cur_buffer->group_length;

			// Longest?
			if (lister->edit_length > length ||
				(lister->edit_length < length && lister->edit_length > lister->edit_old_width))
			{
				longest = 1;
				lister->edit_flags |= EDITF_LONGEST;
			}
		}

		// Need to recalculate lister width?
		if (longest)
			lister->edit_flags |= EDITF_RECALC;
	}

	// A change to datestamp or protection fields needs a recalc if a proportional font is used
	else if (lister->more_flags & LISTERF_PROP_FONT)
		lister->edit_flags |= EDITF_RECALC;

	return ret;
}

// Normal key edit
BOOL lister_edit_key_string(Lister *lister, unsigned char key, UWORD qual)
{
	BOOL fail = 0;
	char *temp = GUI->global_undo_buffer + 512;

	// Illegal keys for name
	if (lister->edit_type == DISPLAY_NAME)
	{
		// Can't have : or /
		if (key == ':' || key == '/')
			fail = 1;
	}

	// Backspace?
	if (key == 8)
	{
		// At start of buffer?
		if (lister->edit_pos == 0)
			fail = 1;

		// Delete to beginning of string?
		else if (qual & IEQUAL_ANYSHIFT)
		{
			// Copy string
			strcpy(lister->edit_ptr, lister->edit_ptr + lister->edit_pos);
			lister->edit_length -= lister->edit_pos;
			lister->edit_pos = 0;
			return 0;
		}

		// Otherwise, delete backwards
		else
		{
			// Copy string
			strcpy(lister->edit_ptr + lister->edit_pos - 1, lister->edit_ptr + lister->edit_pos);

			// Decrease length and move cursor
			--lister->edit_length;
			--lister->edit_pos;
			return 0;
		}
	}

	// Delete?
	else if (key == 127)
	{
		// At end of buffer?
		if (lister->edit_pos == lister->edit_length)
			fail = 1;

		// Delete to end of string?
		else if (qual & IEQUAL_ANYSHIFT)
		{
			// Clear string
			lister->edit_ptr[lister->edit_pos] = 0;
			lister->edit_length = lister->edit_pos;
			lister->edit_flags |= EDITF_REFRESH;
			return 0;
		}

		// Otherwise, delete forwards
		else
		{
			// Copy string
			strcpy(lister->edit_ptr + lister->edit_pos, lister->edit_ptr + lister->edit_pos + 1);

			// Decrease length
			--lister->edit_length;

			// Set refresh flag
			lister->edit_flags |= EDITF_REFRESH;
			return 0;
		}
	}

	// Return?
	else if (key == 13)
		return 1;

	// Control-q does undo
	else if (key == 17 || key == 21)
	{
		// Copy from undo buffer
		strcpy(lister->edit_ptr, UNDO);

		// Fix position
		lister->edit_length = strlen(lister->edit_ptr);
		lister->edit_pos = lister->edit_length;

		// Set refresh flag
		lister->edit_flags |= EDITF_REFRESH;
		return 0;
	}

	// Control-x does clear
	else if (key == 24)
	{
		// Copy to undo buffer
		strcpy(UNDO, lister->edit_ptr);

		// Clear buffer
		*lister->edit_ptr = 0;
		lister->edit_length = 0;
		lister->edit_pos = 0;

		// Set refresh flag
		lister->edit_flags |= EDITF_REFRESH;
		return 0;
	}

	// Control-c does copy
	else if (key == 3)
	{
		struct ClipHandle *clip;

		// Open clipboard
		if ((clip = OpenClipBoard(0)))
		{
			// Write name to clipboard
			WriteClipString(clip, lister->edit_ptr, lister->edit_length);

			// Close clipboard
			CloseClipBoard(clip);
		}
		return 0;
	}

	// Control-v does paste
	else if (key == 22)
	{
		struct ClipHandle *clip;

		// Open clipboard
		if ((clip = OpenClipBoard(0)))
		{
			// Clear buffer
			GUI->global_undo_buffer[512] = 0;

			// Read string from clipboard
			ReadClipString(clip, GUI->global_undo_buffer + 512, 500);

			// Close clipboard
			CloseClipBoard(clip);

			// Got string?
			if (GUI->global_undo_buffer[512])
			{
				char *ptr;

				// Check string for illegal characters
				for (ptr = GUI->global_undo_buffer + 512; *ptr; ptr++)
					if (*ptr == '/' || *ptr == ':' || *ptr < 32)
						break;

				// String is ok?
				if (!*ptr)
				{
					// Copy string to undo buffer
					strcpy(UNDO, lister->edit_ptr);

					// Copy to string
					stccpy(lister->edit_ptr, GUI->global_undo_buffer + 512, lister->edit_max_length);

					// Fix length
					lister->edit_length = strlen(lister->edit_ptr);
					lister->edit_pos = lister->edit_length;

					// Set refresh flag
					lister->edit_flags |= EDITF_REFRESH;
					return 0;
				}
			}
		}

		// Failed
		fail = 1;
	}

	// Escape does cancel
	else if (key == 27)
	{
		// Set cancel flag
		lister->edit_flags |= EDITF_CANCEL;
		return 1;
	}

	// Invalid key?
	else if (key < 32)
		fail = 1;

	// See if buffer is full
	else if (lister->edit_length == lister->edit_max_length)
		fail = 1;

	// Failed?
	if (fail)
	{
		// Flash error
		DisplayBeep(lister->window->WScreen);
		return 0;
	}

	// Make space in buffer
	strcpy(temp, lister->edit_ptr + lister->edit_pos);
	lister->edit_ptr[lister->edit_pos++] = key;
	lister->edit_ptr[lister->edit_pos] = 0;
	strcat(lister->edit_ptr, temp);
	++lister->edit_length;
	return 0;
}

// Edit protection bits
BOOL lister_edit_key_protect(Lister *lister, unsigned char key)
{
	static char *protkeys = "hsparwed";
	short a;

	// Return?
	if (key == 13)
		return 1;

	// Escape?
	else if (key == 27)
	{
		// Set cancel flag
		lister->edit_flags |= EDITF_CANCEL;
		return 1;
	}

	// Delete set to 'rwed'
	else if (key == 8 || key == 127)
	{
		strcpy(lister->edit_ptr, "----rwed");
		lister->edit_flags |= EDITF_REFRESH;
		return 0;
	}

	// Space toggles
	else if (key == 32)
	{
		// Get the key
		key = protkeys[lister->edit_pos];
	}

	// Convert to lowercase
	if (key >= 'A' && key <= 'Z')
		key += 'a' - 'A';

	// Check for valid key
	for (a = 0; a < 8; a++)
		if (key == protkeys[a])
			break;

	// Invalid?
	if (a == 8)
	{
		// Flash error
		DisplayBeep(lister->window->WScreen);
		return 0;
	}

	// Key set in string?
	if (lister->edit_ptr[a] != '-')
	{
		// Turn off
		lister->edit_ptr[a] = '-';
	}

	// Not set
	else
	{
		// Turn on
		lister->edit_ptr[a] = protkeys[a];
	}

	// Set refresh flag
	lister->edit_flags |= EDITF_REFRESH;
	return 0;
}

// Edit protection bits
BOOL lister_edit_key_netprotect(Lister *lister, unsigned char key)
{
	static char *protkeys = "rwedRWED";
	short a;

	// Return?
	if (key == 13)
		return 1;

	// Escape?
	else if (key == 27)
	{
		// Set cancel flag
		lister->edit_flags |= EDITF_CANCEL;
		return 1;
	}

	// Delete set to 'rwed'
	else if (key == 8 || key == 127)
	{
		strcpy(lister->edit_ptr, "rwedrwed");
		lister->edit_flags |= EDITF_REFRESH;
		return 0;
	}

	// Space toggles
	else if (key == 32)
	{
		// Get the key
		key = protkeys[lister->edit_pos];
	}

	// Check for valid key
	for (a = 0; a < 8; a++)
		if (key == protkeys[a])
			break;

	// Invalid?
	if (a == 8)
	{
		// Flash error
		DisplayBeep(lister->window->WScreen);
		return 0;
	}

	// Key set in string?
	if (lister->edit_ptr[a] != '-')
	{
		// Turn off
		lister->edit_ptr[a] = '-';
	}

	// Not set
	else
	{
		// Turn on
		lister->edit_ptr[a] = tolower(protkeys[a]);
	}

	// Set refresh flag
	lister->edit_flags |= EDITF_REFRESH;
	return 0;
}

// Show error
void lister_edit_error(Lister *lister, short str)
{
	// Build header
	lsprintf(lister->work_buffer, "%s failed", GetString(&locale, str));

	// Get error
	Fault(IoErr(), lister->work_buffer, lister->work_buffer + 80, 256);

	// Show error
	title_error(lister->work_buffer + 80, 8);
}

// Calculate edit or cursor position
void lister_edit_calc_position(Lister *lister, short type)
{
	// Calculate edit position?
	if (type == 1)
	{
		short pos;
		struct TextExtent extent;

		// Get position within field
		pos = lister->cursor_pos - lister->edit_field_pos;

		// Find edit position
		if (!lister->edit_ptr)
		{
			lister->edit_pos = 0;
			extent.te_Width = 0;
		}
		else
			lister->edit_pos = TextFit(lister->window->RPort,
									   lister->edit_ptr,
									   strlen(lister->edit_ptr),
									   &extent,
									   NULL,
									   1,
									   pos,
									   lister->window->RPort->TxHeight);

		// Fix cursor position
		lister->cursor_pos = lister->edit_field_pos + extent.te_Width;
	}

	// Calculate cursor position
	else
	{
		// Get cursor position within field
		lister->cursor_pos = TextLength(lister->window->RPort, lister->edit_ptr, lister->edit_pos);
		lister->cursor_pos += lister->edit_field_pos;
	}
}
