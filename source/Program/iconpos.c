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
#include "iconpos.h"

void iconpos_configure(IPCData *ipc, struct MsgPort *port)
{
	iconpos_info *info;
	struct Node *info_ptr, *next;
	Cfg_Desktop *desk;
	short ret;

	// Initialise icon position list
	lock_listlock(&GUI->iconpos_list, 0);
	NewList((struct List *)&GUI->iconpos_list);

	// Lock desktop list
	GetSemaphore(&environment->desktop_lock, SEMF_SHARED, 0);

	// Go through desktop list
	for (desk = (Cfg_Desktop *)environment->desktop.mlh_Head; desk->node.mln_Succ;
		 desk = (Cfg_Desktop *)desk->node.mln_Succ)
	{
		// Icon position?
		if (desk->data.dt_Type == DESKTOP_ICONPOS)
		{
			// Allocate structure
			if ((info = AllocVec(sizeof(iconpos_info), MEMF_CLEAR)))
			{
				// Initialise
				info->ip_Data.ip_Flags = desk->data.dt_Flags;
				info->ip_Data.ip_Area = *((struct IBox *)&desk->data.dt_Data);
				info->ip_Data.ip_Area.Left += GUI->window->LeftEdge;
				info->ip_Data.ip_Area.Top += GUI->window->TopEdge;

				// Open window
				iconpos_openwindow(info, port);

				// Add to list
				AddTail((struct List *)&GUI->iconpos_list, &info->ip_Data.ip_Node);
			}
		}
	}

	// Unlock lists
	unlock_listlock(&GUI->iconpos_list);
	FreeSemaphore(&environment->desktop_lock);

	// Set flag to say we're positioning
	GUI->flags2 |= GUIF2_ICONPOS;

	// Display requester
	ret = super_request_args(GUI->screen_pointer,
							 GetString(&locale, MSG_ICONPOS_INSTRUCTIONS),
							 SRF_SCREEN_PARENT | SRF_IPC,
							 ipc,
							 GetString(&locale, MSG_OKAY),
							 GetString(&locale, MSG_CANCEL),
							 0);

	// Clear positioning flag
	GUI->flags2 &= ~GUIF2_ICONPOS;

	// Lock position list
	lock_listlock(&GUI->iconpos_list, 0);

	// User pressed ok?
	if (ret == 1)
	{
		Cfg_Desktop *next;

		// Lock desktop list
		GetSemaphore(&environment->desktop_lock, SEMF_SHARED, 0);

		// Go through desktop list, free icon positions
		for (desk = (Cfg_Desktop *)environment->desktop.mlh_Head; desk->node.mln_Succ; desk = next)
		{
			// Cache next
			next = (Cfg_Desktop *)desk->node.mln_Succ;

			// Icon position?
			if (desk->data.dt_Type == DESKTOP_ICONPOS)
			{
				// Remove and free it
				Remove((struct Node *)desk);
				FreeMemH(desk);
			}
		}

		// Go through new list
		for (info_ptr = GUI->iconpos_list.list.lh_Head; info_ptr->ln_Succ; info_ptr = info_ptr->ln_Succ)
		{
			// Get info pointer
			info = (iconpos_info *)(((char *)info_ptr) - sizeof(WindowID));

			// Allocate new desktop entry
			if ((desk = AllocMemH(environment->desktop_memory, sizeof(Cfg_Desktop) + sizeof(ULONG))))
			{
				// Update window position
				if (info->ip_Data.ip_Window)
				{
					// Update position
					info->ip_Data.ip_Area.Left = info->ip_Data.ip_Window->LeftEdge;
					info->ip_Data.ip_Area.Top = info->ip_Data.ip_Window->TopEdge;
					info->ip_Data.ip_Area.Width = info->ip_Data.ip_Window->Width;
					info->ip_Data.ip_Area.Height = info->ip_Data.ip_Window->Height;
				}

				// Initialise it
				desk->data.dt_Type = DESKTOP_ICONPOS;
				desk->data.dt_Flags = info->ip_Data.ip_Flags;
				desk->data.dt_Size = sizeof(CFG_DESK) + sizeof(ULONG);

				// Convert window position to main window-relative
				info->ip_Data.ip_Area.Left -= GUI->window->LeftEdge;
				info->ip_Data.ip_Area.Top -= GUI->window->TopEdge;

				// Bounds-check
				if (info->ip_Data.ip_Area.Left < 0)
					info->ip_Data.ip_Area.Left = 0;
				if (info->ip_Data.ip_Area.Top < 0)
					info->ip_Data.ip_Area.Top = 0;

				// Store window position
				*((struct IBox *)&desk->data.dt_Data) = info->ip_Data.ip_Area;

				// Add to list
				AddTail((struct List *)&environment->desktop, (struct Node *)desk);
			}
		}

		// Unlock desktop list
		FreeSemaphore(&environment->desktop_lock);
	}

	// Go through icon pos list
	for (info_ptr = GUI->iconpos_list.list.lh_Head; info_ptr->ln_Succ; info_ptr = next)
	{
		// Cache next
		next = info_ptr->ln_Succ;

		// Get info pointer
		info = (iconpos_info *)(((char *)info_ptr) - sizeof(WindowID));

		// Close window
		iconpos_closewindow(info);

		// Free entry
		FreeVec(info);
	}

	// Unlock list
	unlock_listlock(&GUI->iconpos_list);
}

// Add a new area
void iconpos_new_area(struct Rectangle *rect)
{
	iconpos_info *info;

	// Lock position list
	lock_listlock(&GUI->iconpos_list, TRUE);

	// Allocate new position info structure
	if ((info = AllocVec(sizeof(iconpos_info), MEMF_CLEAR)))
	{
		// Initialise
		info->ip_Data.ip_Flags =
			ICONPOSF_APPICONS | ICONPOSF_DISKS | ICONPOSF_LISTERS | ICONPOSF_GROUPS | ICONPOSF_LEFTOUTS;
		info->ip_Data.ip_Area.Left = rect->MinX;
		info->ip_Data.ip_Area.Top = rect->MinY;
		info->ip_Data.ip_Area.Width = RECTWIDTH(rect);
		info->ip_Data.ip_Area.Height = RECTHEIGHT(rect);

		// Minimum size
		if (info->ip_Data.ip_Area.Width < 40)
			info->ip_Data.ip_Area.Width = 40;
		if (info->ip_Data.ip_Area.Height < 40)
			info->ip_Data.ip_Area.Height = 40;

		// Open window
		iconpos_openwindow(info, GUI->iconpos_port);

		// Add to list
		AddTail((struct List *)&GUI->iconpos_list, &info->ip_Data.ip_Node);
	}

	// Unlock list
	unlock_listlock(&GUI->iconpos_list);
}

// Open window
void iconpos_openwindow(iconpos_info *info, struct MsgPort *port)
{
	PopUpItem *item;

	// Get menu handle
	if (!(info->ip_Data.ip_Menu = PopUpNewHandle((IPTR)info, 0, &locale)))
		return;

	// Build menu
	if ((info->ip_Data.ip_Item[IPITEM_APPICON] =
			 PopUpNewItem(info->ip_Data.ip_Menu, MSG_ICONPOS_APPICON, MENU_ICONPOS_APPICON, POPUPF_CHECKIT)) &&
		info->ip_Data.ip_Flags & ICONPOSF_APPICONS)
		info->ip_Data.ip_Item[IPITEM_APPICON]->flags |= POPUPF_CHECKED;
	if ((info->ip_Data.ip_Item[IPITEM_DISKS] =
			 PopUpNewItem(info->ip_Data.ip_Menu, MSG_ICONPOS_DISKS, MENU_ICONPOS_DISKS, POPUPF_CHECKIT)) &&
		info->ip_Data.ip_Flags & ICONPOSF_DISKS)
		info->ip_Data.ip_Item[IPITEM_DISKS]->flags |= POPUPF_CHECKED;
	if ((info->ip_Data.ip_Item[IPITEM_LISTERS] =
			 PopUpNewItem(info->ip_Data.ip_Menu, MSG_ICONPOS_LISTERS, MENU_ICONPOS_LISTERS, POPUPF_CHECKIT)) &&
		info->ip_Data.ip_Flags & ICONPOSF_LISTERS)
		info->ip_Data.ip_Item[IPITEM_LISTERS]->flags |= POPUPF_CHECKED;
	if ((info->ip_Data.ip_Item[IPITEM_GROUPS] =
			 PopUpNewItem(info->ip_Data.ip_Menu, MSG_ICONPOS_GROUPS, MENU_ICONPOS_GROUPS, POPUPF_CHECKIT)) &&
		info->ip_Data.ip_Flags & ICONPOSF_GROUPS)
		info->ip_Data.ip_Item[IPITEM_GROUPS]->flags |= POPUPF_CHECKED;
	if ((info->ip_Data.ip_Item[IPITEM_LEFTOUT] =
			 PopUpNewItem(info->ip_Data.ip_Menu, MSG_ICONPOS_LEFTOUT, MENU_ICONPOS_LEFTOUT, POPUPF_CHECKIT)) &&
		info->ip_Data.ip_Flags & ICONPOSF_LEFTOUTS)
		info->ip_Data.ip_Item[IPITEM_LEFTOUT]->flags |= POPUPF_CHECKED;

	PopUpSeparator(info->ip_Data.ip_Menu);
	item = PopUpNewItem(info->ip_Data.ip_Menu, MSG_ICONPOS_PRIORITY, 0, POPUPF_SUB);

	// Got priority item? Allocate list
	if (item && (item->data = AllocMemH(info->ip_Data.ip_Menu->ph_Memory, sizeof(PopUpMenu))))
	{
		short a;
		short pri;

		// Get priority
		pri = ICONPOSF_PRIORITY(info->ip_Data.ip_Flags);
		if (pri > 127)
			pri -= 256;

		// Initialise list
		NewList((struct List *)item->data);

		// Set list pointer
		info->ip_Data.ip_Menu->ph_List = (struct List *)item->data;

		// Five pre-set priorities
		for (a = 0; a < 5; a++)
		{
			// Build string
			lsprintf(info->ip_Data.ip_Buf[a], "%ld", (a * 5) - 10);

			// Create item
			if ((info->ip_Data.ip_Item[IPITEM_PRIORITY + a] = PopUpNewItem(info->ip_Data.ip_Menu,
																		   (IPTR)info->ip_Data.ip_Buf[a],
																		   MENU_ICONPOS_PRIORITY0 + a,
																		   POPUPF_STRING | POPUPF_CHECKIT)))
			{
				// Selected?
				if (pri == (a * 5) - 10)
					info->ip_Data.ip_Item[IPITEM_PRIORITY + a]->flags |= POPUPF_CHECKED;
			}
		}
	}

	// Open window
	if ((info->ip_Data.ip_Window = OpenWindowTags(0,
												  WA_Left,
												  info->ip_Data.ip_Area.Left,
												  WA_Top,
												  info->ip_Data.ip_Area.Top,
												  WA_Width,
												  info->ip_Data.ip_Area.Width,
												  WA_Height,
												  info->ip_Data.ip_Area.Height,
												  WA_Title,
												  GetString(&locale, MSG_ICONPOS_WINDOW),
												  WA_AutoAdjust,
												  TRUE,
												  WA_CloseGadget,
												  TRUE,
												  WA_DragBar,
												  TRUE,
												  WA_DepthGadget,
												  TRUE,
												  WA_RMBTrap,
												  TRUE,
												  WA_SizeGadget,
												  TRUE,
												  WA_CustomScreen,
												  GUI->screen_pointer,
												  WA_MinWidth,
												  80,
												  WA_MinHeight,
												  60,
												  WA_MaxWidth,
												  (ULONG)~0,
												  WA_MaxHeight,
												  (ULONG)~0,
												  WA_SimpleRefresh,
												  TRUE,
												  WA_NoCareRefresh,
												  TRUE,
												  WA_WindowName,
												  DOPUS_WIN_NAME,
												  TAG_END)))
	{
		// Install message port
		info->ip_Data.ip_Window->UserPort = port;

		// Set IDCMP flags
		ModifyIDCMP(info->ip_Data.ip_Window, IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS);

		// Save info pointer in userdata
		info->ip_Data.ip_Window->UserData = (void *)info;

		// Set window ID
		SetWindowID(info->ip_Data.ip_Window, 0, WINDOW_ICONPOS, 0);
	}
}

// Close window
void iconpos_closewindow(iconpos_info *info)
{
	// Valid window?
	if (info->ip_Data.ip_Window)
	{
		// Close window (safely with shared IDCMP)
		CloseWindowSafely(info->ip_Data.ip_Window);
		info->ip_Data.ip_Window = 0;

		// Free popup menu
		PopUpFreeHandle(info->ip_Data.ip_Menu);
		info->ip_Data.ip_Menu = 0;
	}
}

// Handle IDCMP
BOOL iconpos_idcmp(struct IntuiMessage *msg)
{
	iconpos_info *info;
	short res;

	// Get info pointer
	info = (iconpos_info *)msg->IDCMPWindow->UserData;

	// Look at message code
	switch (msg->Class)
	{
	// Mouse button for pop-up menu
	case IDCMP_MOUSEBUTTONS:

		// Only use right button
		if (msg->Code != MENUDOWN)
			break;

		// Do pop-up
		if ((res = DoPopUpMenu(msg->IDCMPWindow, &info->ip_Data.ip_Menu->ph_Menu, 0, msg->Code)) == -1)
			break;

		// Priority selection?
		if (res >= MENU_ICONPOS_PRIORITY0 && res <= MENU_ICONPOS_PRIORITY4)
		{
			short pri, a;

			// Get priority
			res -= MENU_ICONPOS_PRIORITY0;
			pri = (res * 5) - 10;

			// Store priority
			info->ip_Data.ip_Flags = ICONPOSF_CLEARPRI(info->ip_Data.ip_Flags);
			info->ip_Data.ip_Flags |= ICONPOSF_MAKEPRI(pri);

			// Fix item selection marks
			for (a = 0; a < 5; a++)
				if (info->ip_Data.ip_Item[IPITEM_PRIORITY + a])
				{
					// Selected?
					if (res == a)
						info->ip_Data.ip_Item[IPITEM_PRIORITY + a]->flags |= POPUPF_CHECKED;

					// Not selected
					else
						info->ip_Data.ip_Item[IPITEM_PRIORITY + a]->flags &= ~POPUPF_CHECKED;
				}
		}

		// Look at selection
		else
			switch (res)
			{
			// AppIcon
			case MENU_ICONPOS_APPICON:
				info->ip_Data.ip_Flags ^= ICONPOSF_APPICONS;
				break;

			// Disks
			case MENU_ICONPOS_DISKS:
				info->ip_Data.ip_Flags ^= ICONPOSF_DISKS;
				break;

			// Listers
			case MENU_ICONPOS_LISTERS:
				info->ip_Data.ip_Flags ^= ICONPOSF_LISTERS;
				break;

			// Groups
			case MENU_ICONPOS_GROUPS:
				info->ip_Data.ip_Flags ^= ICONPOSF_GROUPS;
				break;

			// Leftouts
			case MENU_ICONPOS_LEFTOUT:
				info->ip_Data.ip_Flags ^= ICONPOSF_LEFTOUTS;
				break;
			}
		break;

	// Close
	case IDCMP_CLOSEWINDOW:

		// Reply to the message
		ReplyMsg((struct Message *)msg);

		// Close the window
		iconpos_closewindow(info);

		// Lock list
		lock_listlock(&GUI->iconpos_list, 1);

		// Remove node and free it
		Remove(&info->ip_Data.ip_Node);
		FreeVec(info);

		// Unlock list
		unlock_listlock(&GUI->iconpos_list);
		return 0;
	}

	return 1;
}
