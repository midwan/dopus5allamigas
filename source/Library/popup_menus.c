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

#include "dopuslib.h"
#include "boopsi.h"
#include "popup.h"

#define SHADOW
#define SHADOW_X 4
#define SHADOW_Y 4

// Do a pop-up menu
UWORD LIBFUNC L_DoPopUpMenu(REG(a0, struct Window *window),
							REG(a1, PopUpMenu *menu),
							REG(a2, PopUpItem **sel_item),
							REG(d0, UWORD code),
							REG(a6, struct MyLibrary *libbase))
{
	PopUpData *data;
	PopUpItem *item;
	unsigned short qual, bullet, sub;
	BOOL first_move = 1;
	short first_x, first_y, still_ticks;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = dopuslibbase_global;
#endif

	// Clear selection item
	if (sel_item)
		*sel_item = 0;

	// Check valid pointers
	if (!window || !menu)
		return (UWORD)-1;

	// Code we need to end the menu
	if (code == SELECTDOWN)
	{
		code = SELECTUP;
		qual = IEQUALIFIER_LEFTBUTTON;
	}
	else if (code == MENUDOWN)
	{
		code = MENUUP;
		qual = IEQUALIFIER_RBUTTON;
	}
	else
		return (UWORD)-1;

	// Allocate data
	if (!(data = AllocVec(sizeof(PopUpData), MEMF_CLEAR)))
		return (UWORD)-1;

	// Get popup delay
	if ((still_ticks = ((struct LibData *)libbase->ml_UserData)->popup_delay) < 1)
		still_ticks = STILL_TICKS;

	// Sticky straight away?
	if (menu->flags & POPUPMF_STICKY)
	{
		// Lock menu
		data->locked = 1;
		code &= ~IECODE_UP_PREFIX;
	}

	// Create our own message port, and a timer
	if (!(data->port = CreateMsgPort()) || !(data->timer = L_AllocTimer(UNIT_VBLANK, 0)))
	{
		DeleteMsgPort(data->port);
		FreeVec(data);
		return (UWORD)-1;
	}

	// Store some pointers, get font we use
	data->parent_window = window;
	data->menu = menu;
	data->font = ((menu->flags & POPUPMF_FONT) && menu->font) ? menu->font : window->WScreen->RastPort.Font;
	data->menu_list[0].item_list = &menu->item_list;

	// Lo-res screen?
	if (L_ScreenInfo(window->WScreen) & SCRI_LORES)
	{
		// Small images?
		if (data->font->tf_YSize <= 13)
		{
			// Bullet
			data->bullet_width = SMALL_LO_BULLET_WIDTH;
			data->bullet_height = SMALL_LO_BULLET_HEIGHT;
			bullet = BULLET_SMALL_LO;

			// Sub
			data->sub_width = SMALL_LO_SUB_WIDTH;
			data->sub_height = SMALL_LO_SUB_HEIGHT;
			sub = SUB_SMALL_LO;
		}

		// Big
		else
		{
			// Bullet
			data->bullet_width = BIG_LO_BULLET_WIDTH;
			data->bullet_height = BIG_LO_BULLET_HEIGHT;
			bullet = BULLET_BIG_LO;

			// Sub
			data->sub_width = BIG_LO_SUB_WIDTH;
			data->sub_height = BIG_LO_SUB_HEIGHT;
			sub = SUB_BIG_LO;
		}
	}

	// Hi-res screen
	else
	{
		// Small images?
		if (data->font->tf_YSize <= 15)
		{
			// Bullet
			data->bullet_width = SMALL_HI_BULLET_WIDTH;
			data->bullet_height = SMALL_HI_BULLET_HEIGHT;
			bullet = BULLET_SMALL_HI;

			// Sub
			data->sub_width = SMALL_HI_SUB_WIDTH;
			data->sub_height = SMALL_HI_SUB_HEIGHT;
			sub = SUB_SMALL_HI;
		}

		// Big
		else
		{
			// Bullet
			data->bullet_width = BIG_HI_BULLET_WIDTH;
			data->bullet_height = BIG_HI_BULLET_HEIGHT;
			bullet = BULLET_BIG_HI;

			// Sub
			data->sub_width = BIG_HI_SUB_WIDTH;
			data->sub_height = BIG_HI_SUB_HEIGHT;
			sub = SUB_BIG_HI;
		}
	}

	// Allocate image data
	if (!(data->bullet_data = AllocVec(
			  (data->bullet_height + data->sub_height + (SCROLLIMAGE_HEIGHT << 1)) * sizeof(UWORD), MEMF_CHIP)))
	{
		popup_cleanup(data);
		return (UWORD)-1;
	}

	// Get sub image pointer
	data->sub_data = data->bullet_data + data->bullet_height;

	// Standard separator height
	data->sep_height = 6;

	// Horizontal padding needed
	data->item_h_pad = data->font->tf_XSize + data->font->tf_YSize + 2;
	if (data->item_h_pad < data->sub_width)
		data->item_h_pad = data->sub_width;

	// Get scroll image pointer
	data->scroll_image[0] = data->sub_data + data->sub_height;
	data->scroll_image[1] = data->scroll_image[0] + SCROLLIMAGE_HEIGHT;

	// Initialise scroll image data
	data->scroll_image[0][0] = 48 << 8;
	data->scroll_image[0][1] = 120 << 8;
	data->scroll_image[0][2] = 252 << 8;
	data->scroll_image[1][0] = 252 << 8;
	data->scroll_image[1][1] = 120 << 8;
	data->scroll_image[1][2] = 48 << 8;

	// Initialise bullet data
	switch (bullet)
	{
	// Small lores
	case BULLET_SMALL_LO:
		data->bullet_data[0] = 112 << 8;
		data->bullet_data[1] = 248 << 8;
		data->bullet_data[2] = 248 << 8;
		data->bullet_data[3] = 112 << 8;
		break;

	// Big lores
	case BULLET_BIG_LO:
		data->bullet_data[0] = 126 << 8;
		data->bullet_data[1] = 255 << 8;
		data->bullet_data[2] = 255 << 8;
		data->bullet_data[3] = 255 << 8;
		data->bullet_data[4] = 126 << 8;
		break;

	// Small hires
	case BULLET_SMALL_HI:
		data->bullet_data[0] = 96 << 8;
		data->bullet_data[1] = 240 << 8;
		data->bullet_data[2] = 240 << 8;
		data->bullet_data[3] = 96 << 8;
		break;

	// Big hires
	case BULLET_BIG_HI:
		data->bullet_data[0] = 112 << 8;
		data->bullet_data[1] = 248 << 8;
		data->bullet_data[2] = 248 << 8;
		data->bullet_data[3] = 248 << 8;
		data->bullet_data[4] = 112 << 8;
		break;
	}

	// Initialise sub data
	switch (sub)
	{
	// Small lores
	case SUB_SMALL_LO:
		data->sub_data[0] = 0xc0 << 8;
		data->sub_data[1] = 0xf0 << 8;
		data->sub_data[2] = 0xfc << 8;
		data->sub_data[3] = 0xf0 << 8;
		data->sub_data[4] = 0xc0 << 8;
		break;

	// Big lores
	case SUB_BIG_LO:
		data->sub_data[0] = 0xc0 << 8;
		data->sub_data[1] = 0xf0 << 8;
		data->sub_data[2] = 0xfc << 8;
		data->sub_data[3] = 0xff << 8;
		data->sub_data[4] = 0xfc << 8;
		data->sub_data[5] = 0xf0 << 8;
		data->sub_data[6] = 0xc0 << 8;
		break;

	// Small hires
	case SUB_SMALL_HI:
		data->sub_data[0] = 0x80 << 8;
		data->sub_data[1] = 0xc0 << 8;
		data->sub_data[2] = 0xe0 << 8;
		data->sub_data[3] = 0xc0 << 8;
		data->sub_data[4] = 0x80 << 8;
		break;

	// Big hires
	case SUB_BIG_HI:
		data->sub_data[0] = 0x80 << 8;
		data->sub_data[1] = 0xc0 << 8;
		data->sub_data[2] = 0xe0 << 8;
		data->sub_data[3] = 0xf0 << 8;
		data->sub_data[4] = 0xf8 << 8;
		data->sub_data[5] = 0xf8 << 8;
		data->sub_data[6] = 0xf0 << 8;
		data->sub_data[7] = 0xe0 << 8;
		data->sub_data[8] = 0xc0 << 8;
		data->sub_data[9] = 0x80 << 8;
		break;
	}

#ifdef __AROS__
	{
		int i, numwords;

		for (i = 0; i < 3; i++)
		{
			data->scroll_image[0][i] = AROS_BE2WORD(data->scroll_image[0][i]);
			data->scroll_image[1][i] = AROS_BE2WORD(data->scroll_image[1][i]);
		}

		if (bullet == BULLET_SMALL_LO || bullet == BULLET_SMALL_HI)
			numwords = 4;
		else
			numwords = 5;

		for (i = 0; i < numwords; i++)
		{
			data->bullet_data[i] = AROS_BE2WORD(data->bullet_data[i]);
		}

		if (sub == SUB_SMALL_LO || bullet == SUB_SMALL_LO)
			numwords = 5;
		else if (sub == SUB_BIG_LO)
			numwords = 7;
		else
			numwords = 10;

		for (i = 0; i < numwords; i++)
		{
			data->sub_data[i] = AROS_BE2WORD(data->sub_data[i]);
		}
	}
#endif

	// Get drawinfo and pens to use
	data->drawinfo = GetScreenDrawInfo(window->WScreen);
	data->fpen = (menu->flags & POPUPMF_USE_PEN) ? POPUPMF_GET_PEN(menu->flags) : data->drawinfo->dri_Pens[TEXTPEN];
	data->bpen = data->drawinfo->dri_Pens[BACKGROUNDPEN];
	data->fpen_sel = (data->menu->flags & POPUPMF_BACKFILL && data->menu->backfill)
						 ? data->drawinfo->dri_Pens[HIGHLIGHTTEXTPEN]
						 : data->drawinfo->dri_Pens[FILLTEXTPEN];
	data->bpen_sel =
		(data->menu->flags & POPUPMF_BACKFILL && data->menu->backfill) ? data->bpen : data->drawinfo->dri_Pens[FILLPEN];

	// Save window's port and flags
	data->old_port = window->UserPort;
	data->old_idcmp = window->IDCMPFlags;
	data->old_flags = window->Flags;

	// Install our own port and set our flags
	Forbid();

	// Go through window's port and steal messages
	if (window->UserPort)
	{
		struct IntuiMessage *msg, *next;

		// Go through messages
		for (msg = (struct IntuiMessage *)window->UserPort->mp_MsgList.lh_Head; msg->ExecMessage.mn_Node.ln_Succ;
			 msg = next)
		{
			// Cache next
			next = (struct IntuiMessage *)msg->ExecMessage.mn_Node.ln_Succ;

			// Match window message
			if (msg->IDCMPWindow == window)
			{
				// Remove and send to our port
				Remove((struct Node *)msg);
				PutMsg(data->port, (struct Message *)msg);
			}
		}
	}

	// Install port, set flags
	window->UserPort = data->port;
	ModifyIDCMP(window,
				IDCMP_CHANGEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_NEWSIZE | IDCMP_INACTIVEWINDOW |
					IDCMP_RAWKEY | IDCMP_REFRESHWINDOW);
	window->Flags |= WFLG_REPORTMOUSE | WFLG_RMBTRAP;
	Permit();

	// Is window active?
	if (window->Flags & WFLG_WINDOWACTIVE)
		data->was_active = 1;

	// Open main list
	if (!(popup_init_list(data, 0)))
	{
		popup_cleanup(data);
		return (UWORD)-1;
	}

	// Start timer
	L_StartTimer(data->timer, 0, TICK_TIME);

	// Get initial wait bits
	data->wait_bits = 1 << data->port->mp_SigBit | 1 << data->timer->port->mp_SigBit;

	// Get initial mouse position
	first_x = window->MouseX;
	first_y = window->MouseY;

	// Event loop
	FOREVER
	{
		struct IntuiMessage *msg;
		short which;

		// Messages from menu window?
		for (which = 0; which < MENU_DEPTH; which++)
		{
			// Window open?
			if (!data->menu_list[which].window)
				continue;

			// Check for messages
			while ((msg = (struct IntuiMessage *)GetMsg(data->menu_list[which].window->UserPort)))
			{
				// Refresh?
				if (msg->Class == IDCMP_REFRESHWINDOW)
				{
					// Refresh it
					BeginRefresh(data->menu_list[which].window);
					popup_init_display(data, which);
					popup_display(data, which);
					EndRefresh(data->menu_list[which].window, TRUE);
				}

				// Mouse button
				else if (msg->Class == IDCMP_MOUSEBUTTONS && msg->Code == SELECTDOWN)
				{
					// Menu locked?
					if (data->locked)
					{
						UWORD ret = (UWORD)-1;
						BOOL ok = 1;

						// Selected item?
						if (data->menu_list[data->menu_current].sel_item &&
							data->menu_list[data->menu_current].sel_item->item_name != POPUP_BARLABEL &&
							!(data->menu_list[data->menu_current].sel_item->flags & POPUPF_DISABLED))
						{
							// Item with sub-items?
							if (data->menu_list[data->menu_current].sel_item->flags & POPUPF_SUB &&
								data->menu_list[data->menu_current].sel_item->data)
							{
								// Set item list pointer
								data->menu_list[data->menu_current + 1].item_list =
									data->menu_list[data->menu_current].sel_item->data;

								// Initialise display
								if (popup_init_list(data, data->menu_current + 1))
								{
									// Set current menu pointer
									++data->menu_current;
								}
								ok = 0;
							}

							// Ending menu
							else
							{
								// Store ID
								ret = data->menu_list[data->menu_current].sel_item->id;

								// Toggle check mark?
								if (data->menu_list[data->menu_current].sel_item->flags & POPUPF_CHECKIT)
									data->menu_list[data->menu_current].sel_item->flags ^= POPUPF_CHECKED;

								// Item to return
								if (sel_item)
									*sel_item = data->menu_list[data->menu_current].sel_item;
							}
						}

						// Cleanup and return
						if (ok)
						{
							ReplyMsg((struct Message *)msg);
							popup_cleanup(data);
							return ret;
						}
					}
				}

				// Reply to message
				ReplyMsg((struct Message *)msg);
			}
		}

		// Timer returned?
		while (L_CheckTimer(data->timer))
		{
			// Restart timer
			L_StartTimer(data->timer, 0, TICK_TIME);

			// Are we scrolling?
			if (data->menu_list[data->menu_current].scroll_dir)
			{
				short height, count;
				char *text;

				// Get top item
				for (item = (PopUpItem *)data->menu_list[data->menu_current].item_list->mlh_Head, count = 0;
					 item->node.mln_Succ && count < data->menu_list[data->menu_current].top_item;
					 item = (PopUpItem *)item->node.mln_Succ, count++)

					// Failure?
					if (!item->node.mln_Succ)
					{
						data->menu_list[data->menu_current].scroll_dir = 0;
						break;
					}

				// Scrolling up?
				if (data->menu_list[data->menu_current].scroll_dir == -1)
				{
					// Already at top?
					if (data->menu_list[data->menu_current].top_item <= 0)
					{
						data->menu_list[data->menu_current].scroll_dir = 0;
						break;
					}

					// Need height of previous item
					item = (PopUpItem *)item->node.mln_Pred;
				}

				// Down
				else
				{
					// Already at bottom?
					if (data->menu_list[data->menu_current].bottom_item >=
						data->menu_list[data->menu_current].item_count)
					{
						data->menu_list[data->menu_current].scroll_dir = 0;
						break;
					}
				}

				// Get scroll height
				if (item->flags & POPUPF_LOCALE)
					text = L_GetString(data->menu->locale, (LONG)item->item_name);
				else
					text = (char *)item->item_name;
				height = ((IPTR)text == POPUP_BARLABEL) ? data->sep_height : data->menu_list[data->menu_current].item_height;

				// Do scroll
				data->menu_list[data->menu_current].top_item += data->menu_list[data->menu_current].scroll_dir;
				ScrollRaster(data->menu_list[data->menu_current].window->RPort,
							 0,
							 data->menu_list[data->menu_current].scroll_dir * height,
							 1,
							 data->menu_list[data->menu_current].item_y,
							 data->menu_list[data->menu_current].pos.Width - 2,
							 data->menu_list[data->menu_current].bottom_y);

				// Show items
				popup_display(data, data->menu_current);
			}

			// Increment tick count
			else if ((++data->tick_count) > still_ticks)
			{
				// Reset tick count
				data->tick_count = 0;

				// Still for the required time; are we over a menu with sub-items?
				if (data->menu_current < MENU_DEPTH - 1 && data->menu_list[data->menu_current].sel_item &&
					data->menu_list[data->menu_current].sel_item->flags & POPUPF_SUB &&
					!(data->menu_list[data->menu_current].sel_item->flags & POPUPF_DISABLED) &&
					data->menu_list[data->menu_current].sel_item->data)
				{
					// Set item list pointer
					data->menu_list[data->menu_current + 1].item_list =
						data->menu_list[data->menu_current].sel_item->data;

					// Initialise display
					if (popup_init_list(data, data->menu_current + 1))
					{
						// Set current menu pointer
						++data->menu_current;
					}
				}
			}
			break;
		}

		// Any messages?
		while ((msg = (struct IntuiMessage *)GetMsg(data->port)))
		{
			struct IntuiMessage msg_copy;

			// Refresh message?
			if (msg->Class == IDCMP_REFRESHWINDOW || msg->Class == IDCMP_NEWSIZE || msg->Class == IDCMP_CHANGEWINDOW)
			{
				// Got callback?
				if (data->menu->flags & POPUPMF_REFRESH && data->menu->callback)
				{
					// Reply
					ReplyMsg((struct Message *)msg);

					// Call callback
					REFCALL(data->menu->callback, msg->Class, window, (APTR)data->menu->userdata);
				}

				// Otherwise, pass it on for later
				else
				{
					// Pass along to old port
					PutMsg(data->old_port, (struct Message *)msg);
				}

				continue;
			}

			// Copy the message and reply
			msg_copy = *msg;
			ReplyMsg((struct Message *)msg);

			// Ignore intuiticks
			if (msg_copy.Class == IDCMP_INTUITICKS)
				continue;

			// If correct button isn't down any longer, force cancel (unless locked)
			if (msg_copy.Class != IDCMP_MOUSEBUTTONS && !(msg_copy.Qualifier & qual) && !data->locked)
			{
				msg_copy.Class = IDCMP_RAWKEY;
				msg_copy.Code = 0x45;
			}

			// Look at message
			switch (msg_copy.Class)
			{
			// Key press
			case IDCMP_RAWKEY:

				// Escape falls through to abort
				if (msg_copy.Code != 0x45)
				{
					// Do we support help?
					if (msg_copy.Code == 0x5f && (data->menu->flags & POPUPMF_HELP))
					{
						// Selected item?
						if (data->menu_list[data->menu_current].sel_item &&
							data->menu_list[data->menu_current].sel_item->item_name != POPUP_BARLABEL)
						{
							UWORD ret;

							// Store ID
							ret = data->menu_list[data->menu_current].sel_item->id;

							// Item to return
							if (sel_item)
								*sel_item = data->menu_list[data->menu_current].sel_item;

							// Set help bit
							ret |= (1 << 15);

							// Cleanup and return
							popup_cleanup(data);
							return ret;
						}
					}
					break;
				}

			// Inactive window does abort
			case IDCMP_INACTIVEWINDOW:
				popup_cleanup(data);
				return (UWORD)-1;

			// Mouse button
			case IDCMP_MOUSEBUTTONS: {
				UWORD ret = (UWORD)-1;

				// Is it the code we want?
				if (msg_copy.Code == code)
				{
					// Selected item?
					if (data->menu_list[data->menu_current].sel_item &&
						data->menu_list[data->menu_current].sel_item->item_name != POPUP_BARLABEL &&
						!(data->menu_list[data->menu_current].sel_item->flags & POPUPF_DISABLED))
					{
						// Item with sub-items?
						if (data->menu_list[data->menu_current].sel_item->flags & POPUPF_SUB &&
							data->menu_list[data->menu_current].sel_item->data)
						{
							// Set item list pointer
							data->menu_list[data->menu_current + 1].item_list =
								data->menu_list[data->menu_current].sel_item->data;

							// Initialise display
							if (popup_init_list(data, data->menu_current + 1))
							{
								// Set current menu pointer
								++data->menu_current;
							}
							break;
						}

						// Store ID
						ret = data->menu_list[data->menu_current].sel_item->id;

						// Toggle check mark?
						if (data->menu_list[data->menu_current].sel_item->flags & POPUPF_CHECKIT)
							data->menu_list[data->menu_current].sel_item->flags ^= POPUPF_CHECKED;

						// Item to return
						if (sel_item)
							*sel_item = data->menu_list[data->menu_current].sel_item;
					}

					// No selection, see if we can go sticky
					else
					{
						short x, y;

						// Convert coordinates to menu-relative
						x = msg_copy.MouseX + data->parent_window->LeftEdge -
							data->menu_list[data->menu_current].pos.Left;
						y = msg_copy.MouseY + data->parent_window->TopEdge -
							data->menu_list[data->menu_current].pos.Top;

						// Are we near the original position, and over the menu?
						if (x >= data->menu_list[data->menu_current].o_x - 5 &&
							x <= data->menu_list[data->menu_current].o_x + 5 &&
							y >= data->menu_list[data->menu_current].o_y - 5 &&
							y <= data->menu_list[data->menu_current].o_y + 5 &&
							(data->menu->flags & POPUPMF_ABOVE ||
							 (x >= 0 && y >= 0 && x < data->menu_list[data->menu_current].pos.Width &&
							  y < data->menu_list[data->menu_current].pos.Height)))
						{
							// Lock open
							data->locked = 1;

							// Now using the left mouse button
							code = SELECTDOWN;
							break;
						}
					}
				}

				// Don't end on button up
				else if (msg_copy.Code & IECODE_UP_PREFIX)
					break;

				// Cleanup and return
				popup_cleanup(data);
				return ret;
			}

			// Mouse move
			case IDCMP_MOUSEMOVE:

				// Still at the first movement stage?
				if (first_move)
				{
					// Have we not moved far enough yet?
					if (window->MouseX >= first_x - 2 && window->MouseX <= first_x + 2 &&
						window->MouseY >= first_y - 2 && window->MouseY <= first_y + 2)
						break;

					// Clear first-move flag
					first_move = 0;
				}

				// Do selection
				if (!popup_select(data, data->menu_current, window->MouseX, window->MouseY))
				{
					// Nothing changed; is this a sub-menu?
					if (data->menu_current > 0)
					{
						short which;

						// See if mouse is over a parent window
						for (which = data->menu_current - 1; which >= 0; which--)
						{
							// Which layer?
							if (WhichLayer(&data->parent_window->WScreen->LayerInfo,
										   window->MouseX + data->parent_window->LeftEdge,
										   window->MouseY + data->parent_window->TopEdge) ==
								data->menu_list[which].window->WLayer)
							{
								// Do selection for parent menu
								if (popup_select(data, which, window->MouseX, window->MouseY))
								{
									short old;

									// Kill sub-menus
									for (old = which + 1; old <= data->menu_current; old++)
									{
										// Remove wait bit
										data->wait_bits &= ~(1 << data->menu_list[which].window->UserPort->mp_SigBit);

										// Close menu display
										popup_close_display(&data->menu_list[old]);
									}

									// On parent menu again
									data->menu_current = which;
								}
								break;
							}
						}
					}
				}
				break;
			}
		}

		// Wait for an event
		Wait(data->wait_bits);
	}
}

// Cleanup popup menus
void popup_cleanup(PopUpData *data)
{
	short a, b = 0;

	// Free DrawInfo
	if (data->drawinfo)
		FreeScreenDrawInfo(data->parent_window->WScreen, data->drawinfo);

	// Free image data
	FreeVec(data->bullet_data);

	// Free timer
	L_FreeTimer(data->timer);

	// Did we have our own port?
	if (data->port)
	{
		struct Message *msg;

		// Install the old port
		Forbid();
		data->parent_window->UserPort = data->old_port;
		ModifyIDCMP(data->parent_window, data->old_idcmp);
		data->parent_window->Flags = data->old_flags;
		Permit();

		// Flush port
		while ((msg = GetMsg(data->port)))
			ReplyMsg(msg);

		// Delete port
		DeleteMsgPort(data->port);
	}

	// Kill menu windows in reverse order
	for (a = MENU_DEPTH - 1; a >= 0; a--)
	{
		if (data->menu_list[a].window)
		{
			popup_close_display(&data->menu_list[a]);
			b = 1;
		}
	}

	// If parent window was active, reactivate it
	if (b && data->was_active)
		ActivateWindow(data->parent_window);

	// Free data
	FreeVec(data);
}

// Initialise popup display
void popup_init_display(PopUpData *data, short which)
{
	struct Rectangle rect;
	struct RastPort *rp;

	// Cache rastport
	rp = data->menu_list[which].window->RPort;

	// Clear background
	EraseRect(rp, 0, 0, data->menu_list[which].pos.Width - 1, data->menu_list[which].pos.Height - 1);

	// Draw border
	rect.MinX = 0;
	rect.MinY = 0;
	rect.MaxX = data->menu_list[which].pos.Width - 1;
	rect.MaxY = data->menu_list[which].pos.Height - 1;
	L_DrawBox(rp, &rect, data->drawinfo, FALSE);

#ifdef SHADOW
	{
		short shadowx, shadowy;
		struct RastPort rp;
		UWORD dither[2];

		// Get shadow size
		shadowx = data->menu_list[which].window->Width - data->menu_list[which].pos.Width;
		shadowy = data->menu_list[which].window->Height - data->menu_list[which].pos.Height;

		// Initialise fake rastport
		InitRastPort(&rp);
		SetAPen(&rp, data->drawinfo->dri_Pens[SHADOWPEN]);
		SetDrMd(&rp, JAM1);
		SetAfPt(&rp, dither, 1);
		dither[0] = 0x5555;
		dither[1] = 0xaaaa;

		// Right bitmap?
		if (data->menu_list[which].right_bitmap)
		{
			// Draw dither into bitmap
			rp.BitMap = data->menu_list[which].right_bitmap;
			RectFill(&rp, 0, SHADOW_Y, SHADOW_X - 1, data->menu_list[which].pos.Height + SHADOW_Y - 1);

			// Copy bitmap to window
			ClipBlit(&rp,
					 0,
					 0,
					 data->menu_list[which].window->RPort,
					 data->menu_list[which].window->Width - shadowx,
					 0,
					 shadowx,
					 data->menu_list[which].pos.Height + shadowy,
					 0xc0);
		}

		// Bottom bitmap?
		if (data->menu_list[which].bottom_bitmap)
		{
			// Draw dither into bitmap
			rp.BitMap = data->menu_list[which].bottom_bitmap;
			RectFill(&rp, SHADOW_X, 0, data->menu_list[which].pos.Width + SHADOW_X - 1, SHADOW_Y - 1);

			// Copy bitmap to window
			ClipBlit(&rp,
					 0,
					 0,
					 data->menu_list[which].window->RPort,
					 0,
					 data->menu_list[which].window->Height - shadowy,
					 data->menu_list[which].pos.Width + shadowx,
					 shadowy,
					 0xc0);
		}
	}
#endif

	// Need to scroll?
	if (data->menu_list[which].scroll)
	{
		// Draw scroll imagery at the top
		SetAPen(rp, data->fpen);
		Move(rp, 1, SCROLL_V_SIZE);
		Draw(rp, data->menu_list[which].pos.Width - 1, SCROLL_V_SIZE);

		// Draw scroll imagery at the bottom
		Move(rp, 1, data->menu_list[which].pos.Height - 1 - SCROLL_V_SIZE);
		Draw(rp, data->menu_list[which].pos.Width - 1, data->menu_list[which].pos.Height - 1 - SCROLL_V_SIZE);

		// Arrow images at the top
		BltTemplate(
			(PLANEPTR)data->scroll_image[0], 0, 2, rp, data->item_h_pad, 1, SCROLLIMAGE_WIDTH, SCROLLIMAGE_HEIGHT);

		BltTemplate((PLANEPTR)data->scroll_image[0],
					0,
					2,
					rp,
					data->menu_list[which].pos.Width - SCROLLIMAGE_WIDTH - 1 - data->item_h_pad,
					1,
					SCROLLIMAGE_WIDTH,
					SCROLLIMAGE_HEIGHT);

		// Arrow images at the bottom
		BltTemplate((PLANEPTR)data->scroll_image[1],
					0,
					2,
					rp,
					data->item_h_pad,
					data->menu_list[which].pos.Height - 1 - SCROLLIMAGE_HEIGHT,
					SCROLLIMAGE_WIDTH,
					SCROLLIMAGE_HEIGHT);

		BltTemplate((PLANEPTR)data->scroll_image[1],
					0,
					2,
					rp,
					data->menu_list[which].pos.Width - SCROLLIMAGE_WIDTH - 1 - data->item_h_pad,
					data->menu_list[which].pos.Height - 1 - SCROLLIMAGE_HEIGHT,
					SCROLLIMAGE_WIDTH,
					SCROLLIMAGE_HEIGHT);
	}
}

// Display popup menus
void popup_display(PopUpData *data, short which)
{
	PopUpItem *item;
	short count, item_y;

	// Set pens
	SetAPen(data->menu_list[which].window->RPort, data->fpen);
	SetBPen(data->menu_list[which].window->RPort, data->bpen);

	// Look at select item
	if (data->menu_list[which].sel_item && data->menu_list[which].sel_item->node.mln_Succ)
	{
		// If it's a barlabel, clear select pointer
		if (!(data->menu_list[which].sel_item->flags & POPUPF_LOCALE) &&
			data->menu_list[which].sel_item->item_name == POPUP_BARLABEL)
			data->menu_list[which].sel_item = 0;
	}

	// Go through items
	for (item = (PopUpItem *)data->menu_list[which].item_list->mlh_Head,
		count = 0,
		item_y = data->menu_list[which].item_y,
		data->menu_list[which].bottom_item = data->menu_list[which].top_item;
		 item->node.mln_Succ;
		 item = (PopUpItem *)item->node.mln_Succ, count++)
	{
		// Is this item displayable?
		if (count >= data->menu_list[which].top_item)
		{
			short height;

			// Show item
			if (!(height = popup_show_item(data, which, item, item_y, (data->menu_list[which].sel_item == item), TRUE)))
				break;

			// Selected item?
			if (data->menu_list[which].sel_item == item)
			{
				// Store position
				data->menu_list[which].sel_item_y = item_y;
			}

			// Increment position
			item_y += height;

			// Increment bottom line
			++data->menu_list[which].bottom_item;
		}
	}

	// Store bottom position
	data->menu_list[which].bottom_y = item_y;

	// Clear to end of display
	if (item_y <= data->menu_list[which].pos.Height - data->menu_list[which].item_y)
	{
		EraseRect(data->menu_list[which].window->RPort,
				  data->menu_list[which].item_x - 1,
				  item_y,
				  data->menu_list[which].pos.Width - data->menu_list[which].item_x,
				  data->menu_list[which].pos.Height - data->menu_list[which].item_y);
	}
}

// Show an item
short popup_show_item(PopUpData *data, short which, PopUpItem *item, short item_y, BOOL sel, BOOL full)
{
	short len, height, fg, bg, x;
	char *text;
	struct TextExtent extent;
	struct RastPort *rp;
	APTR image = 0;

	// Cache rastport
	rp = data->menu_list[which].window->RPort;

	// Get this item's text
	if (item->flags & POPUPF_LOCALE && item->item_name != POPUP_BARLABEL)
	{
		text = L_GetString(data->menu->locale, (LONG)item->item_name);
	}
	else
		text = (char *)item->item_name;

	// Get height
	height = ((IPTR)text == POPUP_BARLABEL) ? data->sep_height : data->menu_list[which].item_height;

	// Will it fit?
	if (item_y + height > data->menu_list[which].item_y + data->menu_list[which].total_height)
		return 0;

	// Bar label?
	if ((IPTR)text == POPUP_BARLABEL)
	{
		short a;

		// Draw separator
		for (a = 0; a < 2; a++)
		{
			SetAPen(rp, data->drawinfo->dri_Pens[SHADOWPEN - a]);
			Move(rp, 1, item_y + 2 + a);
			Draw(rp, data->menu_list[which].pos.Width - 2, item_y + 2 + a);
		}

		return height;
	}

	// If disabled and not doing a full refresh, return
	if (item->flags & POPUPF_DISABLED && !full)
		return height;

	// Selected?
	if (sel && !(item->flags & POPUPF_DISABLED))
	{
		// Get pens
		fg = data->fpen_sel;
		bg = data->bpen_sel;
	}

	// Unselected
	else
	{
		// Get pens
		fg = data->fpen;
		bg = data->bpen;
		sel = 0;
	}

	// Are we showing an image?
	if (!(item->flags & POPUPF_CHECKIT) && item->flags & POPUPF_IMAGE && item->image)
		image = item->image;

	// If not, we draw the left of the item
	else
	{
		// Erase to start of text
		if (bg == data->bpen)
			EraseRect(rp, 1, item_y, data->menu_list[which].item_x - 1, item_y + height - 1);
		else
		{
			SetAPen(rp, bg);
			RectFill(rp, 1, item_y, data->menu_list[which].item_x - 1, item_y + height - 1);
		}
	}

	// Get x position
	x = data->menu_list[which].item_x;

	// Set pens
	SetAPen(rp, fg);
	SetBPen(rp, bg);

	// Show bullet?
	if (item->flags & POPUPF_CHECKIT && item->flags & POPUPF_CHECKED)
	{
		short x, y;

		// Get bullet position
		x = 1 + ((data->menu_list[which].h_pad_left - data->bullet_width) >> 1);
		y = item_y + ((height - data->bullet_height) >> 1);

		// Draw bullet
		SetDrMd(rp, JAM1);
		BltTemplate((PLANEPTR)data->bullet_data, 0, 2, rp, x, y, data->bullet_width, data->bullet_height);
		SetDrMd(rp, JAM2);
	}

	// Or an image to display?
	else if (image)
	{
		struct Rectangle rect;
		struct TagItem tags[8];

		// Get image position
		rect.MinX = 1;
		rect.MinY = item_y;
		rect.MaxX = data->menu_list[which].h_pad_left - data->menu_list[which].h_pad_left_extra - 1;
		rect.MaxY = item_y + height - 1;

		// Fill out tags
		tags[0].ti_Tag = IM_Rectangle;
		tags[0].ti_Data = (IPTR)&rect;
		tags[1].ti_Tag = IM_ClipBoundary;
		tags[1].ti_Data = 1;
		tags[2].ti_Tag = IM_State;
		tags[2].ti_Data = (sel) ? 1 : 0;
		tags[3].ti_Tag = (data->menu->flags & POPUPMF_NO_SCALE) ? TAG_IGNORE : IM_Scale;
		tags[3].ti_Data = 1;
		tags[4].ti_Tag = IM_Erase;
		tags[4].ti_Data = bg;
		tags[5].ti_Tag = IM_Mask;
		tags[5].ti_Data = 1;
		tags[6].ti_Tag = IM_Backfill;
		tags[6].ti_Data = (data->menu->flags & POPUPMF_BACKFILL && data->menu->backfill) ? TRUE : FALSE;
		tags[7].ti_Tag = TAG_END;

		// Draw image
		L_RenderImage(rp, image, 0, 0, tags);

		// Reset pens
		SetAPen(rp, fg);
		SetBPen(rp, bg);
	}

	// Valid text?
	if (text)
	{
		short y;

		// If we have a backfill, switch to JAM1
		if (bg == data->bpen && data->menu->flags & POPUPMF_BACKFILL && data->menu->backfill)
			SetDrMd(rp, JAM1);
		else if (rp->DrawMode != JAM2)
			SetDrMd(rp, JAM2);

		// Get y position
		y = item_y + data->font->tf_Baseline;
		y += (height - data->font->tf_YSize) >> 1;

		// Get length of text
		len = TextFit(rp,
					  text,
					  strlen(text),
					  &extent,
					  0,
					  1,
					  data->menu_list[which].item_width + ((((struct Library *)GfxBase)->lib_Version < 39) ? 1 : 0),
					  height);

		// Display text
		if (len > 0)
		{
			// Show text
			Move(rp, data->menu_list[which].item_x, y);
			Text(rp, text, len);
			x += extent.te_Width;

			// Disabled?
			if (item->flags & POPUPF_DISABLED)
			{
				UWORD ghost[2];

				// Set ghosting pattern
				ghost[0] = 0x8888;
				ghost[1] = 0x2222;
				SetAfPt(rp, ghost, 1);
				SetDrMd(rp, JAM1);

				// Ghost in background colour
				SetAPen(rp, bg);
				RectFill(rp,
						 data->menu_list[which].item_x,
						 item_y,
						 data->menu_list[which].item_x + extent.te_Width - 1,
						 item_y + height - 1);

				// Clear ghosting pattern
				SetAfPt(rp, 0, 0);
				SetDrMd(rp, JAM2);
			}
		}

		// Erase left, below and above text
		if (height > data->font->tf_YSize)
		{
			if (bg == data->bpen)
			{
				EraseRect(rp,
						  data->menu_list[which].h_pad_left - data->menu_list[which].h_pad_left_extra,
						  item_y,
						  data->menu_list[which].item_x - 1,
						  item_y + height - 1);
				EraseRect(rp, data->menu_list[which].item_x, item_y, x - 1, y - data->font->tf_Baseline - 1);
				EraseRect(rp,
						  data->menu_list[which].item_x,
						  y - data->font->tf_Baseline + data->font->tf_YSize,
						  x - 1,
						  item_y + height - 1);
			}
			else
			{
				SetAPen(rp, bg);
				RectFill(rp,
						 data->menu_list[which].h_pad_left - data->menu_list[which].h_pad_left_extra,
						 item_y,
						 data->menu_list[which].item_x - 1,
						 item_y + height - 1);
				RectFill(rp, data->menu_list[which].item_x, item_y, x - 1, y - data->font->tf_Baseline - 1);
				RectFill(rp,
						 data->menu_list[which].item_x,
						 y - data->font->tf_Baseline + data->font->tf_YSize,
						 x - 1,
						 item_y + height - 1);
			}
		}
	}

	// Erase to end of line
	if (x <= data->menu_list[which].pos.Width - 2)
	{
		if (bg == data->bpen)
			EraseRect(rp, x, item_y, data->menu_list[which].pos.Width - 2, item_y + height - 1);
		else
		{
			SetAPen(rp, bg);
			RectFill(rp, x, item_y, data->menu_list[which].pos.Width - 2, item_y + height - 1);
		}
	}

	// Show sub marker?
	if (item->flags & POPUPF_SUB)
	{
		short x, y;

		// Get sub position
		x = data->menu_list[which].pos.Width - data->menu_list[which].h_pad_right - 2 +
			((data->menu_list[which].h_pad_right - data->sub_width) >> 1);
		y = item_y + ((height - data->sub_height) >> 1);

		// Draw sub
		SetAPen(rp, fg);
		SetDrMd(rp, JAM1);
		BltTemplate((PLANEPTR)data->sub_data, 0, 2, rp, x, y, data->sub_width, data->sub_height);
		SetDrMd(rp, JAM2);
	}

	return height;
}

// Mouse moved; change selection
BOOL popup_select(PopUpData *data, short which, short x, short y)
{
	PopUpItem *item;
	short count, item_y;

	// Convert coordinates to menu-relative
	x += data->parent_window->LeftEdge - data->menu_list[which].pos.Left;
	y += data->parent_window->TopEdge - data->menu_list[which].pos.Top;

	// Not scrolling by default
	data->menu_list[which].scroll_dir = 0;

	// See if this list can scroll
	if (data->menu_list[which].scroll && x >= 0 && x < data->menu_list[which].pos.Width)
	{
		// Scrolling up?
		if (y < data->menu_list[which].item_y)
		{
			data->menu_list[which].scroll_dir = -1;
			data->menu_list[which].sel_item = 0;
			popup_display(data, which);
			return 0;
		}

		// Scrolling down?
		if (y > data->menu_list[which].pos.Height - data->menu_list[which].item_y)
		{
			data->menu_list[which].scroll_dir = 1;
			data->menu_list[which].sel_item = 0;
			popup_display(data, which);
			return 0;
		}
	}

	// Is mouse off the menu?
	if (x < 0 || x >= data->menu_list[which].pos.Width || y < 0 || y >= data->menu_list[which].pos.Height)
	{
		// Moved off menu?
		if (data->menu_list[which].sel_item)
		{
			// Unselect old item
			popup_show_item(
				data, which, data->menu_list[which].sel_item, data->menu_list[which].sel_item_y, FALSE, FALSE);
			data->menu_list[which].sel_item = 0;
			data->tick_count = 0;
			return 1;
		}
		return 0;
	}

	// Go through items
	for (item = (PopUpItem *)data->menu_list[which].item_list->mlh_Head,
		count = 0,
		item_y = data->menu_list[which].item_y;
		 item->node.mln_Succ;
		 item = (PopUpItem *)item->node.mln_Succ, count++)
	{
		// Is this item displayable?
		if (count >= data->menu_list[which].top_item)
		{
			// Are we over this item?
			if (y >= item_y && y < item_y + ((item->item_name == POPUP_BARLABEL) ? data->sep_height
																				 : data->menu_list[which].item_height))
			{
				// Selected item changed?
				if (data->menu_list[which].sel_item != item)
				{
					// Unselect old item
					if (data->menu_list[which].sel_item)
					{
						popup_show_item(data,
										which,
										data->menu_list[which].sel_item,
										data->menu_list[which].sel_item_y,
										FALSE,
										FALSE);
					}

					// Store new item
					data->menu_list[which].sel_item = item;
					data->menu_list[which].sel_item_y = item_y;
					data->tick_count = 0;

					// Highlight new selection
					popup_show_item(data, which, item, item_y, TRUE, FALSE);
					return 1;
				}
			}

			// Increment y position
			item_y += (item->item_name == POPUP_BARLABEL) ? data->sep_height : data->menu_list[which].item_height;
		}
	}

	return 0;
}

PopUpItem *getpopupitem(struct MinList *, UWORD);

// Get item in a pop-up menu
PopUpItem *LIBFUNC L_GetPopUpItem(REG(a0, PopUpMenu *menu), REG(d0, UWORD id))
{
	if (!menu)
		return 0;
	return getpopupitem(&menu->item_list, id);
}

PopUpItem *getpopupitem(struct MinList *list, UWORD id)
{
	PopUpItem *item;

	// Go through items
	for (item = (PopUpItem *)list->mlh_Head; item->node.mln_Succ; item = (PopUpItem *)item->node.mln_Succ)
	{
		// Match id?
		if (item->id == id)
			return item;

		// Sub-items?
		if (item->flags & POPUPF_SUB && item->data)
		{
			PopUpItem *sub;

			// Find in sub-items
			if ((sub = getpopupitem((struct MinList *)item->data, id)))
				return sub;
		}
	}

	// Didn't find it
	return 0;
}

BOOL popup_init_list(PopUpData *data, short which)
{
	PopUpItem *item;
	unsigned short max_height = 0;
	struct RastPort rp;
#ifdef SHADOW
	short depth;
#endif

	// Initial sizes
	data->menu_list[which].item_width = 0;
	data->menu_list[which].item_height = data->font->tf_YSize;
	data->menu_list[which].h_pad_left = data->item_h_pad;
	data->menu_list[which].h_pad_right = data->item_h_pad;

	// Not scaling images?
	if (data->menu->flags & POPUPMF_NO_SCALE)
	{
		// Scan menu items
		for (item = (PopUpItem *)data->menu_list[which].item_list->mlh_Head; item->node.mln_Succ;
			 item = (PopUpItem *)item->node.mln_Succ)
		{
			// Not a separator?
			if (item->item_name == POPUP_BARLABEL)
				continue;

			// Does item have an image to display?
			if (!(item->flags & POPUPF_CHECKIT) && item->flags & POPUPF_IMAGE && item->image)
			{
				struct TagItem tags[3];

				// Get image size
				tags[0].ti_Tag = IM_Width;
				tags[1].ti_Tag = IM_Height;
				tags[2].ti_Tag = TAG_END;
				L_GetImageAttrs(item->image, tags);

				// Extra space for images
				data->menu_list[which].h_pad_left_extra = 4;

				// Add padding to image size
				tags[0].ti_Data += 2 + data->menu_list[which].h_pad_left_extra;
				tags[1].ti_Data += 2;

				// Biggest image so far?
				if (data->menu_list[which].item_height < tags[1].ti_Data)
					data->menu_list[which].item_height = tags[1].ti_Data;

				// Check there's room on the left
				if (data->menu_list[which].h_pad_left < tags[0].ti_Data)
					data->menu_list[which].h_pad_left = tags[0].ti_Data;
			}
		}
	}

	// Initialise dummy rastport and set font
	InitRastPort(&rp);
	SetFont(&rp, data->font);

	// Scan menu items
	for (item = (PopUpItem *)data->menu_list[which].item_list->mlh_Head, data->menu_list[which].pos.Height = ITEM_V_PAD;
		 item->node.mln_Succ;
		 item = (PopUpItem *)item->node.mln_Succ)
	{
		short len;
		char *text;

		// Get this item's text
		if (item->flags & POPUPF_LOCALE)
			text = L_GetString(data->menu->locale, (LONG)item->item_name);
		else
			text = (char *)item->item_name;

		// Check valid text
		if (text && (IPTR)text != POPUP_BARLABEL)
		{
			// Get length of text
			len = TextLength(&rp, text, strlen(text));

			// Greater than current width?
			if (len > data->menu_list[which].item_width)
				data->menu_list[which].item_width = len;

			// Increment height
			data->menu_list[which].pos.Height += data->menu_list[which].item_height;

			// Increment count
			++data->menu_list[which].item_count;
		}

		// Increment item count
		else if ((IPTR)text == POPUP_BARLABEL)
		{
			++data->menu_list[which].sep_count;
			data->menu_list[which].pos.Height += data->sep_height;
		}
		else
		{
			++data->menu_list[which].item_count;
			data->menu_list[which].pos.Height += data->menu_list[which].item_height;
		}

		// Not too high for screen?
		if (data->menu_list[which].pos.Height <= data->parent_window->WScreen->Height - (SCROLL_V_SIZE << 1))
			max_height = data->menu_list[which].pos.Height;
	}

	// Valid item count?
	if (data->menu_list[which].item_count < 1)
		return 0;

	// Increase item width to compensate for incorrect char length
	// returned by OS4 TextFit() function in menu layout.

	data->menu_list[which].item_width++;

	// Calculate menu width
	data->menu_list[which].pos.Width =
		data->menu_list[which].item_width + data->menu_list[which].h_pad_left + data->menu_list[which].h_pad_right;

	// Bounds-check width
	if (data->menu_list[which].pos.Width > data->parent_window->WScreen->Width)
		data->menu_list[which].pos.Width = data->parent_window->WScreen->Width;

	// Get final item width
	data->menu_list[which].item_width =
		data->menu_list[which].pos.Width - data->menu_list[which].h_pad_left - data->menu_list[which].h_pad_right;

	// Get item offsets
	data->menu_list[which].item_x = data->menu_list[which].h_pad_left;
	data->menu_list[which].item_y = ITEM_V_PAD >> 1;
	data->menu_list[which].item_count += data->menu_list[which].sep_count;

	// Bounds-check height
	if (data->menu_list[which].pos.Height > data->parent_window->WScreen->Height)
	{
		// Too big, we need to scroll
		if (max_height)
			data->menu_list[which].pos.Height = max_height + (SCROLL_V_SIZE << 1);
		else
			data->menu_list[which].pos.Height = data->parent_window->WScreen->Height;
		data->menu_list[which].scroll = 1;

		// Increase vertical padding for imagery
		data->menu_list[which].item_y += SCROLL_V_SIZE;
	}

	// Total visible height
	data->menu_list[which].total_height = data->menu_list[which].pos.Height - (data->menu_list[which].item_y << 1);

	// If this is a sub-menu, get position from parent item
	if (which > 0)
	{
		short left;
		short space = (data->menu_list[which].h_pad_right << 1);

		// Start to right of parent menu
		left = data->menu_list[which - 1].pos.Left + data->menu_list[which - 1].pos.Width - space;

		// Must be greater than left of parent
		if (left < data->menu_list[which - 1].pos.Left + space)
			left = data->menu_list[which - 1].pos.Left + space;

		// See if window is off the screen to the right
		if (left + data->menu_list[which].pos.Width > data->parent_window->WScreen->Width)
			left = data->parent_window->WScreen->Width - data->menu_list[which].pos.Width;

		// Does clipping put the window over the parent?
		if (left < data->menu_list[which - 1].pos.Left + (data->menu_list[which - 1].pos.Width >> 1))
			left = data->menu_list[which - 1].pos.Left - data->menu_list[which].pos.Width + 4;

		// Store left
		data->menu_list[which].pos.Left = left;

		// Top of sub-menu at top of item
		data->menu_list[which].pos.Top = data->menu_list[which - 1].pos.Top + data->menu_list[which - 1].sel_item_y;
	}

	// Get position from parent window?
	else if (data->menu->flags & POPUPMF_ABOVE && data->parent_window)
	{
		// Open above window
		data->menu_list[which].pos.Left = data->parent_window->LeftEdge;
		data->menu_list[which].pos.Top = data->parent_window->TopEdge - data->menu_list[which].pos.Height;
	}

	// Otherwise, mouse based
	else
	{
		// Get current position from mouse
		data->menu_list[which].pos.Left = data->parent_window->WScreen->MouseX - 8;
		data->menu_list[which].pos.Top = data->parent_window->WScreen->MouseY - 4;
	}

#ifdef SHADOW
	// Check menu position
	if (data->menu_list[which].pos.Left + data->menu_list[which].pos.Width + SHADOW_X >
		data->parent_window->WScreen->Width)
	{
		data->menu_list[which].pos.Left =
			data->parent_window->WScreen->Width - data->menu_list[which].pos.Width - SHADOW_X;
	}
	if (data->menu_list[which].pos.Left < 0)
		data->menu_list[which].pos.Left = 0;
	if (data->menu_list[which].pos.Top + data->menu_list[which].pos.Height + SHADOW_Y >
		data->parent_window->WScreen->Height)
	{
		data->menu_list[which].pos.Top =
			data->parent_window->WScreen->Height - data->menu_list[which].pos.Height - SHADOW_Y;
	}
	if (data->menu_list[which].pos.Top < 0)
		data->menu_list[which].pos.Top = 0;

	// Get depth
	if (((struct Library *)GfxBase)->lib_Version >= 39)
		depth = GetBitMapAttr(data->parent_window->WScreen->RastPort.BitMap, BMA_DEPTH);
	else
		depth = data->parent_window->WScreen->RastPort.BitMap->Depth;

	// Allocate bitmaps to store 'underneath' imagery
	if ((data->menu_list[which].right_bitmap = L_NewBitMap(SHADOW_X,
														   data->menu_list[which].pos.Height + SHADOW_Y,
														   depth,
														   BMF_CLEAR,
														   data->parent_window->WScreen->RastPort.BitMap)))
	{
		// Copy image to bitmap
		BltBitMap(data->parent_window->WScreen->RastPort.BitMap,
				  data->menu_list[which].pos.Left + data->menu_list[which].pos.Width,
				  data->menu_list[which].pos.Top,
				  data->menu_list[which].right_bitmap,
				  0,
				  0,
				  SHADOW_X,
				  data->menu_list[which].pos.Height + SHADOW_Y,
				  0xc0,
				  0xff,
				  0);
	}

	// Bottom of menu
	if ((data->menu_list[which].bottom_bitmap = L_NewBitMap(data->menu_list[which].pos.Width + SHADOW_X,
															SHADOW_Y,
															depth,
															BMF_CLEAR,
															data->parent_window->WScreen->RastPort.BitMap)))
	{
		// Copy image to bitmap
		BltBitMap(data->parent_window->WScreen->RastPort.BitMap,
				  data->menu_list[which].pos.Left,
				  data->menu_list[which].pos.Top + data->menu_list[which].pos.Height,
				  data->menu_list[which].bottom_bitmap,
				  0,
				  0,
				  data->menu_list[which].pos.Width + SHADOW_X,
				  SHADOW_Y,
				  0xc0,
				  0xff,
				  0);
	}
#endif

	// Create menu
	if (!(data->menu_list[which].window =
			  OpenWindowTags(0,
							 WA_Left,
							 data->menu_list[which].pos.Left,
							 WA_Top,
							 data->menu_list[which].pos.Top,
							 WA_Width,
							 data->menu_list[which].pos.Width + SHADOW_X,
							 WA_Height,
							 data->menu_list[which].pos.Height + SHADOW_Y,
							 WA_Borderless,
							 TRUE,
							 WA_CustomScreen,
							 data->parent_window->WScreen,
							 WA_WindowName,
							 DOPUS_WIN_NAME,
							 WA_SimpleRefresh,
							 TRUE,
							 WA_IDCMP,
							 IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS,
							 (data->menu->flags & POPUPMF_BACKFILL && data->menu->backfill) ? WA_BackFill : TAG_IGNORE,
							 data->menu->backfill,
							 TAG_END)))
	{
		return 0;
	}

	// Set ID
	L_SetWindowID(data->menu_list[which].window, &data->menu_list[which].id, WINDOW_POPUP_MENU, 0);

	// Save position
	data->menu_list[which].pos.Left = data->menu_list[which].window->LeftEdge;
	data->menu_list[which].pos.Top = data->menu_list[which].window->TopEdge;

	// Save original mouse position
	data->menu_list[which].o_x = data->menu_list[which].window->MouseX;
	data->menu_list[which].o_y = data->menu_list[which].window->MouseY;

	// Get wait bit
	data->wait_bits |= 1 << data->menu_list[which].window->UserPort->mp_SigBit;

	// Set draw mode and font
	SetDrMd(data->menu_list[which].window->RPort, JAM2);
	SetFont(data->menu_list[which].window->RPort, data->font);

	// Initialise display
	popup_init_display(data, which);

	// Display items
	popup_display(data, which);

	return 1;
}

// Close menu display
void popup_close_display(PopUpList *list)
{
	// Close window
	if (list->window)
	{
		CloseWindow(list->window);
		list->window = 0;
	}

	// Free shadow bitmaps
	if (list->right_bitmap)
	{
		L_DisposeBitMap(list->right_bitmap);
		list->right_bitmap = 0;
	}
	if (list->bottom_bitmap)
	{
		L_DisposeBitMap(list->bottom_bitmap);
		list->bottom_bitmap = 0;
	}
	list->sel_item = 0;
}

void LIBFUNC L_GetPopUpImageSize(REG(a0, struct Window *window),
								 REG(a1, PopUpMenu *menu),
								 REG(a2, short *width),
								 REG(a3, short *height))
{
	short sub_width;

	// Lo-res screen?
	if (L_ScreenInfo(window->WScreen) & SCRI_LORES)
	{
		// Small images?
		if (window->WScreen->RastPort.Font->tf_YSize <= 13)
		{
			sub_width = SMALL_LO_SUB_WIDTH;
		}

		// Big
		else
		{
			sub_width = BIG_LO_SUB_WIDTH;
		}
	}

	// Hi-res screen
	else
	{
		// Small images?
		if (window->WScreen->RastPort.Font->tf_YSize <= 15)
		{
			sub_width = SMALL_HI_SUB_WIDTH;
		}

		// Big
		else
		{
			sub_width = BIG_HI_SUB_WIDTH;
		}
	}

	// Get default size
	*width = window->WScreen->RastPort.Font->tf_XSize + window->WScreen->RastPort.Font->tf_YSize + 2;
	*height = window->WScreen->RastPort.Font->tf_YSize;

	// Check width
	if (*width < sub_width)
		*width = sub_width;

	// Make image square
	if (*width > *height)
		*width = *height;
}

void LIBFUNC L_SetPopUpDelay(REG(d0, short delay), REG(a6, struct MyLibrary *libbase))
{
	struct LibData *data;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = dopuslibbase_global;
#endif

	// Get data pointer, store delay value
	data = (struct LibData *)libbase->ml_UserData;
	data->popup_delay = delay;
}
