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

#include "simplerequest.h"

#define TEXT_X_SPACE 16
#define GAD_X_EXTRA 16
#define GAD_X_SPACE 8

short LIBFUNC L_DoSimpleRequest(REG(a0, struct Window *parent),
								REG(a1, struct DOpusSimpleRequest *simple),
								REG(a6, struct Library *libbase))
{
	APTR memory;
	simplereq_data *data;
	UWORD gadgetid = 0;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = (struct Library *)dopuslibbase_global;
#endif

	// Allocate memory handle
	if (!(memory = L_NewMemHandle(0, 0, MEMF_CLEAR)))
		return 1;

	// Allocate data
	if (!(data = (simplereq_data *)L_AllocMemH(memory, sizeof(simplereq_data))))
	{
		L_FreeMemHandle(memory);
		return 1;
	}

	// Initialise
	data->simple = simple;
	data->parent = parent;
	data->string_buffer = simple->string_buffer;
	data->gadgets = simple->gadgets;
	data->libbase = (struct MyLibrary *)libbase;
	data->signal = -1;
	data->memory = memory;

	// Center window; over mouse?
	if (simple->flags & SRF_MOUSE_POS)
	{
		data->req_dims.char_dim.Left = POS_MOUSE_CENTER;
		data->req_dims.char_dim.Top = POS_MOUSE_CENTER;
	}

	// Over parent
	else
	{
		data->req_dims.char_dim.Left = POS_CENTER;
		data->req_dims.char_dim.Top = POS_CENTER;
	}

	// Construct requester
	if (!(simple_build(data)))
	{
		// Failed
		L_FreeMemHandle(memory);
		return 1;
	}

	// IPC port supplied?
	if (simple->flags & SRF_IPC && simple->ipc)
	{
		data->ipc = simple->ipc;
		data->waitbits = 1 << data->ipc->command_port->mp_SigBit;
	}

	// Signal supplied?
	else if (simple->flags & SRF_SIGNAL)
	{
		data->signal = (short)(IPTR)simple->ipc;
		data->waitbits = 1 << data->signal;
	}

	// Otherwise
	else
	{
		data->waitbits = IPCSIG_QUIT;
		SetSignal(0, IPCSIG_QUIT);
	}

	// Open requester
	if (_simplereq_open(data, parent))
	{
		// Wait on requester
		FOREVER
		{
			struct IntuiMessage *msg;
			IPCMessage *imsg;
			short break_flag = 0;
			ULONG waitres;

			// IPC message?
			if (data->ipc)
			{
				while ((imsg = (IPCMessage *)GetMsg(data->ipc->command_port)))
				{
					if (imsg->command == IPC_HIDE)
						_simplereq_close(data);
					else if (imsg->command == IPC_SHOW)
					{
						if (!(_simplereq_open(data, imsg->data)))
							break_flag = 1;
					}
					else if (imsg->command == IPC_ACTIVATE)
					{
						if (data->window)
						{
							WindowToFront(data->window);
							ActivateWindow(data->window);
						}
					}
					else if (imsg->command == IPC_QUIT || imsg->command == IPC_ABORT)
					{
						break_flag = 1;
						gadgetid = (UWORD)-1;
					}
					L_IPC_Reply(imsg);
				}
			}

			// Intuition message?
			if (data->window)
			{
				while ((msg = L_GetWindowMsg(data->window->UserPort, (APTR)libbase)))
				{
					struct IntuiMessage copy_msg;

					// Copy message and reply
					copy_msg = *msg;
					L_ReplyWindowMsg(msg);

					// Check message type
					if (copy_msg.Class == IDCMP_GADGETUP)
					{
						// Get gadget ID
						gadgetid = ((struct Gadget *)copy_msg.IAddress)->GadgetID;

						// Checkbox?
						if (gadgetid == GAD_CHECK_ID)
						{
							// Store value
							if (simple->check_ptr)
								*simple->check_ptr = copy_msg.Code;
						}

						// Tab on a string gadget?
						else if (gadgetid == GAD_STRING2_ID ||
								 (gadgetid == GAD_STRING_ID &&
								  (copy_msg.Code == 0x9 || copy_msg.Code == 0x45 ||
								   copy_msg.Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))))
							continue;

						// Popup gadget?
						else if (gadgetid == GAD_POPUP_ID)
						{
							char buf[400], file[40], *ptr;
							struct TagItem tags[4];

							// Get initial path and file
							strcpy(buf, (char *)L_GetGadgetValue(data->objlist, GAD_STRING_ID, data->libbase));
							if ((ptr = FilePart(buf)))
							{
								stccpy(file, ptr, 40);
								*ptr = 0;
							}
							else
								file[0] = 0;

							// Initialise tags
							tags[0].ti_Tag = ASLFR_Window;
							tags[0].ti_Data = (IPTR)data->window;
							tags[1].ti_Tag = ASLFR_InitialFile;
							tags[1].ti_Data = (IPTR)file;
							tags[2].ti_Tag = ASLFR_InitialDrawer;
							tags[2].ti_Data = (IPTR)buf;
							tags[3].ti_Tag = TAG_END;

							// Show requester
							if (simple->filereq && AslRequest(simple->filereq, tags))
							{
								// Build new path
								strcpy(buf, ((struct FileRequester *)simple->filereq)->fr_Drawer);
								AddPart(buf, ((struct FileRequester *)simple->filereq)->fr_File, 400);

								// Show in string gadget
								L_SetGadgetValue(data->objlist, GAD_STRING_ID, (IPTR)buf);
								L_ActivateStrGad(GADGET(L_GetObject(data->objlist, GAD_STRING_ID)), data->window);
							}
							continue;
						}

						// Set break flag
						else
							break_flag = 1;
					}

					// Key press
					else if (copy_msg.Class == IDCMP_VANILLAKEY)
					{
						// Y = yes
						if (copy_msg.Code == 'y' || copy_msg.Code == 'Y')
						{
							gadgetid = 1;
							break_flag = 1;
						}

						// N = no
						else if (copy_msg.Code == 'n' || copy_msg.Code == 'N')
						{
							gadgetid = 0;
							break_flag = 1;
						}
					}

					// Raw key press (OS4 strgclass swallows Escape before VANILLAKEY)
					else if (copy_msg.Class == IDCMP_RAWKEY)
					{
						if (copy_msg.Code == 0x45)
						{
							gadgetid = 0;
							break_flag = 1;
						}
					}
				}
			}

			// Check break flag
			if (break_flag)
				break;

			// Wait for a message
			waitres = Wait(data->waitbits | ((data->window) ? (1 << data->window->UserPort->mp_SigBit) : 0));

			// Quit?
			if (waitres & IPCSIG_QUIT)
			{
				gadgetid = 0;
				break;
			}

			// Signal?
			else if (data->signal != -1 && (waitres & (1 << data->signal)))
			{
				gadgetid = (UWORD)-5;
				break;
			}
		}
	}

	// Close requester
	_simplereq_close(data);

	// Free memory
	L_FreeMemHandle(memory);

	// Return gadget code
	return gadgetid;
}

BOOL _simplereq_open(simplereq_data *data, void *parent)
{
	// Not already open?
	if (!data->window)
	{
		short x, space, gad, object, gadsize = 0;

		// Set parent
		data->new_win.parent = parent;

		// Open window
		if (!(data->window = L_OpenConfigWindow(&data->new_win, data->libbase)))
			return 0;

		// Are gadgets smaller than text?
		if (data->max_gadget_width < data->max_text_width - 4)
		{
			short max;

			// Go through gadgets, get widest one
			for (gad = 0, max = 0; data->gadgets[gad]; gad++)
			{
				short width;

				// Get gadget width
				width = TextLength(data->window->RPort, data->gadgets[gad], strlen(data->gadgets[gad]));

				// Widest so far?
				if (width > max)
					max = width;
			}

			// Is there room for all gadgets to be the same size?
			if ((((max + GAD_X_EXTRA + GAD_X_SPACE) * data->gadget_count) - GAD_X_SPACE) <= data->max_text_width)
			{
				// Use standard size
				gadsize = max;
				data->max_gadget_width = (gadsize + GAD_X_EXTRA) * data->gadget_count;
			}
		}

		// Initialise x coordinate
		x = 3;

		// Less than 3 gadgets?
		if (data->gadget_count < 3)
			space = 0;

		// Gadgets are smaller than text?
		else if (data->max_gadget_width < data->max_text_width - 4)
			space = (data->max_text_width - data->max_gadget_width + 4) / (data->gadget_count - 1);

		// Fixed spacing
		else
			space = GAD_X_SPACE;

		// Go through gadgets
		for (gad = 0, object = data->first_object; data->gadgets[gad]; gad++, object++)
		{
			short width;

			// Get gadget width
			if (gadsize > 0)
				width = gadsize;
			else
				width = TextLength(data->window->RPort, data->gadgets[gad], strlen(data->gadgets[gad]));

			// Fill out gadget position
			data->objects[object].char_dims.Top = POS_RIGHT_JUSTIFY;
			data->objects[object].char_dims.Height = 1;
			data->objects[object].fine_dims.Top = -3;
			data->objects[object].fine_dims.Width = width + GAD_X_EXTRA;
			data->objects[object].fine_dims.Height = 6;

			// Special positioning?
			if (data->gadget_count < 3)
			{
				// Center?
				if (data->gadget_count == 1)
					data->objects[object].char_dims.Left = POS_CENTER;

				// Left?
				else if (gad == 0)
					data->objects[object].fine_dims.Left = x;

				// Right
				else
				{
					data->objects[object].char_dims.Left = POS_RIGHT_JUSTIFY;
					data->objects[object].fine_dims.Left = -3;
				}
			}

			// Final gadget?
			else if (gad == data->gadget_count - 1)
			{
				// Right-justify
				data->objects[object].char_dims.Left = POS_RIGHT_JUSTIFY;
				data->objects[object].fine_dims.Left = -3;
			}

			// Normal positioning
			else
				data->objects[object].fine_dims.Left = x;

			// Increment position
			x += width + GAD_X_EXTRA + space;
		}

		// Add objects
		if (!(data->objlist = L_AddObjectList(data->window, data->objects, data->libbase)))
		{
			L_CloseConfigWindow(data->window, data->libbase);
			data->window = 0;
			return 0;
		}

		// String gadget? Initialise and activate it
		if (data->string_buffer)
		{
			// Initialise (if a string)
			if (!(data->simple->flags & SRF_LONGINT))
			{
				// Initialise first gadget
				L_SetGadgetValue(data->objlist, GAD_STRING_ID, (IPTR)data->simple->string_buffer);

				// Initialise second string gadget
				if (data->string_buffer_2)
					L_SetGadgetValue(data->objlist, GAD_STRING2_ID, (IPTR)data->string_buffer_2);
			}

			// Activate it
			L_ActivateStrGad(GADGET(L_GetObject(data->objlist, GAD_STRING_ID)), data->window);
		}

		// Initialise checkbox
		if (data->simple->flags & SRF_CHECKMARK && data->simple->check_ptr)
			L_SetGadgetValue(data->objlist, GAD_CHECK_ID, (ULONG)*data->simple->check_ptr);
	}

	return 1;
}

void _simplereq_close(simplereq_data *data)
{
	if (data->window)
	{
		// Get strings
		if (data->string_buffer)
		{
			strcpy(data->string_buffer, (char *)L_GetGadgetValue(data->objlist, GAD_STRING_ID, data->libbase));

			if (data->string_buffer_2)
				strcpy(data->string_buffer_2, (char *)L_GetGadgetValue(data->objlist, GAD_STRING2_ID, data->libbase));
		}

		// Close window
		L_CloseConfigWindow(data->window, data->libbase);
		data->window = 0;
	}
}

// Simple interface to DoSimpleRequest
short LIBFUNC L_SimpleRequest(REG(a0, struct Window *parent),
							  REG(a1, char *title),
							  REG(a2, char *buttons),
							  REG(a3, char *message),
                              REG(d0, char *buffer),
                              REG(d1, APTR params),
                              REG(d2, long buffersize),
                              REG(d3, ULONG flags),
							  REG(a6, struct Library *libbase))
{
	char *data, *ptr;
	struct DOpusSimpleRequest *simple;
	short gadget_count, a;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = (struct Library *)dopuslibbase_global;
#endif

	// Allocate some storage space
	if (!(data = AllocVec(1200 + strlen(message), MEMF_CLEAR)))
		return 0;

	// Simple request pointer
	simple = (struct DOpusSimpleRequest *)data;
	ptr = data + sizeof(struct DOpusSimpleRequest);

	// Count gadgets
	if (!buttons)
		gadget_count = 0;
	else
	{
		for (gadget_count = 1, a = 0; buttons[a]; a++)
			if (buttons[a] == '|')
				++gadget_count;
	}

	// Gadget pointers
	simple->gadgets = (char **)ptr;
	ptr += sizeof(char *) * (gadget_count + 1);

	// Return value pointers
	simple->return_values = (int *)ptr;
	ptr += sizeof(int) * gadget_count;

	// Fill out gadget pointers
	if (gadget_count > 0)
	{
		short num = 0, ret = 1;
		BOOL first = 1;

		// Copy button string
		strcpy(ptr, buttons);

		// Fill in pointers
		for (a = 0; ptr[a] && num < gadget_count; a++)
		{
			// Start of a new button?
			if (first)
			{
				simple->return_values[num] = (num < gadget_count - 1 || gadget_count < 2) ? (ret++) : 0;
				simple->gadgets[num++] = ptr + a;
				first = 0;
			}

			// End of a button?
			else if (ptr[a] == '|')
			{
				first = 1;
				ptr[a] = 0;
			}
		}
	}

	// Build message
	LSprintf(data + 1024, message, params);

	// Fill out requester
	simple->title = (title) ? title : (((parent && !(flags & SRF_SCREEN_PARENT))) ? (char *)parent->Title : 0);
	simple->message = data + 1024;

	// Got a buffer?
	if (buffer)
	{
		simple->string_buffer = buffer;
		simple->string_len = buffersize;
	}

	// Or in flags
	simple->flags |= flags;

	// Do requester
	a = L_DoSimpleRequest(parent, simple, libbase);

	// Free data
	FreeVec(data);
	return a;
}

// Build requester
BOOL simple_build(simplereq_data *data)
{
	struct DOpusSimpleRequest *simple;
	short count, object, pos;
	struct TextFont *font = 0;
	struct RastPort rp;
	struct Screen *scr, *pub = 0;
	short font_try;

	// Get simple pointer
	simple = data->simple;

	// Get screen pointer
	if (simple->flags & SRF_SCREEN_PARENT && data->parent)
		scr = data->parent;
	else if (data->parent)
		scr = ((struct Window *)data->parent)->WScreen;
	else if ((pub = LockPubScreen(0)))
		scr = pub;
	else
		return 0;

	// Initialise dummy rastport
	InitRastPort(&rp);

	// Try 3 fonts (screen font, default font, topaz)
	for (font_try = 0; font_try < 3; font_try++)
	{
		short width, len, pos;
		char *ptr;

		// Get font
		if (font_try == 0)
			font = scr->RastPort.Font;
		else if (font_try == 1)
			font = ((struct GfxBase *)GfxBase)->DefaultFont;
		else
			font = topaz_font;

		// Set font in dummy rastport
		SetFont(&rp, font);

		// Initialise counts
		data->text_lines = 0;
		data->max_text_width = 0;

		// Get max text width and number of lines
		for (pos = 0, len = 0, ptr = simple->message;; pos++)
		{
			// End of a line?
			if (simple->message[pos] == '\0' || simple->message[pos] == '\n')
			{
				// Get text width
				width = TextLength(&rp, ptr, len);

				// See if this is the longest line
				if (width > data->max_text_width)
					data->max_text_width = width;

				// Reset length, increment line count
				len = 0;
				ptr = simple->message + pos + 1;
				++data->text_lines;

				// End of text?
				if (simple->message[pos] == '\0')
					break;
			}

			// Otherwise, increment length
			else
				len++;
		}

		// Will fit in screen?
		if (data->max_text_width < scr->Width - TEXT_X_SPACE)
			break;
	}

	// Add space to text
	if (data->max_text_width >= scr->Width - TEXT_X_SPACE)
		data->max_text_width = scr->Width;
	else
		data->max_text_width += TEXT_X_SPACE;

	// Save font to use
	simple->font = font;

	// Unlock public screen
	if (pub)
		UnlockPubScreen(0, pub);

	// Count gadgets
	for (count = 0; simple->gadgets[count]; count++)
	{
		short width;

		// Get width of gadget text
		width = TextLength(&rp, simple->gadgets[count], strlen(simple->gadgets[count]));

		// Add to total gadget width
		data->max_gadget_width += width + GAD_X_EXTRA;
	}

	// Save count
	data->gadget_count = count;

	// Are gadgets bigger than text?
	if (data->max_gadget_width + ((count - 1) * GAD_X_SPACE) > data->max_text_width)
	{
		// Add spacing between gadgets
		data->max_gadget_width += ((count - 1) * GAD_X_SPACE);
	}

	// Calculate window width
	data->req_dims.fine_dim.Width =
		10 + ((data->max_text_width > data->max_gadget_width) ? data->max_text_width : data->max_gadget_width);

	// Height is number of text lines plus one line for gadgets
	data->req_dims.char_dim.Height = data->text_lines + 1;
	data->req_dims.fine_dim.Height = 39 + ((data->text_lines - 1) * 3);

	// String gadget?
	if (simple->string_buffer)
	{
		// Get string length
		data->string_len = simple->string_len;

		// Add height
		data->req_dims.char_dim.Height++;
		data->req_dims.fine_dim.Height += 5;

		// Second string gadget?
		if (data->string_len < 0)
		{
			// Fix length
			data->string_len = -data->string_len;
			data->string_buffer_2 = simple->string_buffer + data->string_len + 1;
			data->string_len_2 = data->string_len;

			// Adjust height
			data->req_dims.char_dim.Height++;
			data->req_dims.fine_dim.Height += 5;
		}
		else if (simple->flags & SRF_BUFFER2)
		{
			data->string_buffer_2 = simple->string_buffer_2;
			data->string_len_2 = simple->string_len_2;

			// Adjust height
			data->req_dims.char_dim.Height++;
			data->req_dims.fine_dim.Height += 5;
		}
	}

	// Checkmark?
	if (simple->flags & SRF_CHECKMARK)
	{
		// Adjust height
		data->req_dims.char_dim.Height++;
		data->req_dims.fine_dim.Height += 9;
	}

	// Fill out NewConfigWindow
	data->new_win.dims = &data->req_dims;
	data->new_win.title = simple->title;
	data->new_win.flags = WINDOW_NO_CLOSE | WINDOW_AUTO_KEYS | WINDOW_REQ_FILL;
	if (simple->flags & SRF_SCREEN_PARENT)
		data->new_win.flags |= WINDOW_SCREEN_PARENT;
	data->new_win.font = simple->font;

	// Calculate number of objects we'll need; area plus text plus gadgets plus end
	data->object_count = data->text_lines + data->gadget_count + 2;

	// If strbuf is supplied, need a string gadget
	if (simple->string_buffer)
	{
		++data->object_count;
		if (data->string_buffer_2)
			++data->object_count;
		if (simple->flags & SRF_FILEREQ && simple->filereq)
			++data->object_count;
	}

	// Checkmark?
	if (simple->flags & SRF_CHECKMARK)
		++data->object_count;

	// Allocate objects
	if (!(data->objects = (ObjectDef *)L_AllocMemH(data->memory, sizeof(ObjectDef) * data->object_count)))
		return 0;

	// Fill out text area
	data->objects[0].type = OD_AREA;
	data->objects[0].char_dims.Width = SIZE_MAXIMUM;
	data->objects[0].char_dims.Height = data->text_lines + ((simple->flags & SRF_CHECKMARK) ? 1 : 0);
	data->objects[0].fine_dims.Left = 3;
	data->objects[0].fine_dims.Top = 3;
	data->objects[0].fine_dims.Width = -3;
	data->objects[0].fine_dims.Height = 24 + ((data->text_lines - 1) * 3) + ((simple->flags & SRF_CHECKMARK) ? 9 : 0);
	data->objects[0].gadgetid = 32767;
	data->objects[0].flags = AREAFLAG_RECESSED | AREAFLAG_ERASE | AREAFLAG_THIN;

	// Layout tags
	data->rel_tags[0].ti_Tag = GTCustom_LayoutRel;
	data->rel_tags[0].ti_Data = 32767;
	data->rel_tags[1].ti_Tag = TAG_END;

	// Allocate key array
	data->keys = (unsigned char *)L_AllocMemH(data->memory, data->gadget_count);

	// Build objects; first text
	for (pos = 0, object = 1, data->start_pos = 0;; pos++)
	{
		// End of a text line?
		if (simple->message[pos] == '\0' || simple->message[pos] == '\n')
		{
			// Fill out text object
			data->objects[object].type = OD_TEXT;
			data->objects[object].object_kind = TEXTPEN;
			data->objects[object].char_dims.Top = object - 1;
			data->objects[object].char_dims.Width = SIZE_MAXIMUM;
			data->objects[object].char_dims.Height = 1;
			data->objects[object].fine_dims.Top = 10 + ((object - 1) * 3);

			// Get copy of text
			if ((data->objects[object].gadget_text = (IPTR)L_AllocMemH(data->memory, (pos - data->start_pos) + 1)))
			{
				CopyMem(simple->message + data->start_pos,
						(char *)data->objects[object].gadget_text,
						pos - data->start_pos);
				data->objects[object].flags = TEXTFLAG_TEXT_STRING | TEXTFLAG_NO_USCORE;
			}

			// Flags and id
			data->objects[object].flags |= TEXTFLAG_CENTER;
			data->objects[object].gadgetid = GAD_TEXT_ID;
			data->objects[object].taglist = data->rel_tags;

			// Save start position for next line, increment object count
			data->start_pos = pos + 1;
			++object;

			// End of text?
			if (simple->message[pos] == '\0')
				break;
		}
	}

	// String gadget?
	if (simple->string_buffer)
	{
		// Second string gadget?
		if (data->string_buffer_2)
		{
			data->objects[object].type = OD_GADGET;
			data->objects[object].object_kind = (simple->flags & SRF_LONGINT) ? INTEGER_KIND : STRING_KIND;
			data->objects[object].char_dims.Top = POS_RIGHT_JUSTIFY - 2;
			data->objects[object].char_dims.Width = SIZE_MAXIMUM;
			data->objects[object].char_dims.Height = 1;
			data->objects[object].fine_dims.Left = 3;
			data->objects[object].fine_dims.Top = -16;
			data->objects[object].fine_dims.Width = -3;
			data->objects[object].fine_dims.Height = 4;
			data->objects[object].gadgetid = GAD_STRING2_ID;
			data->objects[object].taglist = data->tags2;

			// String tags
			data->tags2[0].ti_Tag = (simple->flags & SRF_LONGINT) ? GTIN_Number : TAG_IGNORE;
			data->tags2[0].ti_Data = (simple->flags & SRF_LONGINT) ? (ULONG)atoi(data->string_buffer_2) : 0;
			data->tags2[1].ti_Tag = (simple->flags & SRF_LONGINT) ? GTIN_MaxChars : GTST_MaxChars;
			data->tags2[1].ti_Data = data->string_len;
			data->tags2[2].ti_Tag = STRINGA_Justification;
			data->tags2[2].ti_Data = (simple->flags & SRF_CENTJUST)
										 ? GACT_STRINGCENTER
										 : ((simple->flags & SRF_RIGHTJUST) ? GACT_STRINGRIGHT : GACT_STRINGLEFT);
			data->tags2[3].ti_Tag = GTCustom_PathFilter;
			data->tags2[3].ti_Data = (simple->flags & SRF_PATH_FILTER) ? 1 : 0;
			data->tags2[4].ti_Tag = GTCustom_Secure;
			data->tags2[4].ti_Data = (simple->flags & SRF_SECURE) ? 1 : 0;
			data->tags2[5].ti_Tag = GTCustom_ThinBorders;
			data->tags2[5].ti_Data = TRUE;
			data->tags2[6].ti_Tag = TAG_END;

			// Increment object
			++object;
		}

		// File requester?
		if (simple->flags & SRF_FILEREQ && simple->filereq)
		{
			// Popup button
			data->objects[object].type = OD_GADGET;
			data->objects[object].object_kind = POPUP_BUTTON_KIND;
			data->objects[object].char_dims.Top = POS_RIGHT_JUSTIFY - 1;
			data->objects[object].char_dims.Width = 0;
			data->objects[object].char_dims.Height = 1;
			data->objects[object].fine_dims.Left = 3;
			data->objects[object].fine_dims.Top = -11;
			data->objects[object].fine_dims.Width = 28;
			data->objects[object].fine_dims.Height = 4;
			data->objects[object].gadgetid = GAD_POPUP_ID;
			data->objects[object].taglist = 0;

			// Increment object
			++object;
		}

		// String gadget
		data->objects[object].type = OD_GADGET;
		data->objects[object].object_kind = (simple->flags & SRF_LONGINT) ? INTEGER_KIND : STRING_KIND;
		data->objects[object].char_dims.Top = POS_RIGHT_JUSTIFY - 1;
		data->objects[object].char_dims.Width = SIZE_MAXIMUM;
		data->objects[object].char_dims.Height = 1;
		data->objects[object].fine_dims.Left = (simple->flags & SRF_FILEREQ && simple->filereq) ? 31 : 3;
		data->objects[object].fine_dims.Top = -11;
		data->objects[object].fine_dims.Width = -3;
		data->objects[object].fine_dims.Height = 4;
		data->objects[object].gadgetid = GAD_STRING_ID;
		data->objects[object].taglist = data->tags;

		// String tags
		data->tags[0].ti_Tag = (simple->flags & SRF_LONGINT) ? GTIN_Number : TAG_IGNORE;
		data->tags[0].ti_Data = (simple->flags & SRF_LONGINT) ? (ULONG)atoi(simple->string_buffer) : 0;
		data->tags[1].ti_Tag = (simple->flags & SRF_LONGINT) ? GTIN_MaxChars : GTST_MaxChars;
		data->tags[1].ti_Data = data->string_len;
		data->tags[2].ti_Tag = STRINGA_Justification;
		data->tags[2].ti_Data = (simple->flags & SRF_CENTJUST)
									? GACT_STRINGCENTER
									: ((simple->flags & SRF_RIGHTJUST) ? GACT_STRINGRIGHT : GACT_STRINGLEFT);
		data->tags[3].ti_Tag = GTCustom_NoSelectNext;
		data->tags[3].ti_Data = TRUE;
		data->tags[4].ti_Tag = GTCustom_History;
		data->tags[4].ti_Data = (simple->flags & SRF_HISTORY) ? (IPTR)simple->history : 0;
		data->tags[5].ti_Tag = GTCustom_PathFilter;
		data->tags[5].ti_Data = (simple->flags & SRF_PATH_FILTER) ? 1 : 0;
		data->tags[6].ti_Tag = GTCustom_Secure;
		data->tags[6].ti_Data = (simple->flags & SRF_SECURE) ? 1 : 0;
		data->tags[7].ti_Tag = GTCustom_ThinBorders;
		data->tags[7].ti_Data = TRUE;
		data->tags[8].ti_Tag = TAG_END;

		// Increment object
		++object;
	}

	// Checkmark?
	if (simple->flags & SRF_CHECKMARK)
	{
		// Fill out gadget
		data->objects[object].type = OD_GADGET;
		data->objects[object].object_kind = CHECKBOX_KIND;
		data->objects[object].char_dims.Top = POS_RIGHT_JUSTIFY;
		data->objects[object].char_dims.Left = POS_CENTER;
		data->objects[object].char_dims.Height = 1;
		data->objects[object].fine_dims.Left = FPOS_TEXT_OFFSET + (strlen(simple->check_text) >> 1);
		data->objects[object].fine_dims.Top = -4;
		data->objects[object].fine_dims.Width = 28;
		data->objects[object].fine_dims.Height = 6;
		data->objects[object].gadget_text = (IPTR)simple->check_text;
		data->objects[object].flags = TEXTFLAG_TEXT_STRING | PLACETEXT_RIGHT;
		data->objects[object].gadgetid = GAD_CHECK_ID;
		data->objects[object].taglist = data->rel_tags;

		// Increment object
		++object;
	}

	// Build gadgets data
	data->first_object = object;
	for (pos = 0; simple->gadgets[pos]; pos++, object++)
	{
		int a, prior;

		// Fill out gadget object
		data->objects[object].type = OD_GADGET;
		data->objects[object].object_kind = BUTTON_KIND;
		data->objects[object].flags = TEXTFLAG_TEXT_STRING | PLACETEXT_IN | BUTTONFLAG_THIN_BORDERS;
		data->objects[object].gadgetid = simple->return_values[pos];

		// Okay button?
		if (simple->return_values[pos] == 1)
			data->objects[object].flags |= BUTTONFLAG_OKAY_BUTTON;

		// Cancel button?
		else if (simple->return_values[pos] == 0)
			data->objects[object].flags |= BUTTONFLAG_CANCEL_BUTTON;

		// Get unique key for this gadget
		for (a = 0; simple->gadgets[pos][a]; a++)
		{
			// Get key
			data->keys[pos] = (unsigned char)simple->gadgets[pos][a];
			if (data->keys[pos] >= 'A' && data->keys[pos] <= 'Z')
				data->keys[pos] += ('a' - 'A');

			// Check it's not used before
			for (prior = 0; prior < pos; prior++)
				if (data->keys[prior] == data->keys[pos])
					break;

			// Is it unique?
			if (pos == 0 || prior == pos)
				break;

			// Try again
			data->keys[pos] = 0;
		}

		// Get name (with underscore)
		if ((data->objects[object].gadget_text = (IPTR)L_AllocMemH(data->memory, strlen(simple->gadgets[pos]) + 2)))
		{
			short ch, cp, got_us = 0;
			unsigned char gc;

			// Copy string, insert _
			for (ch = 0, cp = 0; simple->gadgets[pos][ch]; ch++)
			{
				gc = (unsigned char)simple->gadgets[pos][ch];
				if (gc >= 'A' && gc <= 'Z')
					gc += ('a' - 'A');
				if (data->keys[pos] == gc)
				{
					if (!got_us)
					{
						((char *)data->objects[object].gadget_text)[cp++] = '_';
						got_us = 1;
					}
				}
				((char *)data->objects[object].gadget_text)[cp++] = simple->gadgets[pos][ch];
			}
		}
		else
			data->objects[object].gadget_text = (IPTR)simple->gadgets[pos];
	}

	return 1;
}
