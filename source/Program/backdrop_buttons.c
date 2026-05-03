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

BOOL backdrop_handle_button(BackdropInfo *info, struct IntuiMessage *msg, unsigned short flags)
{
	BOOL handled = 1;
	BOOL fix_menus = 0;

	// Get lock
	GetSemaphore(&info->idcmp_lock, SEMF_EXCLUSIVE, 0);

	// Select down?
	if (msg->Code == SELECTDOWN)
	{
		BackdropObject *object, *sel_object = 0;
		short count = 0;

		// Stop dragging anything
		backdrop_stop_drag(info);

		// Outside allowable area?
		if (msg->MouseX < info->size.MinX || msg->MouseX > info->size.MaxX || msg->MouseY < info->size.MinY ||
			msg->MouseY > info->size.MaxY)
		{
			// Release lock
			FreeSemaphore(&info->idcmp_lock);
			return 0;
		}

		// Is this a lister?
		if (info->lister)
		{
			// If window isn't the source, make it so
			if (!(info->lister->flags & (LISTERF_SOURCE | LISTERF_SOURCEDEST_LOCK)))
				lister_do_function(info->lister, MENU_LISTER_SOURCE);

			// Refresh title bar
			if (info->lister->flags & LISTERF_SHOW_TITLE)
			{
				lister_refresh_name(info->lister);
				info->lister->flags &= ~LISTERF_SHOW_TITLE;
			}
		}

		// Quickly set RMBTRAP
		info->window->Flags |= WFLG_RMBTRAP;

		// If the icon positioning flag is set, we don't look for an icon
		if (!(GUI->flags2 & GUIF2_ICONPOS) || !(info->flags & BDIF_MAIN_DESKTOP))
		{
			// Lock backdrop list
			lock_listlock(&info->objects, 1);

			// See if we clicked on an object
			if ((sel_object = backdrop_get_object(info, msg->MouseX, msg->MouseY, BDGOF_CHECK_LABEL)))
			{
				// Remove object from list
				Remove(&sel_object->node);

				// Add to head of list
				AddHead(&info->objects.list, &sel_object->node);
			}

			// Unlock exclusive lock
			unlock_listlock(&info->objects);

			// Lock backdrop list
			lock_listlock(&info->objects, 0);

			// See if current selection is still valid
			sel_object = find_backdrop_object(info, sel_object);

			// Go through backdrop list
			for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
				 object = (BackdropObject *)object->node.ln_Succ)
			{
				// Is object selected?
				if (object->state && object != sel_object)
				{
					++count;
				}
			}

			// Deselect all objects?
			if (!(msg->Qualifier & IEQUAL_ANYSHIFT) && (!sel_object || !sel_object->state))
			{
				short num;

				// Go through again
				for (object = (BackdropObject *)info->objects.list.lh_Head, num = 0;
					 object->node.ln_Succ && num < count;
					 object = (BackdropObject *)object->node.ln_Succ)
				{
					// Is object selected?
					if (object->state && object != sel_object)
					{
						// Deselect object
						object->state = 0;
						object->flags |= BDOF_STATE_CHANGE;

						// Refresh object if count is small enough
						if (count < 4)
							backdrop_render_object(info, object, BRENDERF_CLIP);
						++num;
					}
				}

				// Need to redraw?
				if (count >= 4)
					backdrop_show_objects(info, 0);

				// Clear first selected tool
				info->first_sel_tool = 0;

				// If we didn't click on anything else, we need to fix menus
				if (count > 0 && !sel_object)
					fix_menus = 1;
			}
		}

		// Object clicked on?
		if (sel_object)
		{
			BOOL dblclk = 0;

			// Double-click on an object?
			if (sel_object == info->last_sel_object &&
				(DoubleClick(info->seconds, info->micros, msg->Seconds, msg->Micros)))
				dblclk = 1;

			// If icon is not already selected, select it
			if (!sel_object->state)
			{
				// Select object
				sel_object->state = 1;
				sel_object->flags |= BDOF_STATE_CHANGE;

				// If we had nothing else selected before, we need to fix menus
				if (count == 0)
					fix_menus = 1;
			}

			// If shift is held down (and not double-clicked on something), deselect icon
			else if (msg->Qualifier & IEQUAL_ANYSHIFT && !dblclk)
			{
				// Deselect object
				sel_object->state = 0;
				sel_object->flags |= BDOF_STATE_CHANGE;

				// If we had one thing selected before, we need to fix menus
				if (count == 1)
					fix_menus = 1;
			}

			// Render object
			backdrop_render_object(info, sel_object, BRENDERF_CLIP);

			// Was object actually selected?
			if (sel_object->state)
			{
				// Is this a tool?
				if (sel_object->type != BDO_APP_ICON && sel_object->icon->do_Type == WBTOOL)
				{
					// If no tools selected, remember this one
					if (!info->first_sel_tool)
						info->first_sel_tool = sel_object;
				}

				// Double-click on an object?
				if (dblclk)
				{
					// If shift is not down, and we got a double-click, deselect all others
					if (!(msg->Qualifier & IEQUAL_ANYSHIFT))
					{
						short num;

						// Go through icons
						for (object = (BackdropObject *)info->objects.list.lh_Head, num = 0;
							 object->node.ln_Succ && num < count;
							 object = (BackdropObject *)object->node.ln_Succ)
						{
							// Is object selected, and not the one we double-clicked on?
							if (object->state && object != sel_object)
							{
								// Deselect object
								object->state = 0;
								object->flags |= BDOF_STATE_CHANGE;

								// Refresh object if count is small enough
								if (count < 4)
									backdrop_render_object(info, object, BRENDERF_CLIP);
								++num;
							}
						}

						// Need to redraw icons?
						if (count >= 4)
							backdrop_show_objects(info, 0);

						// Clear first selected tool
						info->first_sel_tool = 0;
					}

					// Open object
					backdrop_object_open(info, sel_object, msg->Qualifier, 1, 0, 0);

					// Clear double-click pointers
					info->last_sel_object = 0;
					info->seconds = 0;
					info->flags &= ~BDIF_DRAGGING;

					// Flush message port
					flush_port(info->window->UserPort);

					// Enable menus again
					info->window->Flags &= ~WFLG_RMBTRAP;
				}

				// Otherwise
				else
				{
					// Save object and timestamp
					info->last_sel_object = sel_object;
					info->seconds = msg->Seconds;
					info->micros = msg->Micros;

					// Start dragging this object
					if (!(backdrop_start_drag(info, info->window->WScreen->MouseX, info->window->WScreen->MouseY)))
					{
						// Fix menus
						if (fix_menus && info->lister)
							lister_fix_menus(info->lister, 1);

						// Release lock
						FreeSemaphore(&info->idcmp_lock);
						return 1;
					}

					// Calculate drag offset
					sel_object->drag_x_offset = sel_object->pos.Left - msg->MouseX;
					sel_object->drag_y_offset = sel_object->pos.Top - msg->MouseY;
				}
			}
		}

		// Otherwise, begin rubber banding
		else
		{
			BOOL noband = 0, store = 1;

			// Double-click on background?
			if (flags & BIDCMPF_DOUBLECLICK && !info->last_sel_object && info->last_button == SELECTDOWN &&
				(DoubleClick(info->seconds, info->micros, msg->Seconds, msg->Micros)))
			{
				// Close enough to original position?
				if (msg->MouseX >= info->select.MinX - 5 && msg->MouseX <= info->select.MinX + 5 &&
					msg->MouseY >= info->select.MinY - 5 && msg->MouseY <= info->select.MinY + 5)
				{
					// Run double-click script
					noband = RunScript(SCRIPT_DOUBLECLICK, 0);
					info->seconds = 0;

					// Enable menus again
					info->window->Flags &= ~WFLG_RMBTRAP;
					store = 0;
				}
			}

			// Store doubleclick info
			if (store)
			{
				info->seconds = msg->Seconds;
				info->micros = msg->Micros;
			}

			// Clear last object pointer
			info->last_sel_object = 0;

			// Store coordinates
			info->select.MinX = msg->MouseX;
			info->select.MinY = msg->MouseY;

			// Ok to rubber band?
			if (!noband)
			{
				// Set flag, enable mouse reporting
				info->flags |= BDIF_RUBBERBAND;
				info->window->Flags |= WFLG_REPORTMOUSE;

				// Turn ticks on
				ModifyIDCMP(info->window, info->window->IDCMPFlags | IDCMP_INTUITICKS);
				info->tick_count = 1;
				info->last_tick = 0;

				// Initial coordinates
				info->select.MaxX = info->select.MinX;
				info->select.MaxY = info->select.MinY;

				// Show box (nop?)
				backdrop_rubber_band(info, 0);
			}
		}

		// Unlock backdrop list (unless dragging)
		if (!(info->flags & BDIF_DRAGGING) && (!(GUI->flags2 & GUIF2_ICONPOS) || !(info->flags & BDIF_MAIN_DESKTOP)))
			unlock_listlock(&info->objects);

		// Fix selection count
		if (!(GUI->flags & GUIF2_ICONPOS) || !(info->flags & BDIF_MAIN_DESKTOP))
			backdrop_fix_count(info, 1);
	}

	// Stop dragging?
	else if (msg->Code == MENUDOWN || (msg->Code & IECODE_UP_PREFIX))
	{
		// Rubber-banding something?
		if (info->flags & BDIF_RUBBERBAND)
		{
			// Stop dragging
			backdrop_stop_drag(info);

			// Icon positioning?
			if (GUI->flags2 & GUIF2_ICONPOS && info->flags & BDIF_MAIN_DESKTOP)
			{
				// Add new area?
				if (msg->Code == SELECTUP)
				{
					struct Rectangle rect;

					// Get selection rectangle, convert to screen coordinates
					rect = info->select;
					rect.MinX += info->window->LeftEdge;
					rect.MinY += info->window->TopEdge;
					rect.MaxX += info->window->LeftEdge;
					rect.MaxY += info->window->TopEdge;

					// Check rectangle isn't 'negative'
					if (rect.MaxX < rect.MinX)
					{
						short temp = rect.MaxX;
						rect.MaxX = rect.MinX;
						rect.MinX = temp;
					}
					if (rect.MaxY < rect.MinY)
					{
						short temp = rect.MaxY;
						rect.MaxY = rect.MinY;
						rect.MinY = temp;
					}

					// Add new icon positioning area
					iconpos_new_area(&rect);
				}
			}

			// Select objects
			else
				backdrop_select_area(info, (msg->Code == SELECTUP) ? 1 : 0);

			// Fix menus
			fix_menus = 1;
		}

		// Are we dragging something?
		else if (info->flags & BDIF_DRAGGING)
		{
			BOOL nodrag;

			// Stop dragging
			nodrag = backdrop_stop_drag(info);

			// Reposition objects?
			if (msg->Code == SELECTUP && info->last_sel_object && !nodrag)
			{
				short x, y;
				struct Layer *layer;
				struct Window *window;
				struct AppWindow *appwindow = 0;
				BOOL fail = 0;

				// Convert mouse coordinates to screen-relative
				x = msg->MouseX + info->window->LeftEdge;
				y = msg->MouseY + info->window->TopEdge;

				// Lock layer
				LockScreenLayer(info->window->WScreen);

				// Find which layer we dropped it on
				if ((layer = WhichLayer(&info->window->WScreen->LayerInfo, x, y)))
				{
					// Does layer have a window?
					if ((window = layer->Window))
					{
						BOOL ok = 0;

						// Is it our window?
						if (window == info->window)
						{
							BackdropObject *drop_obj;

							// Lock backdrop list
							lock_listlock(&info->objects, 0);

							// See if it's dropped on anything
							if ((drop_obj = backdrop_get_object(info, msg->MouseX, msg->MouseY, 0)))
							{
								// Is shift/alt down?
								if ((msg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_LALT)) ==
									(IEQUALIFIER_LSHIFT | IEQUALIFIER_LALT))
								{
									// Replace the image
									backdrop_replace_icon_image(info, 0, drop_obj);
									fail = 2;
								}

								// Check it's not dropped on itself
								else if (drop_obj != info->last_sel_object && !drop_obj->state)
								{
									// Can drop on appicons...
									if (drop_obj->type == BDO_APP_ICON)
									{
										// If the icon isn't busy
										if (!(drop_obj->flags & BDOF_BUSY))
											ok = 1;
									}

									// On groups...
									else if (drop_obj->type == BDO_GROUP)
										ok = 1;

									// On leftouts, if...
									else if (drop_obj->type == BDO_LEFT_OUT)
									{
										// They're tools, drawers, projects or trashcans
										if (drop_obj->icon->do_Type == WBTOOL || drop_obj->icon->do_Type == WBDRAWER ||
											drop_obj->icon->do_Type == WBPROJECT ||
											drop_obj->icon->do_Type == WBGARBAGE)
											ok = 1;
									}

									// On disks if this is a left-out
									else if (drop_obj->type == BDO_DISK && info->last_sel_object->type == BDO_LEFT_OUT)
										ok = 1;
								}
							}

							// Unlock backdrop list
							unlock_listlock(&info->objects);
						}

						// Not our window, ok to do drop
						else
							ok = 1;

						// Ok to drop?
						if (ok)
						{
							// Forbid
							Forbid();

							// AppWindow?
							if ((appwindow = WB_FindAppWindow(window)))
							{
								// Do app stuff
								backdrop_drop_appwindow(info, appwindow, x - window->LeftEdge, y - window->TopEdge);
							}

							// Permit
							Permit();
							fail = 1;
						}
					}
					else
						fail = 1;
				}
				else
					fail = 1;

				// Unlock layer
				UnlockScreenLayer(info->window->WScreen);

				// Ok to reposition?
				if (!fail)
				{
					// Get new position
					x = msg->MouseX + info->last_sel_object->drag_x_offset;
					y = msg->MouseY + info->last_sel_object->drag_y_offset;

					// Has position changed?
					if (x != info->last_sel_object->pos.Left || y != info->last_sel_object->pos.Top)
					{
						BackdropObject *object, *on_object;

						// Convert to delta
						x -= info->last_sel_object->pos.Left;
						y -= info->last_sel_object->pos.Top;

						// Lock backdrop list
						lock_listlock(&info->objects, 0);

						// See if we dropped it on an object
						if ((on_object = backdrop_get_object(info, msg->MouseX, msg->MouseY, 0)) && !on_object->state)
						{
							// Handle drop onto another object
							backdrop_drop_object(info, on_object);
						}

						// We didn't
						else
						{
							// Clear region
							backdrop_clear_region(info);

							// Go through backdrop list
							for (object = (BackdropObject *)info->objects.list.lh_Head; object->node.ln_Succ;
								 object = (BackdropObject *)object->node.ln_Succ)
							{
								// Object selected, and not locked?
								if (object->state && !(object->flags & BDOF_LOCKED))
								{
									struct Rectangle new;
									short old_left, old_top;

									// Remember old position
									old_left = object->pos.Left;
									old_top = object->pos.Top;

									// Set new position
									backdrop_icon_position(info, object, old_left + x, old_top + y);

									// Get new movement delta
									x = object->pos.Left - old_left;
									y = object->pos.Top - old_top;

									// Add old position to region
									backdrop_add_region(info, &object->show_rect);

									// Get new position
									new = object->show_rect;
									new.MinX += x;
									new.MinY += y;
									new.MaxX += x;
									new.MaxY += y;

									// Add new position to region
									backdrop_add_region(info, &new);

									// Fix show rectangle
									object->full_size.MinX -= x;
									object->full_size.MaxX -= x;
									object->full_size.MinY -= y;
									object->full_size.MaxY -= y;
								}
							}

							// Finalise region
							backdrop_region_bounds(info);

							// Show all objects
							backdrop_show_objects(info, BDSF_CLEAR);

							// Restore region
							backdrop_install_clip(info);

							// Update virtual size
							backdrop_calc_virtual(info);
						}

						// Unlock backdrop list
						unlock_listlock(&info->objects);

						// Did we move very far?
						if (x < -4 || x > 4 || y < -4 || y > 4)
						{
							// Moved too far for double-click; clear last sel pointer
							info->last_sel_object = 0;
						}
					}
				}

				// Can't drop here
				else if (!appwindow && fail != 2)
					DisplayBeep(info->window->WScreen);
			}
		}

		// Pop-up over icon?
		else if (msg->Code == MENUDOWN)
		{
			BOOL ok = 1;

			// Lister?
			if (flags & BIDCMPF_LISTER && info->lister)
			{
				// Only react if we're in the list area
				if (!(point_in_element(&info->lister->list_area, msg->MouseX, msg->MouseY)))
					ok = 0;
			}

			// Try popup
			if (!ok || !(backdrop_popup(info, msg->MouseX, msg->MouseY, msg->Qualifier, BPOPF_ICONS, 0, 0)))
			{
				// Not handled
				handled = 0;
			}
		}

		// Something else
		else
			handled = 0;
	}

	// Not handled
	else
		handled = 0;

	// Fix menus if needed
	if (fix_menus && info->lister)
		lister_fix_menus(info->lister, 1);

	// Release lock
	FreeSemaphore(&info->idcmp_lock);

	return handled;
}
