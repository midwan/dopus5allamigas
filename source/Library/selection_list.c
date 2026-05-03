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

#include <stdlib.h>

#include "dopuslib.h"
#include "layout_routines.h"

enum {
	GAD_SELECTION_LISTER,
	GAD_SELECTION_OKAY,
	GAD_SELECTION_CANCEL,
	GAD_SELECTION_DIR_INFO,
	GAD_SELECTION_DIR_FIELD,
	GAD_SELECTION_SWITCH_AREA,
	GAD_SELECTION_SWITCH_BASE,
};

const static ConfigWindow _selection_list_window = {{POS_CENTER, POS_CENTER, 42, 10}, {0, 0, 30, 20}};

const static struct TagItem _selection_lister_tags[] = {{GTLV_ShowSelected, 0}, {TAG_END}},

							_selection_dirfield_tags[] = {{GTST_MaxChars, 255}, {TAG_END}},

							_selection_dirinfo_tags[] = {{GTCustom_Control, GAD_SELECTION_DIR_FIELD}, {TAG_END}},

							_selection_switch_tags[] = {{GTCustom_LayoutRel, GAD_SELECTION_SWITCH_AREA},
														{GTCustom_CopyTags, TRUE},
														{TAG_END}};

const static ObjectDef _selection_list_objects[] =
	{

		// Lister
		{OD_GADGET,
		 MY_LISTVIEW_KIND,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {3, 3, -3, -12},
		 0,
		 LISTVIEWFLAG_CURSOR_KEYS,
		 GAD_SELECTION_LISTER,
		 _selection_lister_tags},

		// Okay
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 12, 1},
		 {3, -3, 4, 6},
		 0,
		 BUTTONFLAG_OKAY_BUTTON | TEXTFLAG_TEXT_STRING,
		 GAD_SELECTION_OKAY,
		 0},

		// Cancel
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
		 {-3, -3, 4, 6},
		 0,
		 BUTTONFLAG_CANCEL_BUTTON | TEXTFLAG_TEXT_STRING,
		 GAD_SELECTION_CANCEL,
		 0},

		{OD_END},

		// Dir field info box
		{OD_GADGET,
		 DIR_BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY - 1, 0, 1},
		 {3, -11, 28, 6},
		 0,
		 0,
		 GAD_SELECTION_DIR_INFO,
		 _selection_dirinfo_tags},

		// Blank space
		{OD_AREA, 0, {0, POS_RIGHT_JUSTIFY - 1, SIZE_MAXIMUM, 1}, {31, -11, -3, 6}, 0, AREAFLAG_ERASE, 0, 0},

		// Dir field
		{OD_GADGET,
		 STRING_KIND,
		 {0, POS_RIGHT_JUSTIFY - 1, SIZE_MAXIMUM, 1},
		 {31, -11, -3, 6},
		 0,
		 0,
		 GAD_SELECTION_DIR_FIELD,
		 _selection_dirfield_tags},

		{OD_END}},

					   _switch_area_template = {OD_AREA,
												0,
												{0, POS_RIGHT_JUSTIFY - 1, SIZE_MAXIMUM, 0},
												{3, -11, -3, 8},
												0,
												AREAFLAG_ERASE | AREAFLAG_RAISED,
												GAD_SELECTION_SWITCH_AREA,
												0},

					   _switch_template = {OD_GADGET,
										   CHECKBOX_KIND,
										   {2, 0, 0, 1},
										   {3, 0, 26, 4},
										   0,
										   TEXTFLAG_TEXT_STRING,
										   0,
										   _selection_switch_tags};

// Put up a list to pick things from
short LIBFUNC L_SelectionList(REG(a0, Att_List *list),
							  REG(a1, struct Window *parent),
							  REG(a2, struct Screen *screen),
							  REG(a3, char *title),
							  REG(d0, short selection),
							  REG(d1, ULONG flags),
							  REG(d2, char *buffer),
							  REG(d3, char *okay_txt),
							  REG(d4, char *cancel_txt),
                              REG(d5, char **switch_txt),
                              REG(d6, ULONG *switch_flags),
							  REG(a6, struct MyLibrary *lib))
{
	ConfigWindow dims, mindims;
	NewConfigWindow newwin;
	struct Window *window;
	ObjectList *objlist;
	ObjectDef *objects;
	short okay = 0;
	ULONG seconds = 0, micros = 0;
	struct MsgPort *appport = 0;
	struct AppWindow *appwindow = 0;
	short switch_count = 0;
	struct IBox pos;
	short fontsize;
	char *size_name = "dopus/windows/selection", namebuf[80];

#ifdef __amigaos4__
	lib = dopuslibbase_global;
#endif

	// Get dimensions
	dims = _selection_list_window;
	if (flags & SLF_DIR_FIELD)
		dims.fine_dim.Height += 6;

	// Switches supplied?
	if (flags & SLF_SWITCHES && switch_txt)
	{
		// Count switches
		for (switch_count = 0; switch_txt[switch_count]; switch_count++)
			;

		// Add extra lines for switches
		dims.char_dim.Height += switch_count;
		dims.fine_dim.Height += (6 * switch_count) + 12;

		// Different name
		lsprintf(namebuf, "dopus/windows/selection%ld", switch_count);
		size_name = namebuf;
	}

	// Save minimum size
	mindims = dims;

	// Get saved position (we only use the size bit)
	if (L_LoadPos(size_name, &pos, &fontsize))
	{
		short fsize = 0;
		struct Screen *pub;

		// Get screen font size
		if (parent)
			fsize = parent->WScreen->RastPort.TxHeight;
		else if (screen)
			fsize = screen->RastPort.TxHeight;
		else if ((pub = LockPubScreen(0)))
		{
			fsize = pub->RastPort.TxHeight;
			UnlockPubScreen(0, pub);
		}

		// Use saved size if font is the same
		if (fsize == fontsize)
		{
			dims.char_dim.Width = 0;
			dims.char_dim.Height = 0;
			dims.fine_dim.Width = pos.Width;
			dims.fine_dim.Height = pos.Height;
		}
	}

	// Fill out new window
	newwin.parent = (parent) ? (void *)parent : (void *)screen;
	newwin.dims = &dims;
	newwin.title = title;
	newwin.locale = 0;
	newwin.port = 0;
	newwin.flags = WINDOW_VISITOR | WINDOW_AUTO_KEYS | WINDOW_REQ_FILL | WINDOW_SIZE_BOTTOM;
	if (!parent)
		newwin.flags |= WINDOW_SCREEN_PARENT;
	newwin.font = 0;

	// Allocate object copy
	if (!(objects = AllocVec(sizeof(_selection_list_objects) + (sizeof(ObjectDef) * (switch_count + 1)), 0)))
		return -1;
	CopyMem((char *)_selection_list_objects, (char *)objects, sizeof(_selection_list_objects));

	// Dir field?
	if (flags & SLF_DIR_FIELD)
	{
		// Shrink lister a line
		objects[0].char_dims.Height = SIZE_MAX_LESS - 2;
		objects[0].fine_dims.Height = -19;

		// Change first end to a skip
		objects[3].type = OD_SKIP;

		// File field?
		if (flags & SLF_FILE_FIELD)
			objects[4].object_kind = FILE_BUTTON_KIND;
	}

	// Switches?
	if (switch_count > 0)
	{
		short num;

		// Fix relative position of other objects
		objects[0].char_dims.Height -= switch_count;
		objects[0].fine_dims.Height -= 8 + (switch_count * 6);
		objects[4].char_dims.Top -= switch_count;
		objects[4].fine_dims.Top -= 8 + (switch_count * 6);
		objects[5].char_dims.Top -= switch_count;
		objects[5].fine_dims.Top -= 8 + (switch_count * 6);
		objects[6].char_dims.Top -= switch_count;
		objects[6].fine_dims.Top -= 8 + (switch_count * 6);

		// Initialise area for switches
		CopyMem((char *)&_switch_area_template, (char *)&objects[7], sizeof(ObjectDef));
		objects[7].char_dims.Height = switch_count;
		objects[7].fine_dims.Height = 6 + (switch_count * 6);

		// Initialise switches
		for (num = 0; num < switch_count; num++)
		{
			CopyMem((char *)&_switch_template, (char *)&objects[8 + num], sizeof(ObjectDef));
			objects[8 + num].char_dims.Top = num;
			objects[8 + num].fine_dims.Top = 3 + (num * 6);
			objects[8 + num].gadget_text = (ULONG)switch_txt[num];
			objects[8 + num].gadgetid = GAD_SELECTION_SWITCH_BASE + num;
		}

		// End gadget
		objects[8 + num].type = OD_END;
	}

	// Fill in strings
	objects[GAD_SELECTION_OKAY].gadget_text = (ULONG)okay_txt;
	objects[GAD_SELECTION_CANCEL].gadget_text = (ULONG)cancel_txt;

	// Open requester
	if (!(window = L_OpenConfigWindow(&newwin, lib)) || !(objlist = L_AddObjectList(window, objects, lib)))
	{
		L_CloseConfigWindow(window, lib);
		FreeVec(objects);
		return -1;
	}

	// Free objects
	FreeVec(objects);

	// Set minimum size
	L_SetConfigWindowLimits(window, &mindims, 0);

	// If list is empty, disable lister
	if (!list || IsListEmpty((struct List *)list))
		L_DisableObject(objlist, GAD_SELECTION_LISTER, TRUE);

	// Otherwise
	else
	{
		// Add list
		L_SetGadgetChoices(objlist, GAD_SELECTION_LISTER, (APTR)list);

		// Initial selection
		if (selection != -1)
			L_SetGadgetValue(objlist, GAD_SELECTION_LISTER, selection);
	}

	// Initialise switches
	if (switch_count > 0 && switch_flags)
	{
		short num;
		for (num = 0; num < switch_count; num++)
			if ((*switch_flags) & (1 << num))
				L_SetGadgetValue(objlist, GAD_SELECTION_SWITCH_BASE + num, TRUE);
	}

	// If dir field, make an AppWindow
	if (flags & SLF_DIR_FIELD)
	{
		if ((appport = CreateMsgPort()))
			appwindow = AddAppWindowA(0, 0, window, appport, 0);

		// Use initial field
		if (flags & SLF_USE_INITIAL)
		{
			// Set buffer value
			if (buffer)
				L_SetGadgetValue(objlist, GAD_SELECTION_DIR_FIELD, (ULONG)buffer);
		}
	}

	// Event loop
	FOREVER
	{
		struct IntuiMessage *msg;
		short break_flag = 0;

		// AppMessage?
		if (appport)
		{
			struct AppMessage *amsg;

			// Get messages
			while ((amsg = (struct AppMessage *)GetMsg(appport)))
			{
				// Arguments?
				if (amsg->am_NumArgs > 0)
				{
					char buf[256];

					// Get path from first argument
					L_DevNameFromLockDopus(amsg->am_ArgList[0].wa_Lock, buf, 256, lib);
					AddPart(buf, (flags & SLF_FILE_FIELD) ? (STRPTR)amsg->am_ArgList[0].wa_Name : (STRPTR) "", 256);
					L_SetGadgetValue(objlist, GAD_SELECTION_DIR_FIELD, (ULONG)buf);
				}
				ReplyMsg((struct Message *)amsg);
			}
		}

		// Window messages?
		while ((msg = L_GetWindowMsg(window->UserPort, lib)))
		{
			// Gadget?
			if (msg->Class == IDCMP_GADGETUP)
			{
				switch (((struct Gadget *)msg->IAddress)->GadgetID)
				{
				// Okay/Cancel
				case GAD_SELECTION_OKAY:
					okay = 1;
				case GAD_SELECTION_CANCEL:
					break_flag = 1;
					break;

				// Path field
				case GAD_SELECTION_DIR_INFO:
				case GAD_SELECTION_DIR_FIELD:

					// Return on path change?
					if (flags & SLF_RETURN_PATH)
					{
						// See if path has changed
						if (stricmp(buffer, (char *)L_GetGadgetValue(objlist, GAD_SELECTION_DIR_FIELD, lib)) != 0)
						{
							okay = 1;
							break_flag = 1;
							selection = -1;
							break;
						}
					}
					break;

				// Selection
				case GAD_SELECTION_LISTER:

					// Test double click
					if (selection == msg->Code && DoubleClick(seconds, micros, msg->Seconds, msg->Micros))
					{
						okay = 1;
						break_flag = 1;
						break;
					}

					// Store selection and time for doubleclick
					selection = msg->Code;
					seconds = msg->Seconds;
					micros = msg->Micros;
					break;
				}
			}

			// Key press
			else if (msg->Class == IDCMP_VANILLAKEY)
			{
				Att_Node *node;
				char key;
				short offset;

				// Get key press
				key = tolower(msg->Code);

				// Search list for something starting with this key
				for (node = (Att_Node *)list->list.lh_Head, offset = 0; node->node.ln_Succ;
					 node = (Att_Node *)node->node.ln_Succ, offset++)
				{
					// Does first letter match?
					if (tolower(node->node.ln_Name[0]) == key)
					{
						// Position at this node
						L_SetGadgetValue(objlist, GAD_SELECTION_LISTER, offset);

						// Store selection
						selection = offset;
						break;
					}
				}
			}

			// Close window
			else if (msg->Class == IDCMP_CLOSEWINDOW)
				break_flag = 1;

			// Reply to message
			L_ReplyWindowMsg(msg);
		}

		// Check break flag
		if (break_flag)
			break;

		// Wait for event
		if (Wait(1 << window->UserPort->mp_SigBit | ((appport) ? (1 << appport->mp_SigBit) : 0) | IPCSIG_QUIT) &
			IPCSIG_QUIT)
		{
			selection = -1;
			break;
		}
	}

	// Dir field?
	if (flags & SLF_DIR_FIELD)
	{
		// Get buffer value
		if (buffer)
			strcpy(buffer, (char *)L_GetGadgetValue(objlist, GAD_SELECTION_DIR_FIELD, lib));
	}

	// Kill AppWindow
	if (appwindow)
		RemoveAppWindow(appwindow);
	if (appport)
		DeleteMsgPort(appport);

	// Read switch values
	if (switch_count > 0 && switch_flags && okay)
	{
		short num;
		*switch_flags = 0;
		for (num = 0; num < switch_count; num++)
			if (L_GetGadgetValue(objlist, GAD_SELECTION_SWITCH_BASE + num, lib))
				(*switch_flags) |= 1 << num;
	}

	// Save position
	if (window)
	{
		struct IBox pos;
		pos.Left = window->LeftEdge;
		pos.Top = window->TopEdge;
		pos.Width = window->Width - window->BorderLeft - window->BorderRight;
		pos.Height = window->Height - window->BorderTop - window->BorderBottom;
		L_SavePos(size_name, (struct IBox *)&pos, window->RPort->TxHeight);
	}

	// Close window
	L_CloseConfigWindow(window, lib);

	return (short)((okay) ? selection : -2);
}
