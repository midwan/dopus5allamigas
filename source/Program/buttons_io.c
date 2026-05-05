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

// Save a button bank
int buttons_save(Buttons *buttons, char *name)
{
	short err;

	// Make window busy
	buttons_busy(buttons);

	// Update window size
	if (buttons->window)
	{
		// Get position
		buttons->bank->window.pos.Left = buttons->window->LeftEdge;
		buttons->bank->window.pos.Top = buttons->window->TopEdge;
		buttons->bank->window.pos.Width = buttons->window->GZZWidth;
		buttons->bank->window.pos.Height = buttons->window->GZZHeight;
	}

	// Save bank
	if ((err = SaveButtonBank(buttons->bank, name)))
	{
		error_saving(err, buttons->window, MSG_BUTTONS_ERROR_SAVING, 0);
	}

	// Clear changed flag
	else
		buttons->flags &= ~BUTTONF_CHANGED;

	// Make window unbusy
	buttons_unbusy(buttons);

	return 1;
}

// Save as button bank
void buttons_saveas(Buttons *buttons)
{
	char path[256];

	// Get current path
	strcpy(path, buttons->bank->path);

	// Make window busy
	buttons_busy(buttons);

	// Update window size
	if (buttons->window)
		buttons->bank->window.pos = *((struct IBox *)&buttons->window->LeftEdge);

	// Ask for filename
	while (buttons_request_file(
		buttons, GetString(&locale, MSG_BUTTONS_ENTER_NAME), path, "DOpus5:Buttons/", FRF_DOSAVEMODE))
	{
		short err;

		// Save bank
		if (!(err = SaveButtonBank(buttons->bank, path)))
		{
			// Store path
			strcpy(buttons->bank->path, path);
			strcpy(buttons->last_saved, buttons->bank->path);

			// Clear changed flag
			buttons->flags &= ~BUTTONF_CHANGED;
			break;
		}

		// Display requester
		if (!(error_saving(err, buttons->window, MSG_BUTTONS_ERROR_SAVING, 1)))
			break;
	}

	// Make window unbusy
	buttons_unbusy(buttons);

	// Refresh bank
	buttons_refresh(buttons, BUTREFRESH_REFRESH);
}

// Open a button bank
int buttons_load(Buttons *buttons, struct Screen *screen, char *name)
{
	char path[256];
	Cfg_ButtonBank *bank = 0;
	struct Window *status = 0;

	// Name supplied?
	if (name)
		strcpy(path, name);

	// Get current path
	else if (buttons->bank && buttons->bank->path[0])
		strcpy(path, buttons->bank->path);
	else
		path[0] = 0;

	// Make window busy
	buttons_busy(buttons);

	// Loop while unsuccessful
	while (name || buttons_request_file(
					   buttons, GetString(&locale, MSG_BUTTONS_SELECT_FILE), path, "DOpus5:Buttons/", FRF_PRIVATEIDCMP))
	{
		// Open status window
		status = OpenStatusWindow(GetString(&locale, MSG_BUTTONS_STATUS_TITLE),
								  GetString(&locale, MSG_BUTTONS_LOADING),
								  screen,
								  0,
								  WINDOW_NO_CLOSE);

		// Load bank
		if ((bank = OpenButtonBank(path)))
		{
			// Store path
			strcpy(bank->path, path);
			break;
		}

		// Close status window
		CloseConfigWindow(status);
		status = 0;

		// Display requester
		if (!(AsyncRequestTags(buttons->ipc,
							   REQTYPE_SIMPLE,
							   buttons->window,
							   GET_CALLBACK(buttons_refresh_callback),
							   buttons,
							   AR_Window,
							   buttons->window,
							   AR_Screen,
							   screen,
							   AR_Message,
							   GetString(&locale, MSG_BUTTONS_ERROR_LOADING),
							   AR_Button,
							   GetString(&locale, MSG_RETRY),
							   AR_Button,
							   GetString(&locale, MSG_CANCEL),
							   TAG_END)))
			break;
	}

	// Close status window
	CloseConfigWindow(status);

	// Make window unbusy
	buttons_unbusy(buttons);

	// Valid bank?
	if (bank)
	{
		// Free existing bank
		CloseButtonBank(buttons->bank);

		// Store pointer to new bank
		buttons->bank = bank;
		return 1;
	}

	return 0;
}

// See whether a button bank has been changed
int buttons_check_change(Buttons *buttons, BOOL can_abort)
{
	char text[128];
	short ret;

	// Valid bank?
	if (!buttons || !buttons->bank)
		return 1;

	// Check change flag
	if (!(buttons->flags & BUTTONF_CHANGED))
		return 1;

	// Make window busy
	buttons_busy(buttons);

	// Bank has been changed; build requester text
	lsprintf(text, GetString(&locale, MSG_BUTTONS_CHANGE_WARNING), buttons->bank->window.name);

	// Display requester
	ret = AsyncRequestTags(buttons->ipc,
						   REQTYPE_SIMPLE,
						   buttons->window,
						   GET_CALLBACK(buttons_refresh_callback),
						   buttons,
						   AR_Window,
						   buttons->window,
						   AR_Message,
						   text,
						   AR_Button,
						   GetString(&locale, MSG_SAVE),
						   AR_Button,
						   GetString(&locale, MSG_DISCARD),
						   (can_abort) ? AR_Button : TAG_IGNORE,
						   (can_abort) ? GetString(&locale, MSG_CANCEL) : 0,
						   TAG_END);

	// Make window unbusy
	buttons_unbusy(buttons);

	// Cancel/Discard?
	if (ret == 0)
		return (!can_abort);

	// Do save as
	if (ret == 1)
		buttons_saveas(buttons);
	return 1;
}

// Get a filename
int buttons_request_file(Buttons *buttons, char *title, char *buffer, char *def, ULONG flags)
{
	char *path, *file = 0;
	short ret;

	// Allocate path
	if (!(path = AllocVec(300, MEMF_CLEAR)))
		return 0;

	// Get current path
	if (buffer[0])
		strcpy(path, buffer);
	else if (def)
		strcpy(path, def);

	// Get file pointer
	if (*path)
	{
		file = FilePart(path);
		if (file && file > path)
		{
			strcpy(path + 256, file);
			*file = 0;
			file = path + 256;
		}
	}

	// Show requester
	if ((ret = AsyncRequestTags(buttons->ipc,
								REQTYPE_FILE,
								buttons->window,
								GET_CALLBACK(buttons_refresh_callback),
								buttons,
								AR_Requester,
								buttons->filereq,
								ASLFR_Screen,
								GUI->screen_pointer,
								ASLFR_TitleText,
								title,
								ASLFR_InitialFile,
								file,
								ASLFR_InitialDrawer,
								path,
								ASLFR_Flags1,
								flags | FRF_PRIVATEIDCMP,
								ASLFR_Flags2,
								FRF_REJECTICONS,
								TAG_END)))
	{
		// Build path
		strcpy(buffer, ((struct FileRequester *)buttons->filereq)->fr_Drawer);
		AddPart(buffer, ((struct FileRequester *)buttons->filereq)->fr_File, 256);
	}

	// Free buffer
	FreeVec(path);

	return ret;
}

// Callback to refresh task
REF_CALLBACK_BEGIN(void,
				   ASM SAVEDS buttons_refresh_callback,
				   d0,
				   ULONG,
				   type,
				   a0,
				   struct Window *,
				   window,
				   a1,
				   Buttons *,
				   buttons)
{
	// Size?
	if (type == IDCMP_CHANGEWINDOW)
	{
		BOOL size;

		// Get new internal dimensions
		buttons_fix_internal(buttons);

		// Size changed?
		size = (buttons->pos.Width != window->GZZWidth || buttons->pos.Height != window->GZZHeight);

		// Store size
		buttons->pos.Left = window->LeftEdge;
		buttons->pos.Top = window->TopEdge;
		buttons->pos.Width = window->GZZWidth;
		buttons->pos.Height = window->GZZHeight;

		// Need to redraw?
		if (size)
		{
			// Set flag to indicate we've resized
			buttons->flags |= BUTTONF_HAVE_RESIZED;

			// Refresh buttons display
			buttons_refresh(buttons, BUTREFRESH_RESIZE | BUTREFRESH_REFRESH);
		}
	}

	// Refresh
	else
	{
		BOOL resize = 0;

		// Turn off highlight
		if (buttons->flags & BUTTONF_HIGH_SHOWN)
			buttons_show_highlight(buttons);

		// Resize?
		if (buttons->flags & BUTTONF_HAVE_RESIZED)
		{
			buttons->flags &= ~BUTTONF_HAVE_RESIZED;
			resize = 1;
		}

		// Lock layers except for resize
		else
		{
#ifdef LOCKLAYER_OK
			LockLayerInfo(&window->WScreen->LayerInfo);
#else
			Forbid();
#endif
		}

		// Start refresh
		BeginRefresh(buttons->window);

		// Not a resize?
		if (!resize)
		{
			// Refresh buttons
			buttons_refresh(buttons, BUTREFRESH_REFRESH);

			// Unlock layers
#ifdef LOCKLAYER_OK
			UnlockLayerInfo(&window->WScreen->LayerInfo);
#else
			Permit();
#endif
		}

		// End refresh
		EndRefresh(buttons->window, TRUE);
	}
}
