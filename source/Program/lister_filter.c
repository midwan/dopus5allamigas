/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team

Live filter feature
Copyright 2026 Dimitris Panokostas <midwan@gmail.com>

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

// Open the live filter requester over the path field.
void lister_add_filter(Lister *lister)
{
	struct Gadget *gadget;
	struct Hook *hook;
	ListFormat *format;

	// Already open?
	if (lister->filter_name_req)
		return;

	// Need a path field to anchor over
	if (!(lister->flags & LISTERF_PATH_FIELD))
		return;

	// Only makes sense in name view; icon view uses a different backdrop path
	if (lister->flags & LISTERF_VIEW_ICONS)
		return;

	// Allocate requester
	if (!(lister->filter_name_req = AllocVec(sizeof(struct Requester), MEMF_CLEAR)))
		return;

	// Get edit hook that signals this task on every keystroke
	if (!(hook = GetEditHookTags(0,
								 OBJECTF_NO_SELECT_NEXT,
								 GTCustom_ChangeSigTask,
								 (IPTR)FindTask(0),
								 GTCustom_ChangeSigBit,
								 lister->filter_name_bit,
								 TAG_END)))
	{
		FreeVec(lister->filter_name_req);
		lister->filter_name_req = 0;
		return;
	}

	// Pre-fill buffer with the current show pattern
	format = &lister->format;
	stccpy(lister->filter_name, format->show_pattern, sizeof(lister->filter_name));

	// Create string gadget
	if (!(gadget = NewObject(0,
							 "dopusstrgclass",
							 GA_ID,
							 GAD_PATH,
							 GA_RelWidth,
							 0,
							 GA_RelHeight,
							 0,
							 GA_RelVerify,
							 TRUE,
							 STRINGA_MaxChars,
							 sizeof(lister->filter_name) - 1,
							 STRINGA_Buffer,
							 (IPTR)lister->filter_name,
							 STRINGA_Font,
							 (IPTR)FIELD_FONT,
							 STRINGA_EditHook,
							 (IPTR)hook,
							 TAG_END)))
	{
		FreeEditHook(hook);
		FreeVec(lister->filter_name_req);
		lister->filter_name_req = 0;
		return;
	}

	gadget->GadgetType |= GTYP_REQGADGET;

	// Position the requester over the path field
	lister->filter_name_req->LeftEdge = lister->name_area.box.Left + (lister->name_area.box.Height >> 1);
	lister->filter_name_req->TopEdge = lister->name_area.box.Top + (lister->name_area.box.Height >> 1);
	lister->filter_name_req->Width = lister->name_area.box.Width;
	lister->filter_name_req->Height = FIELD_FONT->tf_YSize + 4;

	lister->filter_name_req->ReqGadget = gadget;
	lister->filter_name_req->Flags = NOISYREQ | SIMPLEREQ | NOREQBACKFILL;

	if (!(Request(lister->filter_name_req, lister->window)))
	{
		lister_rem_filter(lister);
		return;
	}

	ActivateGadget(gadget, lister->window, lister->filter_name_req);

	lister->filter_name_hook = hook;
}

// Close the live filter requester; the pattern itself persists in lister->format.
void lister_rem_filter(Lister *lister)
{
	if (lister->filter_name_req)
	{
		EndRequest(lister->filter_name_req, lister->window);
		DisposeObject(lister->filter_name_req->ReqGadget);
		FreeEditHook(lister->filter_name_hook);
		lister->filter_name_hook = 0;
		FreeVec(lister->filter_name_req);
		lister->filter_name_req = 0;
	}
}

// Apply the current filter buffer to the lister's show pattern and re-reject.
void lister_handle_filter(Lister *lister)
{
	ListFormat *format = &lister->format;

	// Short-circuit if the buffer matches what we already have compiled
	if (stricmp(lister->filter_name, format->show_pattern) == 0)
		return;

	// Copy edit buffer into the format's show pattern and re-parse
	stccpy(format->show_pattern, lister->filter_name, sizeof(format->show_pattern));
	ParsePatternNoCase(format->show_pattern, format->show_pattern_p, sizeof(format->show_pattern_p));

	// Re-apply rejects against all buffered entries — no directory re-read
	lister_resort(lister, RESORT_REJECTS);

	// Refresh visible area
	lister_refresh(lister, LREFRESH_FULL);
}
