/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas (dopus5allamigas fork)

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

// Do a lister-specific function
int lister_do_function(Lister *lister, ULONG func)
{
	int result = 1;
	BOOL show_status = 0;
	BOOL spec_ok = 0;

	// Not special buffer or icons, and state not locked, and not iconified?
	if (lister->cur_buffer != lister->special_buffer && !(lister->cur_buffer->more_flags & DWF_LOCK_STATE) &&
		!(lister->flags & LISTERF_ICONIFIED) &&
		(!(lister->flags & LISTERF_VIEW_ICONS) || lister->flags & LISTERF_ICON_ACTION))
		spec_ok = 1;

	// Look at function
	switch (func)
	{
	// Make source
	case MENU_LISTER_SOURCE:

		// If not special buffer or icons, make lister the source
		if (spec_ok)
		{
			lister->flags |= LISTERF_SOURCE;
			lister->flags &= ~(LISTERF_DEST | LISTERF_STORED_SOURCE | LISTERF_STORED_DEST);
			lister_check_source(lister);
			show_status = 1;
		}
		break;

	// Make destination
	case MENU_LISTER_DEST:

		// If not special buffer, make lister the destination
		if (spec_ok)
		{
			lister->flags |= LISTERF_DEST;
			lister->flags &= ~(LISTERF_SOURCE | LISTERF_STORED_SOURCE | LISTERF_STORED_DEST);
			lister_check_dest(lister);
			show_status = 1;
		}
		break;

	// Lock as source
	case MENU_LISTER_LOCK_SOURCE:

		// If not special buffer, lock lister as source
		if (spec_ok)
		{
			lister->flags |= LISTERF_SOURCE | LISTERF_SOURCEDEST_LOCK;
			lister->flags &= ~(LISTERF_DEST | LISTERF_STORED_SOURCE | LISTERF_STORED_DEST);
			show_status = 1;
		}
		break;

	// Lock as destination
	case MENU_LISTER_LOCK_DEST:

		// If not special buffer, lock lister as destination
		if (spec_ok)
		{
			lister->flags |= LISTERF_DEST | LISTERF_SOURCEDEST_LOCK;
			lister->flags &= ~(LISTERF_SOURCE | LISTERF_STORED_SOURCE | LISTERF_STORED_DEST);
			show_status = 1;
		}
		break;

	// Unlock
	case MENU_LISTER_UNLOCK:
		lister->flags &= ~LISTERF_SOURCEDEST_LOCK;
		if (lister->flags & LISTERF_SOURCE)
			lister_check_source(lister);
		else if (lister->flags & LISTERF_DEST)
			lister_check_dest(lister);
		show_status = 1;
		break;

	// Off
	case MENU_LISTER_OFF:

		// If not locked, turn off
		if (!(lister->cur_buffer->more_flags & DWF_LOCK_STATE))
		{
			lister->flags &= ~(LISTERF_SOURCE | LISTERF_DEST | LISTERF_STORED_SOURCE | LISTERF_STORED_DEST |
							   LISTERF_SOURCEDEST_LOCK);
			show_status = 1;
		}
		break;

	// Close
	case MENU_LISTER_CLOSE:
		IPC_Command(lister->ipc, IPC_QUIT, 0, 0, 0, 0);
		break;

	// Parent/Root
	case MENU_LISTER_PARENT:
	case MENU_LISTER_ROOT:
	case MENU_LISTER_PARENT_NEW:
	case MENU_LISTER_ROOT_NEW:

		// Check lister isn't locked
		if (!(lister->flags & LISTERF_LOCK))
		{
			char buffer[256];
			short ret = 0, a;
			BOOL shift = 0;

			// Custom handler?
			if (lister->cur_buffer->buf_CustomHandler[0])
			{
				UWORD qual = 0;

				// Build command
				if (func == MENU_LISTER_PARENT || func == MENU_LISTER_PARENT_NEW)
					strcpy(buffer, "parent");
				else
					strcpy(buffer, "root");

				// New window means shift is down (for this anyway)
				if (func == MENU_LISTER_PARENT_NEW || func == MENU_LISTER_ROOT_NEW)
					qual = IEQUALIFIER_LSHIFT;

				// Send message
				rexx_handler_msg(0,
								 lister->cur_buffer,
								 RXMF_WARN,
								 HA_String,
								 0,
								 buffer,
								 HA_Value,
								 1,
								 lister,
								 HA_String,
								 2,
								 lister->cur_buffer->buf_Path,
								 HA_Qualifier,
								 6,
								 qual,
								 TAG_END);
				break;
			}

			// Open in new window?
			if (func == MENU_LISTER_PARENT_NEW || func == MENU_LISTER_ROOT_NEW)
				shift = 1;

			// Copy current path
			strcpy(buffer, lister->cur_buffer->buf_Path);

			// Try twice
			for (a = 0; a < 2; a++)
			{
				// Do parent/root
				if (func == MENU_LISTER_PARENT || func == MENU_LISTER_PARENT_NEW)
					ret = path_parent(buffer);
				else
					ret = path_root(buffer);

				// Successful?
				if (ret)
					break;

				// For second time through, expand path
				if (a == 0)
				{
					BPTR lock;

					// Lock path
					if ((lock = Lock(buffer, ACCESS_READ)))
					{
						// Expand path
						DevNameFromLockDopus(lock, buffer, 256);
						UnLock(lock);
					}
				}
			}

			// Ok to change?
			if (ret)
			{
				// If shift is down, open new window
				if (shift)
				{
					// Read into new lister
					if (read_new_lister(buffer, lister, 0))
						break;
				}

				// Read into same window
				do_parent_root(lister, buffer);
			}
			else
			{
				function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_devicelist, lister);
			}
		}
		break;

	// Edit format
	case MENU_EDIT_LISTER:
		lister_configure(lister);
		break;

	// Open live filter requester
	case MENU_LISTER_FILTER:
		lister_add_filter(lister);
		break;

	// Leave objects out
	case MENU_ICON_LEAVE_OUT:

		// View as icons?
		if (lister->flags & LISTERF_VIEW_ICONS)
		{
			SetBusyPointer(lister->window);
			backdrop_leave_icons_out(lister->backdrop_info, 0, 0);
			ClearPointer(lister->window);
		}

		// Otherwise
		else
			function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_leaveout, lister);
		break;

	// Icon information
	case MENU_ICON_INFO:

		// Icon view?
		if (lister->flags & LISTERF_VIEW_ICONS)
			backdrop_info(lister->backdrop_info, 0, 0);

		// Normal files
		else
			function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_iconinfo, lister);
		break;

	// Icon information
	case MENU_ICON_DISKINFO:
		lister_diskinfo(lister);
		break;

	// Lock position
	case MENU_LISTER_LOCK_POS:

		// Toggle state
		if (lister->flags & LISTERF_LOCK_POS)
			lister->flags &= ~LISTERF_LOCK_POS;
		else
			lister->flags |= LISTERF_LOCK_POS;

		// Close lister and re-open it
		lister_close(lister, 0);
		lister_open(lister, GUI->screen_pointer);

		// Activate the window
		ActivateWindow(lister->window);
		break;

	// View by icon
	case MENU_LISTER_VIEW_ICON:
	case MENU_LISTER_ICON_ACTION:

		// Is lister not open?
		if (!lister->window)
		{
			// Set icon mode flag
			lister->flags |= LISTERF_VIEW_ICONS;

			// Icon action?
			if (func == MENU_LISTER_ICON_ACTION)
				lister->flags |= LISTERF_ICON_ACTION;

			// Clear source, etc flags
			else
				lister->flags &= ~(LISTERF_SOURCE | LISTERF_DEST | LISTERF_SOURCEDEST_LOCK | LISTERF_ICON_ACTION);
			break;
		}

		// Can't change mode if busy
		if (!(lister->flags & LISTERF_BUSY))
		{
			struct IBox dims, current;
			short extra;

			// Get extra height
			extra = ((lister->toolbar) ? lister->toolbar->height : 0) + lister->window->WScreen->RastPort.TxHeight +
					FIELD_FONT->tf_YSize + 6;

			// Already in icon action mode?
			if (func == MENU_LISTER_ICON_ACTION)
			{
				if (lister->flags & LISTERF_VIEW_ICONS && lister->flags & LISTERF_ICON_ACTION)
					break;
			}

			// Already in view icon mode?
			else if (lister->flags & LISTERF_VIEW_ICONS && !(lister->flags & LISTERF_ICON_ACTION))
				break;

			// Get current dimensions
			dims = *((struct IBox *)&lister->window->LeftEdge);
			current = dims;

			// Changing from text to icon mode?
			if (!(lister->flags & LISTERF_VIEW_ICONS))
			{
				// Get other dimensions
				dims = lister->other_dims;

				// Store new dimensions
				lister->other_dims = current;

				// Update current position if allowed to
				if (!(environment->env->lister_options & LISTEROPTF_SNAPSHOT))
					PositionUpdate(lister, 0);

				// Filter icons on?
				if (lister->format.flags & LFORMATF_REJECT_ICONS)
				{
					// Turn it off
					lister->format.flags &= ~LFORMATF_REJECT_ICONS;
					lister->format.flags |= LFORMATF_TEMP_ICONS;

					// Resort lister
					lister_resort(lister, RESORT_REJECTS);
				}

				// Going into icon action mode?
				if (func == MENU_LISTER_ICON_ACTION)
				{
					// Add vertical height
					dims.Height += extra;
				}
			}

			// Otherwise, already in icon mode
			else
			{
				// Going into icon action mode?
				if (func == MENU_LISTER_ICON_ACTION)
				{
					// Add vertical height
					dims.Height += extra;
				}

				// Going into icon mode
				else
				{
					// Subtract vertical height
					dims.Height -= extra;
					if (dims.Height < lister->window->MinHeight)
						dims.Height = lister->window->MinHeight;
				}
			}

			// Set icon mode flag
			lister->flags |= LISTERF_VIEW_ICONS;

			// Icon action?
			if (func == MENU_LISTER_ICON_ACTION)
				lister->flags |= LISTERF_ICON_ACTION;

			// Clear source, etc flags
			else
				lister->flags &= ~(LISTERF_SOURCE | LISTERF_DEST | LISTERF_SOURCEDEST_LOCK | LISTERF_ICON_ACTION);
			lister->flags &= ~LISTERF_KEY_SELECTION;

			// If window position is locked, unlock it
			if (lister->flags & LISTERF_LOCK_POS && func != MENU_LISTER_ICON_ACTION)
			{
				// Clear flag; close lister
				lister->flags &= ~LISTERF_LOCK_POS;
				lister_close(lister, 0);

				// Position to use
				lister->dimensions.wd_Normal = dims;

				// Re-open lister
				lister_open(lister, GUI->screen_pointer);
			}

			// Otherwise refresh and resize
			else
				lister_mode_change(lister, &dims);

			// Get icons unless waiting for resize
			if (!(lister->more_flags & LISTERF_MODE_CHANGED))
				IPC_Command(lister->ipc, LISTER_GET_ICONS, 0, 0, 0, 0);
		}

		// Update menu
		display_fix_menu(lister->window, WINDOW_LISTER_ICONS, lister);
		lister_fix_menus(lister, 0);
		break;

	// View by name
	case MENU_LISTER_VIEW_NAME:

		// Is lister not open?
		if (!lister->window)
		{
			// Clear flags
			lister->flags &= ~(LISTERF_VIEW_ICONS | LISTERF_ICON_ACTION);
			break;
		}

		// Can't change mode if busy
		if (!(lister->flags & LISTERF_BUSY))
		{
			struct IBox dims, current;

			// Already in name mode?
			if (!(lister->flags & LISTERF_VIEW_ICONS))
				break;

			// Filter icons off temporarily?
			if (lister->format.flags & LFORMATF_TEMP_ICONS)
			{
				// Turn it back on
				lister->format.flags |= LFORMATF_REJECT_ICONS;
				lister->format.flags &= ~LFORMATF_TEMP_ICONS;

				// Resort lister
				lister_resort(lister, RESORT_REJECTS);
			}

			// Update current position if allowed to
			if (!(environment->env->lister_options & LISTEROPTF_SNAPSHOT))
				PositionUpdate(lister, 0);

			// Get current dimensions
			current = *((struct IBox *)&lister->window->LeftEdge);
			if (lister->flags & LISTERF_ICON_ACTION)
			{
				short extra;

				// Get extra height
				extra = ((lister->toolbar) ? lister->toolbar->height : 0) + lister->window->WScreen->RastPort.TxHeight +
						FIELD_FONT->tf_YSize + 6;

				// Subtract extra amount for toolbar, etc
				current.Height -= extra;
			}

			// Get new dimensions
			dims = lister->other_dims;

			// Store new dimensions
			lister->other_dims = current;

			// Clear flags
			lister->flags &= ~(LISTERF_VIEW_ICONS | LISTERF_ICON_ACTION);

			// Refresh and resize
			lister_mode_change(lister, &dims);
		}

		// Update menu
		display_fix_menu(lister->window, WINDOW_LISTER, lister);
		lister_fix_menus(lister, 0);
		break;

	// Show all
	case MENU_LISTER_SHOW_ALL: {
		BOOL ok = 1;

		// Can't change mode if busy
		if (!(lister->flags & LISTERF_BUSY))
		{
			// If not in icon mode, switch to icon mode
			if (!(lister->flags & LISTERF_VIEW_ICONS))
			{
				// Set show all
				lister->flags |= LISTERF_SHOW_ALL;

				// Switch to icon mode (recursive)
				lister_do_function(lister, MENU_LISTER_VIEW_ICON);
				ok = 0;
			}

			// In icon mode, toggle flag
			else
				lister->flags ^= LISTERF_SHOW_ALL;

			// Fix backdrop flag
			if (lister->flags & LISTERF_SHOW_ALL)
				lister->backdrop_info->flags |= BDIF_SHOW_ALL;
			else
				lister->backdrop_info->flags &= ~BDIF_SHOW_ALL;

			// Get icons
			if (ok)
				IPC_Command(lister->ipc, LISTER_GET_ICONS, 0, 0, 0, 0);
		}

		// Update menu
		if (ok)
		{
			display_fix_menu(lister->window, WINDOW_LISTER_ICONS, lister);
			lister_fix_menus(lister, 0);
		}
	}
	break;

	// Iconify lister
	case MENU_LISTER_ICONIFY:

		// Already iconified?
		if (lister->flags & LISTERF_ICONIFIED)
			break;

		// Try to iconify
		if (lister_iconify(lister))
		{
			// Close window
			lister_close(lister, 0);
		}
		break;

	// Restore
	case MENU_LISTER_RESTORE:
		if (lister->more_flags & LISTERF_TITLEBARRED)
			lister_zoom_window(lister);
		break;

	// Snapshot lister position
	case MENU_LISTER_SNAPSHOT:

		// Window must be open
		if (lister_valid_window(lister))
		{
			// Set busy
			lister_busy(lister, 1);

			// Snapshot owner
			PositionUpdate(lister, POSUPF_SAVE);

			// Clear busy
			lister_unbusy(lister, 1);
		}
		break;

	// Unsnapshot lister position
	case MENU_LISTER_UNSNAPSHOT:

		// Set busy
		lister_busy(lister, 1);

		// UnSnapshot owner
		PositionRemove(lister, TRUE);

		// Clear busy
		lister_unbusy(lister, 1);
		break;

	// Open icons
	case MENU_ICON_OPEN: {
		iconopen_packet *packet;

		// Allocate packet
		if ((packet = get_icon_packet(lister->backdrop_info, 0, 0, 0)))
		{
			// Start process
			if (!(misc_startup("dopus_icon_open", MENU_ICON_OPEN, lister->window, packet, 1)))
				FreeVec(packet);
		}
	}
	break;

	// Snapshot icons
	case MENU_ICON_SNAPSHOT_ICON:
	case MENU_ICON_UNSNAPSHOT:

		// Set busy
		lister_busy(lister, 1);

		// Snapshot icons
		backdrop_snapshot(lister->backdrop_info, (func == MENU_ICON_UNSNAPSHOT), 0, 0);

		// Clear busy
		lister_unbusy(lister, 1);
		break;

	// Snapshot window/all
	case MENU_ICON_SNAPSHOT_WINDOW:
	case MENU_ICON_SNAPSHOT_ALL: {
		// Set busy pointer
		lister_busy(lister, 1);

		// Snapshot icons
		if (func == MENU_ICON_SNAPSHOT_ALL)
			backdrop_snapshot(lister->backdrop_info, 0, 1, 0);

		// Snapshot owner
		PositionUpdate(lister, POSUPF_SAVE);

		// Clear busy pointer
		lister_unbusy(lister, 1);
	}
	break;

	// Select All
	case MENU_ICON_SELECT_ALL:

		// Icon mode?
		if (lister->flags & LISTERF_VIEW_ICONS)
		{
			backdrop_select_all(lister->backdrop_info, 1);
		}

		// Name mode
		else
		{
			function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_all, lister);
		}
		break;

	// Clean Up
	case MENU_ICON_CLEANUP:
		backdrop_cleanup(lister->backdrop_info, 0, 0);
		break;

	// Arrange in a special way
	case MENU_LISTER_ARRANGE_NAME:
	case MENU_LISTER_ARRANGE_TYPE:
	case MENU_LISTER_ARRANGE_SIZE:
	case MENU_LISTER_ARRANGE_DATE:

		// Do cleanup
		lister_busy(lister, 1);
		backdrop_cleanup(lister->backdrop_info, BSORT_NAME + (func - MENU_LISTER_ARRANGE_NAME), 0);
		lister_unbusy(lister, 1);
		break;

	// Rename
	case MENU_ICON_RENAME:

		// Icon mode?
		if (lister->flags & LISTERF_VIEW_ICONS)
		{
			iconopen_packet *packet;

			// Get packet
			if ((packet = get_icon_packet(lister->backdrop_info, 0, 0, 0)))
			{
				// Start process
				if (!(misc_startup("dopus_icon_rename", MENU_ICON_RENAME, lister->window, packet, 0)))
					FreeVec(packet);
			}
		}

		// Name mode
		else
		{
			function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_rename, lister);
		}
		break;

	// Delete
	case MENU_ICON_DELETE:

		// Icon mode?
		if (lister->flags & LISTERF_VIEW_ICONS)
		{
			// Device/cache list?
			if (lister->cur_buffer->more_flags & (DWF_DEVICE_LIST | DWF_CACHE_LIST))
			{
				iconopen_packet *packet;

				// Get packet
				if ((packet = get_icon_packet(lister->backdrop_info, 0, 0, 0)))
				{
					// Start process
					if (!(misc_startup("dopus_icon_delete", MENU_ICON_DELETE, lister->window, packet, 0)))
						FreeVec(packet);
				}
			}

			// Normal delete function
			else
				icon_function(lister->backdrop_info, 0, 0, def_function_delete, 0);
		}

		// Name mode
		else
		{
			function_launch_quick(FUNCTION_RUN_FUNCTION, def_function_delete, lister);
		}
		break;

	// Makedir
	case MENU_ICON_MAKEDIR:
		function_launch(FUNCTION_RUN_FUNCTION,
						def_function_makedir,
						0,
						(lister->flags & LISTERF_VIEW_ICONS) ? FUNCF_ICONS : 0,
						lister,
						0,
						lister->cur_buffer->buf_Path,
						0,
						0,
						0,
						0);
		break;

	// Open parent
	case MENU_OPEN_PARENT:

		// Copy path
		strcpy(lister->work_buffer, lister->cur_buffer->buf_Path);

		// Get parent and read into new lister
		if (!(path_parent(lister->work_buffer)) || !(read_new_lister(lister->work_buffer, lister, 0)))
		{
			// Error
			DisplayBeep(lister->window->WScreen);
		}
		break;

	// Reset icons
	case MENU_ICON_RESET:

		// Set busy pointer
		SetBusyPointer(lister->window);

		// Free objects
		backdrop_free_list(lister->backdrop_info);

		// Get icons
		IPC_Command(lister->ipc, LISTER_GET_ICONS, 1, 0, 0, 0);

		// Clear busy pointer
		ClearPointer(GUI->window);
		break;

	// Don't understand it
	default:
		result = -1;
		break;
	}

	// Update status bar?
	if (show_status && lister->window)
		lister_show_status(lister);

	return result;
}
