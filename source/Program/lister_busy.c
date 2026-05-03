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

// Make a lister busy
// Called from the LISTER PROCESS
void lister_busy(Lister *lister, int lock_only)
{
	// Resets abort flag
	lister->flags &= ~LISTERF_ABORTED;

	// Make sure we're not already locked
	if (!(lister->flags & LISTERF_LOCK))
	{
		// Set lock flag
		lister->flags |= LISTERF_LOCK;

		// Is lister open?
		if (lister->window)
		{
			// Set busy pointer
			SetBusyPointer(lister->window);

			// Disable path gadget
			lister_disable_pathfield(lister, TRUE);

			// Stop any dragging that's going on
			if (lister->flags & LISTERF_VIEW_ICONS)
				backdrop_stop_drag(lister->backdrop_info);

			// Fix menus
			lister_fix_menus(lister, 0);
		}
	}

	// Want to just lock lister?
	if (lock_only)
	{
		lister->flags |= LISTERF_LOCKED;
	}

	// Or make window busy
	else if (!(lister->flags & LISTERF_BUSY))
	{
		// If window is source or destination, store that status
		if (lister->flags & LISTERF_SOURCE)
		{
			lister->flags |= LISTERF_STORED_SOURCE;
			lister->flags &= ~LISTERF_SOURCE;
		}
		if (lister->flags & LISTERF_DEST)
		{
			lister->flags |= LISTERF_STORED_DEST;
			lister->flags &= ~LISTERF_DEST;
		}

		// Set busy flag
		lister->flags |= LISTERF_BUSY;
		lister->flags &= ~LISTERF_BUSY_VISUAL;

		// Send timer request
		StartTimer(lister->busy_timer, 2, 0);

		// Fix priority
		lister_fix_priority(lister);

		// If we're iconified, change icon
		if (lister->flags & LISTERF_ICONIFIED)
			lister_set_busy_icon(lister);
	}
}

// Turn busy pointer off
// Called from the LISTER PROCESS
void lister_unbusy(Lister *lister, int lock_only)
{
	// Clear locked flag
	lister->flags &= ~LISTERF_LOCKED;

	// Want to stop being busy
	if (!lock_only && lister->flags & LISTERF_BUSY)
	{
		// If window was source or destination, see if it still can be
		check_lister_stored(lister);

		// Abort timer
		StopTimer(lister->busy_timer);

		// Clear flag
		lister->flags &= ~LISTERF_BUSY;
		if (lister->window)
			lister_show_status(lister);

		// Clear locker pointer
		lister->locker_ipc = 0;

		// Fix priority
		lister_fix_priority(lister);

		// Also turns the progress indicator off
		if (lister->flags & LISTERF_PROGRESS)
			lister_progress_off(lister);

		// If we're iconified, fix icon
		if (lister->flags & LISTERF_ICONIFIED)
			lister_clear_busy_icon(lister);
	}

	// Unlock window?
	if (lister->flags & LISTERF_LOCK && !(lister->flags & (LISTERF_BUSY | LISTERF_LOCKED)))
	{
		// Clear lock flags
		lister->flags &= ~LISTERF_LOCK;

		// Is window open?
		if (lister->window)
		{
			// Clear pointer
			ClearPointer(lister->window);

			// Enable path gadget if needed
			if (lister->flags & LISTERF_PATH_FIELD)
				lister_disable_pathfield(lister, FALSE);

			// Fix menus
			lister_fix_menus(lister, 0);
		}
	}

	// Clear old flags too
	lister->old_flags &= ~(LISTERF_LOCK | LISTERF_BUSY | LISTERF_LOCKED);
	lister->old_flags |= lister->flags & (LISTERF_LOCK | LISTERF_BUSY | LISTERF_LOCKED);
}

// If lister was source or destination, see if it still can be
void check_lister_stored(Lister *lister)
{
	Lister *current;

	// If not special buffer
	if (lister->cur_buffer != lister->special_buffer)
	{
		if (lister->flags & LISTERF_STORED_SOURCE)
		{
			if (lister->flags & LISTERF_SOURCEDEST_LOCK || !(current = lister_source()) ||
				current->flags & LISTERF_SOURCEDEST_LOCK)
				lister->flags |= LISTERF_SOURCE;
			lister->flags &= ~LISTERF_STORED_SOURCE;
		}

		if (lister->flags & LISTERF_STORED_DEST)
		{
			if (lister->flags & LISTERF_SOURCEDEST_LOCK || !(current = lister_dest()) ||
				current->flags & LISTERF_SOURCEDEST_LOCK)
				lister->flags |= LISTERF_DEST;
			lister->flags &= ~LISTERF_STORED_DEST;
		}
	}
}

// Set busy icon
void lister_set_busy_icon(Lister *lister)
{
	// Rebuild title
	lister_build_icon_name(lister);

	// Got icon?
	if (lister->appicon)
	{
		// Change to busy image
		ChangeAppIcon(lister->appicon,
					  0,  // (struct Image *)GUI->lister_icon_busy->do_Gadget.GadgetRender,
					  0,  // (struct Image *)GUI->lister_icon_busy->do_Gadget.SelectRender,
					  0,
					  CAIF_GHOST | CAIF_SET);  // CAIF_RENDER|CAIF_SELECT);
	}
}

// Clear busy icon
void lister_clear_busy_icon(Lister *lister)
{
	// Rebuild title
	lister_build_icon_name(lister);

	// Got icon?
	if (lister->appicon)
	{
		// Change to original image
		ChangeAppIcon(lister->appicon,
					  0,  // (struct Image *)GUI->lister_icon->do_Gadget.GadgetRender,
					  0,  // (struct Image *)GUI->lister_icon->do_Gadget.SelectRender,
					  0,
					  CAIF_GHOST);	// CAIF_RENDER|CAIF_SELECT);
	}
}
