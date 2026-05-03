#include "config_lib.h"
#include "config_buttons.h"

// struct Library *CxBase;

// Create some new buttons
BOOL _config_buttons_create_new(config_buttons_data *data, struct List *list, short count)
{
	short num;
	Cfg_Button *button = 0;

	// Initialise list
	NewList(list);

	// Create the buttons
	for (num = 0; num < count; num++)
	{
		// Create button
		if ((button = NewButton(data->bank_node->bank->memory)))
		{
			// Add to temporary list
			AddTail(list, &button->node);

			// Set type according to bank type
			if (data->bank_node->bank->window.flags & BTNWF_GFX)
				button->button.flags |= BUTNF_GRAPHIC;
		}
		else
			break;
	}

	// Did we fail to get all we wanted?
	if (!button)
	{
		// Free list
		FreeButtonList(list);
		return 0;
	}

	return 1;
}

// Get current row/column
short _config_buttons_get_colrow(config_buttons_data *data, short *col, short *row)
{
	struct colrow_data crdata;

	// Get current column and row
	if (!(IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_GET_SELECTION, 0, &crdata, 0, REPLY_NO_PORT)))
		return -1;

	// Store column and row
	if (col)
		*col = crdata.col;
	if (row)
		*row = crdata.row;

	return 0;
}

Cfg_Button *_config_buttons_get_button(config_buttons_data *data, short col, short row)
{
	return (Cfg_Button *)IPC_Command(
		data->bank_node->button_ipc, BUTTONEDIT_GET_BUTTON, col, (APTR)row, 0, REPLY_NO_PORT);
}

void _config_buttons_flash(config_buttons_data *data, long state)
{
	if (data->bank_node)
		IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_FLASH, state, 0, 0, 0);
}

void _config_buttons_redraw_button(config_buttons_data *data, bank_node *bank, Cfg_Button *button)
{
	if (bank)
		IPC_Command(bank->button_ipc, BUTTONEDIT_REDRAW_BUTTON, 0, button, 0, 0);
}

// Edit a button
void _config_buttons_edit_button(config_buttons_data *data, short col, short row)
{
	Cfg_Button *button;
	ButtonEdStartup *startup;
	IPCData *ipc;
	BOOL success = 0;
	edit_node *node;

	// Get data
	if (!(startup = AllocVec(sizeof(ButtonEdStartup), MEMF_CLEAR)))
		return;
	startup->command_list = data->command_list;

	// Lock bank
	GetSemaphore(&data->bank_node->bank->lock, SEMF_SHARED, 0);

	// Get button
	if ((button = _config_buttons_get_button(data, col, row)))
	{
		// See if this button is already being edited
		for (node = (edit_node *)data->edit_list.lh_Head; node->node.ln_Succ; node = (edit_node *)node->node.ln_Succ)
		{
			// Is it in the list?
			if (node->button == button)
			{
				// Tell it to activate
				IPC_Command(node->editor, IPC_ACTIVATE, 0, 0, 0, 0);
				break;
			}
		}

		// Get a copy of the button (unless we're already editing it)
		if (!node->node.ln_Succ && (startup->button = CopyButton(button, 0, -1)))
		{
			// Fill out data
			startup->window = data->window;
			startup->owner_ipc = data->ipc;
			startup->main_owner = data->owner_ipc;
			startup->retdata.bank = data->bank_node;
			startup->retdata.orig_button = button;
			startup->retdata.button = startup->button;

			// Initialise screen data
			startup->palette_data.screen_data = data->screen_data;

			// Initialise things
			button_editor_init(startup);

			// Graphical button?
			if (button->button.flags & BUTNF_GRAPHIC)
			{
				// No foreground colour
				startup->palette_data.flags |= PBF_NO_FG;
			}

			// No palette editing or multiple functions for toolbars
			if (data->bank_node->toolbar)
			{
				startup->flags |= BUTTONEDF_NO_ADD;
				startup->palette_data.flags |= PBF_NO_EDIT;
			}

			// Launch editor
			if ((IPC_Launch(&data->bank_node->proc_list,
							&ipc,
							"dopus_button_editor",
							(ULONG)IPC_NATIVE(ButtonEditor),
							STACK_DEFAULT,
							(ULONG)startup,
							(struct Library *)DOSBase)) &&
				ipc)
			{
				success = 1;

				// Add to edit list
				if ((node = AllocVec(sizeof(edit_node), MEMF_CLEAR)))
				{
					node->button = button;
					node->editor = ipc;
					AddTail(&data->edit_list, &node->node);
				}
			}
		}
	}

	// Unlock bank
	FreeSemaphore(&data->bank_node->bank->lock);

	// Free data if not successful
	if (!success)
	{
		if (startup)
		{
			FreeButton(startup->button);
			FreeVec(startup);
		}
	}
}

// Receive an edited button
BOOL _config_buttons_receive_edit(config_buttons_data *data, ButtonReturn *ret, BOOL redraw)
{
	bank_node *bank;
	BOOL success = 0;

	// Stop the select flash
	_config_buttons_flash(data, 0);

	// Look for bank in bank list
	for (bank = (bank_node *)data->bank_list.lh_Head; bank->node.ln_Succ; bank = (bank_node *)bank->node.ln_Succ)
	{
		if (bank == ret->bank)
			break;
	}

	// Did we find the bank?
	if (bank == ret->bank)
	{
		Cfg_Button *button, *after = 0;

		// Lock bank
		GetSemaphore(&bank->bank->lock, SEMF_EXCLUSIVE, 0);

		// Look for button in bank
		for (button = (Cfg_Button *)bank->bank->buttons.lh_Head; button->node.ln_Succ;
			 button = (Cfg_Button *)button->node.ln_Succ)
		{
			// Is this the button we want?
			if (button == ret->orig_button)
				break;
			after = button;
		}

		// Did we find the button?
		if (button->node.ln_Succ)
		{
			Cfg_Button *button_copy;

			// Try to copy the new button
			if ((button_copy = CopyButton(ret->button, bank->bank->memory, bank->bank->window.flags & BTNWF_GFX)))
			{
				// Remove and free existing button
				Remove(&button->node);
				FreeButton(button);

				// Add new button to button list
				Insert(&bank->bank->buttons, &button_copy->node, (struct Node *)after);

				// Set success flag
				success = 1;
				button = button_copy;
			}
		}

		// Unlock bank
		FreeSemaphore(&bank->bank->lock);

		// Need redraw?
		if (success && redraw)
		{
			// If bank is graphical, refresh it completely
			if (bank->bank->window.flags & BTNWF_GFX)
				_config_buttons_refresh(data, bank, BUTREFRESH_RESIZE | BUTREFRESH_REFRESH);

			// Otherwise, just redraw this button
			else
				_config_buttons_redraw_button(data, bank, button);
		}
	}

	// Start the select flash again
	_config_buttons_flash(data, 1);

	return success;
}

// Copy a button to the clipboard
void button_to_clipboard(config_buttons_data *data, Cfg_Button *button)
{
	Cfg_ButtonFunction *func;

	// Add button to list
	AddTail(&data->clip_list, &button->node);

	// Increment clipboard count
	++data->clip_count;

	// Get first function
	func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;

	// Remap if an image
	if (func->node.ln_Succ && func->image)
		RemapImage(func->image, data->window->WScreen, &data->remap);

	// Show clipboard
	show_button_clipboard(data, 3);
}

// Save clipboard list
void button_save_clipboard(config_buttons_data *data, BOOL save)
{
	Cfg_Button *button;
	APTR iff = 0;

	// Open clipboard for IFF
	if (save)
		iff = IFFOpen((char *)1, IFF_CLIP_WRITE, ID_OPUS);

	// Go through button clip list
	for (button = (Cfg_Button *)data->clip_list.lh_Head; button->node.ln_Succ;)
	{
		Cfg_Button *next = (Cfg_Button *)button->node.ln_Succ;

		// Do we have a clipboard file?
		if (iff)
			SaveButton(iff, button);

		// Free this button
		FreeButton(button);

		// Get next button
		button = next;
	}

	// Close IFF file
	IFFClose(iff);

	// Clear clip list
	NewList((struct List *)&data->clip_list);
}

// Load clipboard list
void button_load_clipboard(config_buttons_data *data)
{
	APTR iff;

	// Open clipboard for IFF
	if ((iff = IFFOpen((char *)1, IFF_CLIP_READ, ID_OPUS)))
	{
		Cfg_Button *button;
		Cfg_ButtonFunction *func = 0;
		ULONG chunk;

		// Parse file
		while ((chunk = IFFNextChunk(iff, 0)))
		{
			// Look at chunk type
			switch (chunk)
			{
			// Button definition
			case ID_BUTN:

				// Read button
				if ((button = ReadButton(iff, 0)))
				{
					// Add to clip list
					AddTail(&data->clip_list, &button->node);

					// Get first function
					func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;

					// Increment clipboard count
					++data->clip_count;
				}
				break;

			// Function definition
			case ID_FUNC:

				// Valid function?
				if (func && func->node.ln_Succ)
				{
					// Read function
					ReadFunction(iff, 0, 0, (Cfg_Function *)func);

					// Get next function
					func = (Cfg_ButtonFunction *)func->node.ln_Succ;
				}
				break;
			}
		}

		// Close iff file
		IFFClose(iff);
	}
}

// Show the clipboard list
void show_button_clipboard(config_buttons_data *data, BOOL move)
{
	long top, a;
	Cfg_Button *button;

	// Window open?
	if (!data->clip_window || !data->clip_objlist)
		return;

	// Update clipboard scroller
	if (move & 1)
		SetGadgetChoices(data->clip_objlist, GAD_BUTTONS_CLIPBOARD_SCROLLER, (APTR)data->clip_count);

	// Move to show last entry?
	if (move & 2)
	{
		SetGadgetValue(data->clip_objlist, GAD_BUTTONS_CLIPBOARD_SCROLLER, data->clip_count - 2);
		top = data->clip_count - 2;
	}

	// Get the current top
	else
		top = GetGadgetValue(data->clip_objlist, GAD_BUTTONS_CLIPBOARD_SCROLLER);

	// Find button in the list
	for (button = (Cfg_Button *)data->clip_list.lh_Head, a = 0; button->node.ln_Succ && a < top;
		 button = (Cfg_Button *)button->node.ln_Succ, a++)
		;

	// Button the same as the last one?
	if (button == data->last_clip_draw)
		return;

	// Remember button
	data->last_clip_draw = button;

	// Invalid button, just clear area
	if (!button || !button->node.ln_Succ)
	{
		SetGadgetValue(data->clip_objlist, GAD_BUTTONS_CLIPBOARD_AREA, 0);
		data->clip_valid = 0;
	}

	// Otherwise draw button
	else
	{
		struct IBox box;

		// Get area
		box = AREA(GetObject(data->clip_objlist, GAD_BUTTONS_CLIPBOARD_AREA)).area_pos;
		box.Left++;
		box.Top++;
		box.Height -= 2;
		box.Width -= 2;

		// Draw button
		button_draw(button,
					&box,
					&data->clip_rect,
					data->clip_window,
					FTYPE_LEFT_BUTTON,
					0,
					(short *)data->screen_data.pen_array,
					0,
					0,
					0,
					-1,
					DOpusBase,
					(struct Library *)GfxBase);

		// Set valid flag
		data->clip_valid = 1;
	}
}

// Draw a button
void button_draw(Cfg_Button *button,
				 struct IBox *display_box,
				 struct Rectangle *dest_rect,
				 struct Window *window,
				 short which,
				 Cfg_ButtonFunction *func,
				 short *pen_array,
				 short flags,
				 short fixed_width,
				 short fixed_height,
				 short gfx,
				 struct Library *DOpusBase,
				 struct Library *GfxBase)
{
	short fpen, bpen;
	struct IBox display_area;
	BOOL nodraw = 0;

	// If object is supplied, get display area from that
	if (display_box)
		display_area = *display_box;

	// Otherwise, get from dest_rect
	else
	{
		display_area.Left = dest_rect->MinX;
		display_area.Top = dest_rect->MinY;
		display_area.Width = RECTWIDTH(dest_rect);
		display_area.Height = RECTHEIGHT(dest_rect);
	}

	// Invalid type?
	if (button && gfx != -1)
	{
		if ((gfx && !(button->button.flags & BUTNF_GRAPHIC)) || (!gfx && (button->button.flags & BUTNF_GRAPHIC)))
			button = 0;
	}

	// If button is invalid, assume fixed width is supplied
	if (!button)
	{
		dest_rect->MaxX = fixed_width;
		dest_rect->MaxY = fixed_height;
	}

	// Otherwise
	else
	{
		unsigned short max_width = 0, max_height = 0;
		Cfg_ButtonFunction *func;

		// Got a fixed width?
		if (fixed_width && fixed_height)
		{
			max_width = fixed_width;
			max_height = fixed_height;
		}

		// Otherwise, calculate button size, depending on button type
		else if (button->button.flags & BUTNF_GRAPHIC)
		{
			struct TagItem tags[3];

			// Fill out tags to get info
			tags[0].ti_Tag = IM_Width;
			tags[1].ti_Tag = IM_Height;
			tags[2].ti_Tag = TAG_DONE;

			// Go through images
			for (func = (Cfg_ButtonFunction *)button->function_list.mlh_Head; func->node.ln_Succ;
				 func = (Cfg_ButtonFunction *)func->node.ln_Succ)
			{
				// Initialise data
				tags[0].ti_Data = 0;
				tags[1].ti_Data = 0;

				// Get size of this image
				GetImageAttrs(func->image, tags);

				// Test size against maximum so far
				if (tags[0].ti_Data > max_width)
					max_width = tags[0].ti_Data;
				if (tags[1].ti_Data > max_height)
					max_height = tags[1].ti_Data;
			}

			// No image?
			if (max_width == 0 || max_height == 0)
				nodraw = 1;

			// Add a space around the image
			max_width += 8;
			max_height += 8;

			// Clip to area size
			if (max_width > display_area.Width - 4)
				max_width = display_area.Width - 4;
			if (max_height > display_area.Height - 4)
				max_height = display_area.Height - 4;
		}

		// Text button
		else
		{
			short max_len = 0;

			// Text button; get maximum text length
			for (func = (Cfg_ButtonFunction *)button->function_list.mlh_Head; func->node.ln_Succ;
				 func = (Cfg_ButtonFunction *)func->node.ln_Succ)
			{
				if (func->label)
				{
					short len;

					len = TextLength(window->RPort, func->label, strlen(func->label));
					if (len > max_len)
						max_len = len;
				}
			}

			// Minimum is 8 chars
			if (max_len < window->RPort->Font->tf_XSize * 8)
				max_len = window->RPort->Font->tf_XSize * 8;

			// Button width is text width plus 4
			max_width = max_len + 4;

			// Clip to area size
			if (max_width > display_area.Width - 4)
				max_width = display_area.Width - 4;

			// Get height from font
			max_height = window->RPort->Font->tf_YSize + 4;
		}

		// Store size
		dest_rect->MaxX = max_width;
		dest_rect->MaxY = max_height;
	}

	// Center button within display
	dest_rect->MinX = display_area.Left + ((display_area.Width - dest_rect->MaxX) >> 1);
	dest_rect->MinY = display_area.Top + ((display_area.Height - dest_rect->MaxY) >> 1);

	// Fix button bottom-right corner
	dest_rect->MaxX += dest_rect->MinX - 1;
	dest_rect->MaxY += dest_rect->MinY - 1;

	// Clear the display area around the gadget
	if (display_box)
	{
		struct Rectangle rect;

		// Get object area
		rect.MinX = display_box->Left;
		rect.MinY = display_box->Top;
		rect.MaxX = rect.MinX + display_box->Width - 1;
		rect.MaxY = rect.MinY + display_box->Height - 1;

		// Clear the whole lot?
		if (nodraw)
		{
			if (rect.MinX <= rect.MaxX && rect.MinY <= rect.MaxY)
			{
				EraseRect(window->RPort, rect.MinX, rect.MinY, rect.MaxX, rect.MaxY);
			}
		}

		// Clear sides
		else
		{
			// Clear to the left?
			if (rect.MinX < dest_rect->MinX)
				EraseRect(window->RPort, rect.MinX, rect.MinY, dest_rect->MinX - 1, rect.MaxY);

			// Clear to right?
			if (dest_rect->MaxX < rect.MaxX)
				EraseRect(window->RPort, dest_rect->MaxX + 1, rect.MinY, rect.MaxX, rect.MaxY);

			// Clear to top?
			if (rect.MinY < dest_rect->MinY)
				EraseRect(window->RPort, rect.MinX, rect.MinY, rect.MaxX, dest_rect->MinY - 1);

			// Clear to bottom?
			if (dest_rect->MaxY < rect.MaxY)
				EraseRect(window->RPort, rect.MinX, dest_rect->MaxY + 1, rect.MaxX, rect.MaxY);
		}

		/*
				// Set pen
				SetAPen(window->RPort,0);

				// Clear the whole lot?
				if (nodraw)
				{
					if (rect.MinX<=rect.MaxX &&
						rect.MinY<=rect.MaxY)
					{
						RectFill(window->RPort,
							rect.MinX,rect.MinY,
							rect.MaxX,rect.MaxY);
					}
				}

				// Clear sides
				else
				{
					// Clear to the left?
					if (rect.MinX<dest_rect->MinX)
						RectFill(window->RPort,
							rect.MinX,rect.MinY,
							dest_rect->MinX-1,rect.MaxY);

					// Clear to right?
					if (dest_rect->MaxX<rect.MaxX)
						RectFill(window->RPort,
							dest_rect->MaxX+1,rect.MinY,
							rect.MaxX,rect.MaxY);

					// Clear to top?
					if (rect.MinY<dest_rect->MinY)
						RectFill(window->RPort,
							rect.MinX,rect.MinY,
							rect.MaxX,dest_rect->MinY-1);

					// Clear to bottom?
					if (dest_rect->MaxY<rect.MaxY)
						RectFill(window->RPort,
							rect.MinX,dest_rect->MaxY+1,
							rect.MaxX,rect.MaxY);
				}
		*/
	}

	// Get pens
	fpen = button->button.fpen;
	if (fpen >= 4 && fpen < 252)
		fpen = pen_array[fpen - 4];
	bpen = button->button.bpen;
	if (bpen >= 4 && bpen < 252)
		bpen = pen_array[bpen - 4];

	// Is button valid?
	if (button && !nodraw)
	{
		// Get function
		if (!func)
			func = (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, which);

		// Fill button background
		if (!(button->button.flags & BUTNF_GRAPHIC) || !func || !func->image || flags & DRAW_MASK)
		{
			SetAPen(window->RPort, bpen);
			RectFill(window->RPort, dest_rect->MinX + 1, dest_rect->MinY + 1, dest_rect->MaxX - 1, dest_rect->MaxY - 1);
		}

		// Draw button imagery, depending on button type
		if (button->button.flags & BUTNF_GRAPHIC && func && func->image)
		{
			struct TagItem draw_tags[4];

			// Image tags
			draw_tags[0].ti_Tag = IM_Rectangle;
			draw_tags[0].ti_Data = (ULONG)dest_rect;
			if (flags & DRAW_MASK)
			{
				draw_tags[1].ti_Tag = IM_Mask;
				draw_tags[1].ti_Data = 1;
			}
			else
				draw_tags[1].ti_Tag = TAG_IGNORE;
			draw_tags[2].ti_Tag = IM_Erase;
			draw_tags[2].ti_Data = (flags & DRAW_MASK) ? bpen : 0;
			draw_tags[3].ti_Tag = TAG_DONE;

			// Draw the image
			RenderImage(window->RPort, func->image, 0, 0, draw_tags);
		}

		// Text button?
		else if (!(button->button.flags & BUTNF_GRAPHIC) && func && func->label)
		{
			char *text;

			// Text button; get text pointer
			if ((text = func->label) && text[0])
			{
				struct TextExtent extent;
				short len;

				// Get string length
				if (flags & DRAW_FIRST)
					len = 1;
				else
					len = strlen(text);

				// Calculate text extent
				len = TextFit(window->RPort,
							  text,
							  len,
							  &extent,
							  0,
							  1,
							  dest_rect->MaxX - dest_rect->MinX - 1,
							  window->RPort->TxHeight);

				// Display text centered in button
				SetAPen(window->RPort, fpen);
				SetBPen(window->RPort, bpen);
				Move(window->RPort,
					 dest_rect->MinX + (((dest_rect->MaxX - dest_rect->MinX) + 1 - extent.te_Width) >> 1),
					 dest_rect->MinY + window->RPort->Font->tf_Baseline + 2);
				Text(window->RPort, text, len);
			}
		}
	}

	// Draw button border
	if (!nodraw)
	{
		DrawBox(window->RPort, dest_rect, DRAWINFO(window), FALSE);
	}
}

// Fix button controls
void _config_buttons_fix_controls(config_buttons_data *data)
{
	BOOL selected = 1;

	// Anything selected?
	if (data->select_col == -1 || data->select_row == -1)
		selected = 0;

	// Enable/disable button controls
	DisableObject(data->objlist, GAD_BUTTONS_EDIT, !selected);
	DisableObject(data->objlist, GAD_BUTTONS_COPY, !selected);
	DisableObject(data->objlist, GAD_BUTTONS_CUT, !selected);
	DisableObject(data->objlist, GAD_BUTTONS_ERASE, !selected);

	// Enable/disable row/column controls
	DisableObject(data->objlist, GAD_BUTTONS_COLUMNS_INSERT, !selected);
	DisableObject(data->objlist, GAD_BUTTONS_COLUMNS_DELETE, !(selected && data->bank_node->bank->window.columns > 1));
	DisableObject(data->objlist, GAD_BUTTONS_ROWS_INSERT, !(selected && !data->bank_node->toolbar));
	DisableObject(data->objlist,
				  GAD_BUTTONS_ROWS_DELETE,
				  !(selected && !data->bank_node->toolbar && data->bank_node->bank->window.rows > 1));
}

// Drop a button from the clipboard on something
void _config_buttons_drop_button(config_buttons_data *data, short x, short y)
{
	struct Layer *layer;
	struct Window *window;
	IPCData *ipc = 0;
	ULONG id = 0;
	BOOL ok = 0;

	// Lock layer
	LockLayerInfo(&data->clip_window->WScreen->LayerInfo);

	// Get screen-relative coordinates
	x += data->clip_window->LeftEdge;
	y += data->clip_window->TopEdge;

	// Find which layer we dropped it on
	if ((layer = WhichLayer(&data->clip_window->WScreen->LayerInfo, x, y)))
	{
		// Does layer have a window?
		if ((window = layer->Window))
		{
			// Get window ID
			id = GetWindowID(window);

			// Valid window (not ours)?
			if (window != data->clip_window && window != data->window &&
				(id == WINDOW_BUTTON_CONFIG || id == WINDOW_BUTTONS || id == WINDOW_FUNCTION_EDITOR))
			{
				// Forbid to get app ipc
				Forbid();
				if (!(ipc = (IPCData *)GetWindowAppPort(window)))
					Permit();
			}
			else if (window == data->clip_window)
				ok = 1;
		}
	}

	// Unlock layer
	UnlockLayerInfo(&data->clip_window->WScreen->LayerInfo);

	// Do we have a valid IPC?
	if (ipc)
	{
		// Button bank?
		if (id == WINDOW_BUTTONS)
		{
			Cfg_ButtonBank *bank;
			Point pos;

			// Get bank pointer (this will break forbid)
			bank = (Cfg_ButtonBank *)IPC_Command(ipc, BUTTONEDIT_GIMME_BANK, 0, &pos, 0, REPLY_NO_PORT);

			// Start editing that bank
			_config_buttons_new_bank(data, bank, ipc);

			// Convert coordinates to window-relative
			x -= pos.x;
			y -= pos.y;

			// Copy button to that bank
			_button_to_bank(data, data->last_clip_draw, x, y);
			ok = 1;
		}

		// Otherwise, an editor
		else
		{
			Cfg_Button *button;
			Point *pos;

			// Allocate position
			if ((pos = AllocVec(sizeof(Point), 0)))
			{
				// Store screen coordinates
				pos->x = x;
				pos->y = y;
			}

			// Copy button
			if ((button = CopyButton(data->last_clip_draw, 0, -1)))
			{
				// Send button
				IPC_Command(ipc, BUTTONEDIT_CLIP_BUTTON, 0, button, pos, 0);
				ok = 1;
			}

			// Permit now message is sent
			Permit();
		}
	}

	// Flash screen if we didn't send anything
	if (!ok)
		DisplayBeep(data->clip_window->WScreen);
}

// Fake a button return
void _config_buttons_fake_return(config_buttons_data *data, Cfg_Button *orig_button, Cfg_Button *new_button)
{
	ButtonReturn fake;
	edit_node *node;

	// Fake a button return
	fake.bank = data->bank_node;
	fake.button = new_button;
	fake.orig_button = orig_button;

	// Fake button return
	_config_buttons_receive_edit(data, &fake, 1);

	// See if an editor was open for that button
	for (node = (edit_node *)data->edit_list.lh_Head; node->node.ln_Succ; node = (edit_node *)node->node.ln_Succ)
	{
		// Was it an editor for the button?
		if (node->button == orig_button)
		{
			// Tell editor to quit
			IPC_Quit(node->editor, 0, 0);
			break;
		}
	}
}

// Copy a button to the current bank
void _button_to_bank(config_buttons_data *data, Cfg_Button *button, short x, short y)
{
	Cfg_Button *replace;

	// Get replacement button
	if ((replace = (Cfg_Button *)IPC_Command(
			 data->bank_node->button_ipc, BUTTONEDIT_GET_BUTTON_POINT, (ULONG)&x, (APTR)&y, 0, REPLY_NO_PORT)))
	{
		// Fake button return
		_config_buttons_fake_return(data, replace, button);
	}
}

// Initialise a button editor startup
void button_editor_init(ButtonEdStartup *startup)
{
	// Initialise function editor
	function_editor_init(&startup->func_startup, startup->command_list);

	// Supply data pointers
	startup->win_text_def = &_button_editor_window;
	startup->obj_def = _button_editor_objects;
	startup->obj_name = _button_editor_label_objects;
	startup->obj_image = _button_editor_image_objects;

	// Initialise palette editor
	palette_editor_init(&startup->palette_data);
	startup->palette_data.main_ipc = startup->main_owner;
}

// Initialise palette editor startup
void palette_editor_init(PaletteBoxData *data)
{
	short a, base;

	// Fill out palette box data
	data->newwin.dims = &_palette_box_window;
	data->newwin.title = GetString(locale, MSG_SELECT_COLOURS);
	data->newwin.locale = locale;
	data->newwin.flags = WINDOW_VISITOR | WINDOW_REQ_FILL | WINDOW_AUTO_KEYS;

	// Palette box tags
	data->stuff.stuff1.palette_tags[0].ti_Tag = GTPA_NumColors;
	data->stuff.stuff1.palette_tags[0].ti_Data =
		data->screen_data.pen_count + ((((struct Library *)GfxBase)->lib_Version >= 39) ? 8 : 4);
	data->stuff.stuff1.palette_tags[1].ti_Tag = GTPA_ColorTable;
	data->stuff.stuff1.palette_tags[1].ti_Data = (ULONG)data->pen_array;
	data->stuff.stuff1.palette_tags[2].ti_Tag = GTCustom_LayoutRel;
	data->stuff.stuff1.palette_tags[2].ti_Data = GAD_PALETTE_LAYOUT;
	data->stuff.stuff1.palette_tags[3].ti_Tag = TAG_END;

	// Copy palette aray
	for (base = 0; base < 4; base++)
		data->pen_array[base] = base;
	if (((struct Library *)GfxBase)->lib_Version >= 39)
	{
		for (a = 0; a < 4; a++, base++)
			data->pen_array[base] = 252 + a;
	}
	for (a = 0; a < data->screen_data.pen_count; a++, base++)
		data->pen_array[base] = data->screen_data.pen_array[a];

	// Palette box objects
	CopyMem((char *)_palette_box_objects, (char *)data->stuff.stuff1.object_def, sizeof(data->stuff.stuff1.object_def));
	data->stuff.stuff1.object_def[1].taglist = data->stuff.stuff1.palette_tags;
	data->stuff.stuff1.object_def[2].taglist = data->stuff.stuff1.palette_tags;

	// Libraries
	/*data->DOpusBase=DOpusBase;
	data->IntuitionBase=(struct Library *)IntuitionBase;*/
	data->flags = 0;
}

// Initialise function editor startup
void function_editor_init(FunctionStartup *startup, ULONG command_list)
{
	// Supply libraries
	/*startup->dopus_base=DOpusBase;
	startup->dos_base=(struct Library *)DOSBase;
	startup->int_base=(struct Library *)IntuitionBase;
	startup->util_base=UtilityBase;
	startup->cx_base=CxBase;
	startup->wb_base=WorkbenchBase;
	startup->gfx_base=(struct Library *)GfxBase;
	startup->asl_base=AslBase;
	startup->layers_base=LayersBase;*/

	// Supply locale
	startup->locale = locale;

	// Supply data pointers for function editor
	startup->win_def = &_function_editor_window;
	startup->obj_def = _function_editor_objects;
	startup->func_labels = _function_type_labels;
	startup->flag_list = _funced_flaglist;
	startup->func_list = command_list;
}

// Receive AppMessage dropped onto button bank
void button_receive_appmsg(config_buttons_data *data, struct AppMessage *msg)
{
	Cfg_Button *button;
	Cfg_Instruction *ins;
	struct WBArg *arg;

	// Get first argument
	arg = msg->am_ArgList;

	// Valid argument?
	if (msg->am_NumArgs == 0 || !arg)
		return;

	// Create instruction
	if (!(ins = instruction_from_wbarg(arg, data->bank_node->bank->memory)))
		return;

	// Create button
	if (!(button = button_create_drop(data,
									  data->bank_node->bank->memory,
									  arg,
									  ins,
									  data->bank_node->bank->window.flags,
									  (struct Library *)DOSBase,
									  DOpusBase)))
	{
		// Failed
		FreeInstruction(ins);
		return;
	}

	// Send button to the bank (check this frees in event of failure)
	_button_to_bank(data, button, msg->am_MouseX, msg->am_MouseY);
}

// Create button from drop
Cfg_Button *button_create_drop(config_buttons_data *data,
							   APTR memory,
							   struct WBArg *arg,
							   Cfg_Instruction *ins,
							   unsigned long flags,
							   struct Library *DOSBase,
							   struct Library *DOpusBase)
{
	Cfg_Button *button;
	Cfg_ButtonFunction *func;
	char *label, namebuf[40], buf[256], *name = 0;
	short type;

	// Get argument name and type
	if ((type = funced_appmsg_arg(arg, buf, DOSBase)) == -1)
		return 0;

	// Get name
	strcpy(namebuf, FilePart(buf));

	// Graphical button?
	if (flags & BTNWF_GFX)
	{
		// Add .info suffix
		if (stricmp(buf + strlen(buf) - 5, ".info") != 0)
			strcat(buf, ".info");
		label = buf;

		// Use separate name
		name = namebuf;
	}

	// Text button
	else
		label = namebuf;

	// Create button
	if (!(button = NewButtonWithFunc(memory, label, FTYPE_LEFT_BUTTON)))
		return 0;

	// Get first function
	func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;

	// Separate name?
	if (name)
	{
		// Free existing name
		FreeMemH(func->node.ln_Name);

		// Copy new name
		if ((func->node.ln_Name = AllocMemH(memory, strlen(name) + 1)))
			strcpy(func->node.ln_Name, name);

		// Want an image?
		if (flags & BTNWF_GFX && data)
		{
			// Free existing image
			CloseImage(func->image);

			// Try to open image
			if ((func->image = OpenImage(func->label, 0)))
			{
				// Remap image
				RemapImage(func->image, data->window->WScreen, &data->remap);
			}
		}
	}

	// Add instruction to function
	AddTail((struct List *)&func->instructions, (struct Node *)ins);
	return button;
}
