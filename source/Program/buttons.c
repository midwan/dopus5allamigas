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

// Create a new button bank
Buttons *buttons_new(char *name, Cfg_ButtonBank *bank, struct IBox *box, short type, ULONG flags)
{
	Buttons *buttons;
	IPCData *ipc;

	// Allocate new buttons structure
	if (!(buttons = AllocVec(sizeof(Buttons), MEMF_CLEAR)))
	{
		if (bank)
			CloseButtonBank(bank);
		return 0;
	}

	// Initialise icon position
	buttons->icon_pos_x = NO_ICON_POSITION;
	buttons->icon_pos_y = NO_ICON_POSITION;

	// Need to ask for file?
	if (name == (char *)-1)
		buttons->flags = BUTTONF_LOAD;

	// Bank supplied?
	else if (bank)
		buttons->bank = bank;

	// Filename supplied?
	else if (name)
		strcpy(buttons->buttons_file, name);

	// Create new bank
	else if (!(buttons->bank = NewButtonBank(1, type)))
	{
		FreeVec(buttons);
		return 0;
	}

	// Carry some flags over
	buttons->flags |= flags & (BUTTONF_ICONIFIED | BUTTONF_TOOLBAR | BUTTONF_UNDERMOUSE | BUTTONF_FAIL);

	// Store position
	if (box)
	{
		buttons->pos = *box;
		buttons->flags |= BUTTONF_COORDS_SUPPLIED;
	}

	// Set first-time flag
	buttons->flags |= BUTTONF_FIRST_TIME;

	// Start buttons process
	if (!(IPC_Launch(&GUI->buttons_list,
					 &ipc,
					 "dopus_buttons",
					 (IPTR)&buttons_code,
					 STACK_DEFAULT,
					 (IPTR)buttons,
					 (struct Library *)DOSBase)))
	{
		// Free stuff if process never got off the ground
		if (!ipc)
		{
			CloseButtonBank(buttons->bank);
			FreeVec(buttons);
		}
		return 0;
	}

	return buttons;
}

// Update buttons data
void buttons_update(Buttons *buttons)
{
	if (buttons && buttons->bank)
	{
		// If window is open, get current position
		if (buttons->window)
		{
			buttons->pos.Left = buttons->window->LeftEdge;
			buttons->pos.Top = buttons->window->TopEdge;
			buttons->pos.Width = buttons->window->GZZWidth;
			buttons->pos.Height = buttons->window->GZZHeight;
		}

		// Update icon position
		buttons_update_icon(buttons);
	}
}

// Get button font
void buttons_get_font(Buttons *buttons)
{
	struct TextAttr attr;
	char name[80];

	// Close existing font
	if (buttons->font)
		CloseFont(buttons->font);

	// Lock bank
	GetSemaphore(&buttons->bank->lock, SEMF_SHARED, 0);

	// Fill out attr
	strcpy(name, buttons->bank->window.font_name);
	attr.ta_Name = name;
	attr.ta_YSize = buttons->bank->window.font_size;
	attr.ta_Style = 0;
	attr.ta_Flags = 0;

	// Unlock bank
	FreeSemaphore(&buttons->bank->lock);

	// Get font
	buttons->font = OpenDiskFont(&attr);

	// Set font
	if (buttons->window && buttons->font)
		SetFont(buttons->window->RPort, buttons->font);
}

// Update icon position
void buttons_update_icon(Buttons *buttons)
{
	BackdropObject *object;

	// Got an icon?
	if (!buttons->appicon)
		return;

	// Lock main icon list
	lock_listlock(&GUI->backdrop->objects, 0);

	// Find object
	if ((object = backdrop_find_appicon(GUI->backdrop, (AppEntry *)buttons->appicon)))
	{
		short x, y;
		ULONG iflags;

		// Get position
		x = object->pos.Left;
		y = object->pos.Top;

		// Adjust for borders?
		if (!((iflags = GetIconFlags(object->icon)) & ICONF_BORDER_OFF) &&
			(!(environment->env->desktop_flags & DESKTOPF_NO_BORDERS) || (iflags & ICONF_BORDER_ON)))
		{
			// Shift back by border size
			x -= ICON_BORDER_X;
			y -= ICON_BORDER_Y_TOP;
		}

		// Save position
		buttons->icon_pos_x = x;
		buttons->icon_pos_y = y;
	}

	// Unlock main icon list
	unlock_listlock(&GUI->backdrop->objects);
}
