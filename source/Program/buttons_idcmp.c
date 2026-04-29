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

#define POPUP_TIME 800000

// Process button IDCMP messages
int buttons_process_msg(Buttons *buttons, struct IntuiMessage *msg)
{
	struct Gadget *gadget;

	// Get pointer to gadget (if applicable)
	gadget = (struct Gadget *)msg->IAddress;

	// Look at message class
	switch (msg->Class)
	{
	// Window size changed/Refresh
	case IDCMP_CHANGEWINDOW:
	case IDCMP_REFRESHWINDOW:
		buttons_refresh_callback(msg->Class, buttons->window, buttons);
		break;

	// Window closed
	case IDCMP_CLOSEWINDOW:

		// Close button bank
		if (buttons_check_change(buttons, 1))
			return 0;
		break;

	// Menu pick
	case IDCMP_MENUPICK:
	case IDCMP_MENUHELP: {
		struct MenuItem *item;
		struct Menu *oldstrip = buttons->window->MenuStrip;
		UWORD nextselect;

		// Get item
		nextselect = msg->Code;
		while ((item = ItemAddress(buttons->window->MenuStrip, nextselect)))
		{
			int a;

			// Get next selection
			nextselect = item->NextSelect;

			// Help?
			if (msg->Class == IDCMP_MENUHELP)
			{
				help_menu_help((long)GTMENUITEM_USERDATA(item), 0);
				break;
			}

			// See if we can handle it
			if ((a = buttons_do_function(buttons, (ULONG)(IPTR)GTMENUITEM_USERDATA(item))) == -1)
			{
				MenuEvent *event;

				// Allocate event
				if ((event = AllocVec(sizeof(MenuEvent), 0)))
				{
					// Fill out event
					event->id = (IPTR)GTMENUITEM_USERDATA(item);
					event->menu = item;
					event->window = buttons->window;

					// Send command to main process
					IPC_Command(&main_ipc, MAIN_MENU_EVENT, 0, 0, event, 0);
				}
			}

			// Did buttons close?
			else if (!a)
				return 0;

			// Check for valid next
			if (!nextselect || !buttons->window || buttons->window->MenuStrip != oldstrip)
				break;
		}
	}
	break;

	// Tick
	case IDCMP_INTUITICKS:

		// Increment tick count
		++buttons->tick_count;

		// No gadget down?
		if (!buttons->down_gadget || !(buttons->down_gadget->Flags & GFLG_SELECTED))
			break;

		// Skip this scroll event?
		if (buttons->flags & BUTTONF_SKIP_SCROLL)
		{
			buttons->flags &= ~BUTTONF_SKIP_SCROLL;
			break;
		}
		msg->IAddress = buttons->down_gadget;

	// Gadget down
	case IDCMP_GADGETDOWN:

		// Store gadget
		buttons->down_gadget = (struct Gadget *)msg->IAddress;
		buttons->flags |= BUTTONF_SKIP_SCROLL;

		// Look at gadget ID
		switch (buttons->down_gadget->GadgetID)
		{
		// Drag gadget
		case DRAG_GADGET:

			// Test for double-click
			if (DoubleClick(buttons->drag_click_secs, buttons->drag_click_micros, msg->Seconds, msg->Micros))
			{
				// Bring window to front
				WindowToFront(buttons->window);
				buttons->drag_click_secs = 0;
				break;
			}

			// Store double-click time
			buttons->drag_click_secs = msg->Seconds;
			buttons->drag_click_micros = msg->Micros;
			buttons->down_gadget = 0;
			break;

		// Up arrow
		case GAD_VERT_ARROW_UP:
			if (buttons->button_top > 0)
			{
				SetGadgetAttrs(buttons->vert_scroll, buttons->window, 0, PGA_Top, buttons->button_top - 1, TAG_END);
				buttons_refresh(buttons, BUTREFRESH_REFRESH);
			}
			break;

		// Down arrow
		case GAD_VERT_ARROW_DOWN:
			if (buttons->button_top < buttons->bank->window.rows - buttons->button_rows)
			{
				SetGadgetAttrs(buttons->vert_scroll, buttons->window, 0, PGA_Top, buttons->button_top + 1, TAG_END);
				buttons_refresh(buttons, BUTREFRESH_REFRESH);
			}
			break;

		// Left arrow
		case GAD_HORIZ_ARROW_LEFT:
			if (buttons->button_left > 0)
			{
				SetGadgetAttrs(buttons->horiz_scroll, buttons->window, 0, PGA_Top, buttons->button_left - 1, TAG_END);
				buttons_refresh(buttons, BUTREFRESH_REFRESH);
			}
			break;

		// Right arrow
		case GAD_HORIZ_ARROW_RIGHT:
			if (buttons->button_left < buttons->bank->window.columns - buttons->button_cols)
			{
				SetGadgetAttrs(buttons->horiz_scroll, buttons->window, 0, PGA_Top, buttons->button_left + 1, TAG_END);
				buttons_refresh(buttons, BUTREFRESH_REFRESH);
			}
			break;

		default:
			buttons->down_gadget = 0;
			break;
		}
		break;

	// Gadget up
	case IDCMP_GADGETUP:
		buttons->down_gadget = 0;

		// Iconify?
		if (gadget->GadgetID == GAD_ICONIFY)
		{
			buttons_do_function(buttons, MENU_ICONIFY_BUTTONS);
		}
		break;

	// BOOPSI gadget
	case IDCMP_IDCMPUPDATE:

		switch (GetTagData(GA_ID, 0, (struct TagItem *)gadget))
		{
		// Vertical slider
		case GAD_VERT_SCROLLER:

			// Refresh button display
			buttons_refresh(buttons, BUTREFRESH_REFRESH);
			break;

		// Horizontal slider
		case GAD_HORIZ_SCROLLER:

			// Refresh button display
			buttons_refresh(buttons, BUTREFRESH_REFRESH);
			break;
		}
		break;

	// (In)active window does mousemove (& button up)
	case IDCMP_INACTIVEWINDOW:
		msg->Qualifier &= ~(IEQUALIFIER_LEFTBUTTON | IEQUALIFIER_RBUTTON | IEQUALIFIER_MIDBUTTON);
		buttons->down_gadget = 0;
	case IDCMP_ACTIVEWINDOW:

		// Tell editor to edit us if it's running
		if (msg->Class == IDCMP_ACTIVEWINDOW && GUI->flags & GUIF_BUTTON_EDITOR && !buttons->editor)
		{
			buttons_edit_bank(buttons, -1, -1, 0, 0, FALSE);
		}

	// Mouse move
	case IDCMP_MOUSEMOVE:

		// Dragging a button?
		if (buttons->drag_info)
		{
			buttons_show_drag(buttons, buttons->window->MouseX, buttons->window->MouseY);
			break;
		}

		// No valid button down?
		if (!buttons->button_sel_button)
			break;

		// Check appropriate mouse button is still down
		if ((buttons->button_sel_code == SELECTDOWN && msg->Qualifier & IEQUALIFIER_LEFTBUTTON) ||
			(buttons->button_sel_code == MENUDOWN && msg->Qualifier & IEQUALIFIER_RBUTTON) ||
			(buttons->button_sel_code == MIDDLEDOWN && msg->Qualifier & IEQUALIFIER_MIDBUTTON))
		{
			short x, y;

			// Get current mouse coordinates
			x = buttons->window->MouseX;
			y = buttons->window->MouseY;

			// See if mouse is over button
			if (x >= buttons->button_sel_rect.MinX && x <= buttons->button_sel_rect.MaxX &&
				y >= buttons->button_sel_rect.MinY && y <= buttons->button_sel_rect.MaxY)
			{
				// If button isn't highlighted, select it
				if (!buttons->button_sel_flag)
				{
					buttons_highlight_button(buttons, 1, 0);
					buttons->button_sel_flag = 1;
				}
			}

			// Not over button; deselect button if necessary
			else if (buttons->button_sel_flag)
			{
				buttons_highlight_button(buttons, 0, 0);
				buttons->button_sel_flag = 0;

				// If editor is up, start drag
				if (buttons->editor)
				{
					// Start drag
					buttons_start_drag(buttons, x, y, buttons->button_sel_col, buttons->button_sel_row);
				}
			}
			break;
		}

		// Mouse button must not be down; fall through to fake button up
		if (buttons->button_sel_code == SELECTDOWN)
			msg->Code = SELECTUP;
		else
			msg->Code = MENUUP;

	// Mouse button
	case IDCMP_MOUSEBUTTONS:

		// Clear down gadget
		buttons->down_gadget = 0;

		// Button down?
		if (msg->Code == SELECTDOWN || msg->Code == MENUDOWN || msg->Code == MIDDLEDOWN)
		{
			// Are we dragging a button?
			if (buttons->drag_info)
			{
				// Other mouse button does abort
				if (msg->Code != buttons->button_sel_code)
				{
					buttons_stop_drag(buttons, -1, -1);
					break;
				}
			}

			// Not already a button down?
			if (!buttons->button_sel_button)
			{
				short col, row;
				Cfg_Button *button;

				// See if click is outside button area, or control is held-down
				if (msg->MouseX < buttons->internal.Left || msg->MouseY < buttons->internal.Top ||
					msg->MouseX >= buttons->internal.Left + buttons->internal.Width ||
					msg->MouseY >= buttons->internal.Top + buttons->internal.Height ||
					msg->Qualifier & IEQUALIFIER_CONTROL)
				{
					// Right button does pop-up menu
					if (msg->Code == MENUDOWN)
					{
						UWORD res;

						// Do pop-up menu
						res = button_border_popup(buttons);

						// Do function
						if (res != (UWORD)-1)
						{
							// Help?
							if (res & POPUP_HELPFLAG)
								help_menu_help(res & ~POPUP_HELPFLAG, 0);

							// Edit buttons?
							else if (res == MENU_EDIT_BUTTONS)
								buttons_edit_bank(buttons, -1, -1, 0, 0, TRUE);

							// Do function
							else
							{
								buttons_refresh(buttons, BUTREFRESH_REFRESH);
								if (!(buttons_do_function(buttons, res)))
									return 0;
							}
						}
					}

					// Left button toggles border
					else if (msg->Code == SELECTDOWN)
					{
						// Toggle border state
						if (buttons->border_type & BTNWF_NONE)
						{
							buttons->bank->window.flags &= ~(BTNWF_HORIZ | BTNWF_VERT | BTNWF_NONE | BTNWF_RIGHT_BELOW);
							if (buttons->old_flags != BTNWF_NONE)
								buttons->bank->window.flags |= buttons->old_flags;
						}
						else
						{
							buttons->old_flags = buttons->bank->window.flags &
												 (BTNWF_HORIZ | BTNWF_VERT | BTNWF_NONE | BTNWF_RIGHT_BELOW);
							buttons->bank->window.flags &= ~(BTNWF_HORIZ | BTNWF_VERT | BTNWF_NONE | BTNWF_RIGHT_BELOW);
							buttons->bank->window.flags |= BTNWF_NONE;
						}
						buttons_refresh(buttons, BUTREFRESH_RESIZE);
					}

					break;
				}

				// Store coordinates
				buttons->drag_x_offset = msg->MouseX;
				buttons->drag_y_offset = msg->MouseY;

				// Lock bank
				GetSemaphore(&buttons->bank->lock, SEMF_SHARED, 0);

				// See what button we pressed on
				col = msg->MouseX;
				row = msg->MouseY;
				if (!(button = button_from_point(buttons, &col, &row)))
				{
					FreeSemaphore(&buttons->bank->lock);
					break;
				}

				// If alt is down, edit button
				if (msg->Qualifier & IEQUAL_ANYALT && !(msg->Qualifier & IEQUALIFIER_LCOMMAND))
				{
					// Edit it
					buttons_edit_bank(buttons, col + buttons->button_left, row + buttons->button_top, 0, 0, TRUE);

					// Break out
					FreeSemaphore(&buttons->bank->lock);
					break;
				}

				// Get button type
				buttons->button_sel_which =
					(msg->Code == MIDDLEDOWN || (msg->Code == SELECTDOWN && (msg->Qualifier & IEQUAL_ANYSHIFT)))
						? FTYPE_MID_BUTTON
						: ((msg->Code == MENUDOWN) ? FTYPE_RIGHT_BUTTON : button->current);

				// Left button down?
				if (msg->Code == SELECTDOWN && !(msg->Qualifier & IEQUAL_ANYSHIFT))
				{
					// If editor isn't up, start timer for popup menu
					if (!buttons->editor)
					{
						// Start timer
						StartTimer(buttons->timer, 0, POPUP_TIME);

						// Set flag
						buttons->flags |= BUTTONF_BUTTON_HELD;

						// Save position
						buttons->click_x = msg->MouseX;
						buttons->click_y = msg->MouseY;
					}
				}

				// Store button information
				buttons->button_sel_button = button;
				buttons->button_sel_code = msg->Code;
				buttons->button_sel_col = col;
				buttons->button_sel_row = row;

				// Is button valid?
				if (!(button_valid(button, buttons->button_sel_which)) && !buttons->editor)
				{
					// No
					FreeSemaphore(&buttons->bank->lock);

					// Do popup if left button
					if (buttons->flags & BUTTONF_BUTTON_HELD)
					{
						// Do popup
						buttons_do_popup(buttons, SELECTDOWN);

						// Clear flag
						buttons->flags &= ~BUTTONF_BUTTON_HELD;
					}

					// Clear button pointer
					buttons->button_sel_button = 0;
					break;
				}

				// Store button rectangle
				buttons_get_rect(buttons, col, row, &buttons->button_sel_rect);

				// Stop highlight flashing
				buttons_stop_highlight(buttons);

				// Highlight button
				buttons_highlight_button(buttons, 1, 0);
				buttons->button_sel_flag = 1;

				// Unlock bank
				FreeSemaphore(&buttons->bank->lock);
			}

			// Or, left button down and we pressed right
			else if (buttons->button_sel_code == SELECTDOWN && msg->Code == MENUDOWN)
			{
				// Is the button selected?
				if (buttons->button_sel_flag)
				{
					// Do popup button list
					buttons_do_popup(buttons, MENUDOWN);
				}
			}
		}

		// Button released
		else if ((msg->Code == SELECTUP && buttons->button_sel_code == SELECTDOWN) ||
				 (msg->Code == MENUUP && buttons->button_sel_code == MENUDOWN) ||
				 (msg->Code == MIDDLEUP && buttons->button_sel_code == MIDDLEDOWN))
		{
			Cfg_Button *click_button = 0;

			// Clear held flag
			buttons->flags &= ~BUTTONF_BUTTON_HELD;

			// Dragging a button?
			if (buttons->drag_info)
			{
				buttons_stop_drag(buttons, msg->MouseX, msg->MouseY);
				break;
			}

			// Valid button?
			if (buttons->button_sel_button)
			{
				short which;

				// Which button pressed?
				which = buttons->button_sel_which;

				// Redraw border if necessary
				if (buttons->button_sel_which != buttons->button_sel_button->current || buttons->button_sel_flag)
				{
					buttons->button_sel_which = buttons->button_sel_button->current;
					buttons_highlight_button(buttons, 0, 1);
				}
				else
					buttons_highlight_button(buttons, 0, -1);

				// Was button selected?
				if (buttons->button_sel_flag)
				{
					buttons->button_sel_flag = 0;
					click_button = buttons->button_sel_button;
				}

				// Clear button pointer
				buttons->button_sel_button = 0;
				buttons->button_sel_code = 0;

				// If editor is up, select this row/column
				if (buttons->editor)
				{
					// Was button clicked?
					if (click_button)
					{
						// Store column and row
						buttons->editor_sel_col = buttons->button_sel_col + buttons->button_left;
						buttons->editor_sel_row = buttons->button_sel_row + buttons->button_top;

						// Enable flashing
						buttons->flags &= ~BUTTONF_NO_FLASH;

						// Select button
						IPC_Command(buttons->editor,
									BUTTONEDIT_SELECT_BUTTON,
									buttons->editor_sel_col,
									(APTR)buttons->editor_sel_row,
									0,
									0);

						// Test double-click
						if (click_button == buttons->button_last_click &&
							(DoubleClick(buttons->old_seconds, buttons->old_micros, msg->Seconds, msg->Micros)))
						{
							// Send edit command
							IPC_Command(buttons->editor,
										BUTTONEDIT_EDIT_BUTTON,
										buttons->editor_sel_col,
										(APTR)buttons->editor_sel_row,
										0,
										0);

							// Clear last click
							buttons->button_last_click = 0;

							// Stop highlight flashing
							buttons_stop_highlight(buttons);
						}

						// Store double click time
						else
						{
							buttons->old_seconds = msg->Seconds;
							buttons->old_micros = msg->Micros;
							buttons->button_last_click = click_button;
						}
					}

					// No selected button now
					else
					{
						buttons->editor_sel_col = -1;
						buttons->editor_sel_row = -1;
						buttons->button_sel_button = 0;
						IPC_Command(buttons->editor, BUTTONEDIT_SELECT_BUTTON, -1, (APTR)-1, 0, 0);
					}
				}

				// Otherwise run function
				else if (!(buttons->flags & BUTTONF_EDIT_REQUEST))
				{
					// Run function
					buttons_run_button(buttons, click_button, which);

					// If auto close flag is set, close bank
					if (buttons->bank->window.flags & BTNWF_AUTO_CLOSE)
					{
						if (buttons_check_change(buttons, 1))
							return 0;
					}
					else if (buttons->bank->window.flags & BTNWF_AUTO_ICONIFY)
						buttons_do_function(buttons, MENU_ICONIFY_BUTTONS);
				}
			}
		}
		break;

	// Key press
	case IDCMP_RAWKEY:

		// Help?
		if (msg->Code == 0x5f && !(msg->Qualifier & VALID_QUALIFIERS))
		{
			help_get_help(
				msg->MouseX + buttons->window->LeftEdge, msg->MouseY + buttons->window->TopEdge, msg->Qualifier);
			break;
		}

		// Is editor up?
		if (buttons->editor)
		{
			buttons_edit_key(buttons, msg->Code, msg->Qualifier);
			break;
		}

		// Otherwise
		else
		{
			Cfg_Function *function;

			// Can we match a function key?
			if ((function = match_function_key(msg->Code, msg->Qualifier, buttons->bank, 0, WINDOW_BUTTONS, 0)))
			{
				// Run function
				function_launch(FUNCTION_RUN_FUNCTION, function, 0, 0, 0, 0, 0, 0, 0, 0, buttons);
			}
		}
		break;
	}

	return 1;
}
