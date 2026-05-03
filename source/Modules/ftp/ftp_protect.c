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

#include <proto/dopus5.h>
#include <proto/configopus.h>
#include <proto/module.h>

#include "ftp.h"
#include "ftp_opusftp.h"
#include "ftp_ipc.h"
#include "ftp_recursive.h"
#include "ftp_util.h"
#include "ftp_protect.h"

#define SetFlag(v, f) ((v) |= (f))
#define ClearFlag(v, f) ((v) &= ~(f))
#define ToggleFlag(v, f) ((v) ^= (f))
#define FlagIsSet(v, f) (((v) & (f)) != 0)
#define FlagIsClear(v, f) (((v) & (f)) == 0)

enum {
	GAD_PROTECT_LAYOUT,
	GAD_PROTECT_FILENAME,
	GAD_PROTECT_OK,
	GAD_PROTECT_ALL,
	GAD_PROTECT_SKIP,
	GAD_PROTECT_ABORT,
	GAD_PROTECT_OLD = 10,
	GAD_PROTECT_SET = 20,
	GAD_PROTECT_CLEAR = 30,
	GAD_PROTECT_OLD_UNIX = 40,
	GAD_PROTECT_SET_UNIX = 50,
	GAD_PROTECT_CLEAR_UNIX = 60,

};

// Protection bits
ConfigWindow _protect_window = {{POS_CENTER, POS_CENTER, 38, 5 * 2 + 1}, {0, 0, 54, 58 + 16}};

struct TagItem _protect_layout[] = {{GTCustom_LayoutRel, GAD_PROTECT_LAYOUT}, {TAG_DONE}};

ObjectDef
	_protect_button_template =
		{OD_GADGET, BUTTON_KIND, {0, 0, 2, 1}, {0, 0, 4, 6}, 0, BUTTONFLAG_TOGGLE_SELECT, 0, _protect_layout},

	_protect_objects[] = {

		// Layout
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {3, 3, -3, -13},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_PROTECT_LAYOUT,
		 0},

		// Filename
		{OD_AREA,
		 TEXTPEN,
		 {2, 0, SIZE_MAX_LESS - 2, 1},
		 {5, 5, -5, 4},
		 0,
		 AREAFLAG_RECESSED | TEXTFLAG_CENTER,
		 GAD_PROTECT_FILENAME,
		 _protect_layout},

		// Old
		{OD_TEXT,
		 TEXTPEN,
		 {10, 1, 0, 1},
		 {5, 17, 0, 0},
		 MSG_SET_PROTECT_OLD,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

		// Set
		{OD_TEXT,
		 TEXTPEN,
		 {10, 2, 0, 1},
		 {5, 25, 0, 0},
		 MSG_SET_PROTECT_SET,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

		// Clear
		{OD_TEXT,
		 TEXTPEN,
		 {10, 3, 0, 1},
		 {5, 33, 0, 0},
		 MSG_SET_PROTECT_CLEAR,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

#define UNIX_Y_OFF (16 + 4)

		// Old
		{OD_TEXT,
		 TEXTPEN,
		 {10, 6, 0, 1},
		 {5, 17 + UNIX_Y_OFF, 0, 0},
		 MSG_SET_PROTECT_OLD,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

		// Set
		{OD_TEXT,
		 TEXTPEN,
		 {10, 7, 0, 1},
		 {5, 25 + UNIX_Y_OFF, 0, 0},
		 MSG_SET_PROTECT_SET,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

		// Clear
		{OD_TEXT,
		 TEXTPEN,
		 {10, 8, 0, 1},
		 {5, 33 + UNIX_Y_OFF, 0, 0},
		 MSG_SET_PROTECT_CLEAR,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

		// Ok
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 8, 1},
		 {3, -3, 12, 6},
		 MSG_OK_BUTTON,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_PROTECT_OK,
		 0},

		// All
		{OD_GADGET, BUTTON_KIND, {10, POS_RIGHT_JUSTIFY, 8, 1}, {15, -3, 12, 6}, MSG_ALL_BUTTON, 0, GAD_PROTECT_ALL, 0},

		// Skip
		{OD_GADGET,
		 BUTTON_KIND,
		 {20, POS_RIGHT_JUSTIFY, 8, 1},
		 {27, -3, 12, 6},
		 MSG_SKIP_BUTTON,
		 0,
		 GAD_PROTECT_SKIP,
		 0},

		// Abort
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 8, 1},
		 {-3, -3, 12, 6},
		 MSG_ABORT_BUTTON,
		 BUTTONFLAG_CANCEL_BUTTON,
		 GAD_PROTECT_ABORT,
		 0},

		{OD_END}};

static void read_gadgets(ObjectList *objlist,
						 BOOL unix_type,
						 ULONG *set,
						 ULONG *clear,
						 ULONG *clear_unix,
						 ULONG *set_unix)
{
	int obj;

	if (unix_type)
	{
		*set_unix = *clear_unix = 0;

		for (obj = 0; obj < 9; obj++)
		{
			if (GetGadgetValue(objlist, GAD_PROTECT_SET_UNIX + 8 - obj))
				*set_unix |= 1 << obj;
			else if (GetGadgetValue(objlist, GAD_PROTECT_CLEAR_UNIX + 8 - obj))
				*clear_unix |= 1 << obj;
		}

		*set = prot_unix_to_amiga(*set_unix);
		*clear = prot_unix_to_amiga(*clear_unix);
	}
	else
	{
		*set = *clear = 0;

		for (obj = 0; obj < 8; obj++)
		{
			if (GetGadgetValue(objlist, GAD_PROTECT_SET + 7 - obj))
				*set |= 1 << obj;
			else if (GetGadgetValue(objlist, GAD_PROTECT_CLEAR + 7 - obj))
				*clear |= 1 << obj;
		}

		*set_unix = prot_amiga_to_unix(*set);
		*clear_unix = prot_amiga_to_unix(*clear);
	}
}

static void set_gadgets_dos(ObjectList *objlist, short gadgetid, ULONG set, ULONG clear)
{
	int obj;

	for (obj = 0; obj < 8; obj++)
	{
		SetGadgetValue(objlist, GAD_PROTECT_SET + obj, FlagIsSet(set, 1 << (7 - obj)));
		SetGadgetValue(objlist, GAD_PROTECT_CLEAR + obj, FlagIsSet(clear, 1 << (7 - obj)));
	}
}

static void set_gadgets_unix(ObjectList *objlist, short gadgetid, ULONG set, ULONG clear)
{
	int obj;

	for (obj = 0; obj < 9; obj++)
	{
		SetGadgetValue(objlist, GAD_PROTECT_SET_UNIX + obj, FlagIsSet(set, 1 << (8 - obj)));
		SetGadgetValue(objlist, GAD_PROTECT_CLEAR_UNIX + obj, FlagIsSet(clear, 1 << (8 - obj)));
	}
}

static void bcheck(ObjectList *objlist, short gadgetid)
{
	ULONG set_mask, clear_mask;
	ULONG set_mask_unix, clear_mask_unix;

	if (gadgetid >= GAD_PROTECT_SET_UNIX)
	{
		read_gadgets(objlist, TRUE, &set_mask, &clear_mask, &clear_mask_unix, &set_mask_unix);

		set_mask ^= 0x0f;
		clear_mask ^= 0x0f;

		set_gadgets_dos(objlist, gadgetid, set_mask, clear_mask);
	}
	else
	{
		read_gadgets(objlist, FALSE, &set_mask, &clear_mask, &clear_mask_unix, &set_mask_unix);

		set_mask_unix ^= 0777;
		clear_mask_unix ^= 0777;

		// Check we aren't trying to set and clear the same bit
		if (gadgetid == GAD_PROTECT_SET + 5 || gadgetid == GAD_PROTECT_SET + 7)
			clear_mask_unix &= ~0222;
		else if (gadgetid == GAD_PROTECT_CLEAR + 5 || gadgetid == GAD_PROTECT_CLEAR + 7)
			set_mask_unix &= ~0222;

		set_gadgets_unix(objlist, gadgetid, set_mask_unix, clear_mask_unix);
	}
}

// Get protection
int function_change_get_protect(struct opusftp_globals *og, struct protectgui_msg *pm)
{
	NewConfigWindow new_win;
	struct Window *window;
	ObjectList *objlist;
	short break_flag = 0, x, y, obj;

	static char *prot_bits = "H\0S\0P\0A\0R\0W\0E\0D";
	static char *prot_bits_unix = "R\0W\0X\0R\0W\0X\0R\0W\0X";

	ObjectDef *objects, *objects_unix;

	ULONG set_mask, clear_mask, old_prot;

	old_prot = prot_unix_to_amiga(pm->pm_current);

	// Fix old protection bits
	old_prot = (old_prot & 0xf0) | ((~old_prot) & 0xf);

	// get dos masks
	set_mask = prot_unix_to_amiga(pm->pm_set_mask);
	set_mask = (set_mask & 0xf0) | ((~set_mask) & 0xf);

	clear_mask = prot_unix_to_amiga(pm->pm_clear_mask);
	clear_mask = (clear_mask & 0xf0) | ((~clear_mask) & 0xf);

	if (pm->pm_window)
		new_win.parent = pm->pm_window;
	else
	{
		new_win.parent = og->og_screen;
		new_win.flags = WINDOW_SCREEN_PARENT;
	}

	new_win.dims = &_protect_window;
	new_win.title = GetString(locale, MSG_SELECT_PROTECTION_BITS);
	new_win.locale = locale;
	new_win.port = 0;
	new_win.flags = WINDOW_NO_CLOSE | WINDOW_VISITOR | WINDOW_AUTO_KEYS | WINDOW_REQ_FILL;
	new_win.font = 0;

	// Allocate objects
	if (!(objects = AllocVec(sizeof(ObjectDef) * 8 * 3 + 1, MEMF_CLEAR)))
		return 0;

	if (!(objects_unix = AllocVec(sizeof(ObjectDef) * 9 * 3 + 1, MEMF_CLEAR)))
	{
		FreeVec(objects);
		return 0;
	}

	// Build objects
	for (y = 0, obj = 0; y < 3; y++)
	{
		for (x = 0; x < 8; x++, obj++)
		{
			// Copy template
			CopyMem((char *)&_protect_button_template, (char *)(objects + obj), sizeof(ObjectDef));

			// Fix coordinates
			objects[obj].char_dims.Left = 12 + x * 2;
			objects[obj].char_dims.Top = y + 1;
			objects[obj].fine_dims.Left = 5 + x * 4;
			objects[obj].fine_dims.Top = 14 + y * 8;

			// Type and flags for old display
			if (y == 0)
			{
				objects[obj].type = OD_AREA;
				objects[obj].object_kind = TEXTPEN;
				objects[obj].flags = AREAFLAG_RECESSED | TEXTFLAG_CENTER;

				// Only has text if bit is set
				if (FlagIsSet(old_prot, 1 << (7 - x)))
					objects[obj].gadget_text = (ULONG)&prot_bits[x * 2];
			}

			// Gadget text
			else
				objects[obj].gadget_text = (ULONG)&prot_bits[x * 2];

			// Flags
			objects[obj].flags |= TEXTFLAG_TEXT_STRING;

			// Gadget id
			objects[obj].gadgetid = GAD_PROTECT_OLD + (y * 10) + x;
		}
	}

	// Last object
	objects[obj].type = OD_END;

	// Build objects
	for (y = 0, obj = 0; y < 3; y++)
	{
		for (x = 0; x < 9; x++, obj++)
		{
			// Copy template
			CopyMem((char *)&_protect_button_template, (char *)(objects_unix + obj), sizeof(ObjectDef));

			// Fix coordinates
			objects_unix[obj].char_dims.Left = 12 + x * 2;
			objects_unix[obj].char_dims.Top = y + 1 + 5;
			objects_unix[obj].fine_dims.Left = 5 + x * 4 - 8;
			objects_unix[obj].fine_dims.Top = 14 + y * 8 + UNIX_Y_OFF;

			// Type and flags for old display
			if (y == 0)
			{
				objects_unix[obj].type = OD_AREA;
				objects_unix[obj].object_kind = TEXTPEN;
				objects_unix[obj].flags = AREAFLAG_RECESSED | TEXTFLAG_CENTER;

				// Only has text if bit is set
				if (FlagIsSet(pm->pm_current, 1 << (8 - x)))
					objects_unix[obj].gadget_text = (ULONG)&prot_bits_unix[x * 2];
			}

			// Gadget text
			else
				objects_unix[obj].gadget_text = (ULONG)&prot_bits_unix[x * 2];

			// Flags
			objects_unix[obj].flags |= TEXTFLAG_TEXT_STRING;

			// Gadget id
			objects_unix[obj].gadgetid = GAD_PROTECT_OLD_UNIX + (y * 10) + x;
		}
	}

	// Last object
	objects_unix[obj].type = OD_END;

	// Open window
	if (!(window = OpenConfigWindow(&new_win)) || !(objlist = AddObjectList(window, _protect_objects)) ||
		!(AddObjectList(window, objects)) || !(AddObjectList(window, objects_unix)))

	{
		CloseConfigWindow(window);
		FreeVec(objects);
		FreeVec(objects_unix);

		return 0;
	}

	// Free object memory
	FreeVec(objects);
	// Free object memory
	FreeVec(objects_unix);

	// Show filename
	DisplayObject(window, GetObject(objlist, GAD_PROTECT_FILENAME), -1, -1, FilePart(pm->pm_name));

	// Set initial unix gadget states
	for (obj = 0; obj < 9; obj++)
	{
		if (pm->pm_set_mask & 1 << obj)
			SetGadgetValue(objlist, GAD_PROTECT_SET_UNIX + 8 - obj, 1);
		else if (pm->pm_clear_mask & 1 << obj)
			SetGadgetValue(objlist, GAD_PROTECT_CLEAR_UNIX + 8 - obj, 1);
	}

	// Set initial dos gadget states
	for (obj = 0; obj < 8; obj++)
	{
		if (set_mask & 1 << obj)
			SetGadgetValue(objlist, GAD_PROTECT_SET + 7 - obj, 1);
		else if (clear_mask & 1 << obj)
			SetGadgetValue(objlist, GAD_PROTECT_CLEAR + 7 - obj, 1);
	}

	// Event loop
	FOREVER
	{
		struct IntuiMessage *msg;

		// Any Intuition messages?
		while ((msg = GetWindowMsg(window->UserPort)))
		{
			struct IntuiMessage copy_msg;
			UWORD gadgetid = 0;

			// Copy message and reply
			copy_msg = *msg;
			ReplyWindowMsg(msg);
			if (copy_msg.Class == IDCMP_GADGETUP)
				gadgetid = ((struct Gadget *)copy_msg.IAddress)->GadgetID;

			// Look at message
			switch (copy_msg.Class)
			{
			// Key press
			case IDCMP_VANILLAKEY:

				// See if this matches a protection bit ONLY FOR DOS
				for (obj = 0; obj < 8; obj++)
				{
					if (toupper(copy_msg.Code) == prot_bits[obj * 2])
					{
						short state;

						// Shift is clear
						if (isupper(copy_msg.Code))
							gadgetid = GAD_PROTECT_CLEAR + obj;
						else
							gadgetid = GAD_PROTECT_SET + obj;

						// Toggle state of gadget
						state = 1 - GetGadgetValue(objlist, gadgetid);
						SetGadgetValue(objlist, gadgetid, state);
						break;
					}
				}

				// Not recognised
				if (obj == 8)
					break;

			// Gadget
			case IDCMP_GADGETUP:

				// Clear unix bit?
				if (gadgetid >= GAD_PROTECT_CLEAR_UNIX)
				{
					// Make sure corresponding set bit is off
					SetGadgetValue(objlist, GAD_PROTECT_SET_UNIX + gadgetid - GAD_PROTECT_CLEAR_UNIX, 0);
					// break;
				}

				// Set bit?
				else if (gadgetid >= GAD_PROTECT_SET_UNIX)
				{
					// Make sure corresponding clear bit is off
					SetGadgetValue(objlist, GAD_PROTECT_CLEAR_UNIX + gadgetid - GAD_PROTECT_SET_UNIX, 0);
					// break;
				}
				// Clear dos bit?
				else if (gadgetid >= GAD_PROTECT_CLEAR)
				{
					// Make sure corresponding set bit is off
					SetGadgetValue(objlist, GAD_PROTECT_SET + gadgetid - GAD_PROTECT_CLEAR, 0);
					// break;
				}

				// Set bit?
				else if (gadgetid >= GAD_PROTECT_SET)
				{
					// Make sure corresponding clear bit is off
					SetGadgetValue(objlist, GAD_PROTECT_CLEAR + gadgetid - GAD_PROTECT_SET, 0);
					// break;
				}

				if (gadgetid >= GAD_PROTECT_SET)
				{
					bcheck(objlist, gadgetid);
					break;
				}

				// Another button
				switch (gadgetid)
				{
				case GAD_PROTECT_OK:
				case GAD_PROTECT_ALL:
				case GAD_PROTECT_SKIP:
				case GAD_PROTECT_ABORT:
					break_flag = gadgetid;
					break;
				}
				break;
			}
		}

		if (break_flag)
			break;

		Wait(1 << window->UserPort->mp_SigBit);
	}

	// Get gadget states
	pm->pm_set_mask = 0;
	pm->pm_clear_mask = 0;

	for (obj = 0; obj < 9; obj++)
	{
		if (GetGadgetValue(objlist, GAD_PROTECT_SET_UNIX + 8 - obj))
			pm->pm_set_mask |= 1 << obj;
		else if (GetGadgetValue(objlist, GAD_PROTECT_CLEAR_UNIX + 8 - obj))
			pm->pm_clear_mask |= 1 << obj;
	}

	// Close window
	CloseConfigWindow(window);

	// Return correct code
	if (break_flag == GAD_PROTECT_ABORT)
		return -1;
	if (break_flag == GAD_PROTECT_SKIP)
		return 0;
	if (break_flag == GAD_PROTECT_ALL)
		return 2;
	return 1;
}
