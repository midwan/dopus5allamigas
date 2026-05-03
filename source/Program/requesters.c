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

unsigned char map_font_colour(unsigned char);

// generic requester wrapper
short super_request(APTR parent, char *message, IPCData *ipc, IPTR *flagptr)
{
	struct super_request_data *data;
	short ret = 1;

	// Allocate data
	if (!(data = AllocVec(sizeof(struct super_request_data), MEMF_CLEAR)))
		return 0;

	// Initialise data
	data->ret_val = 1;

	// Initialise simple request structure
	data->request.title = GetString(&locale, MSG_DIRECTORY_OPUS_REQUEST);
	data->request.message = message;
	data->request.gadgets = data->gadgets;
	data->request.return_values = data->return_values;
	data->request.flags = flagptr[0];

	// Asked to open on main window?
	if (!(data->request.flags & SRF_SCREEN_PARENT) && parent == (APTR)GUI->window)
	{
		// Change to open on main screen, unless mouse positioning is set
		if (!(data->request.flags & SRF_MOUSE_POS))
		{
			parent = GUI->screen_pointer;
			data->request.flags |= SRF_SCREEN_PARENT;
		}
	}

	// Buffer supplied?
	if (data->request.flags & SRF_BUFFER)
	{
		// Get buffer pointer
		data->request.string_buffer = (char *)flagptr[ret++];

		// Get buffer length
		data->request.string_len = (int)flagptr[ret++];
	}

	// Buffer supplied?
	if (data->request.flags & SRF_BUFFER2)
	{
		// Get buffer pointer
		data->request.string_buffer_2 = (char *)flagptr[ret++];

		// Get buffer length
		data->request.string_len_2 = (int)flagptr[ret++];
	}

	// History supplied?
	if (data->request.flags & SRF_HISTORY)
	{
		// Get history list
		data->request.history = (Att_List *)flagptr[ret++];
	}

	// IPC supplied?
	if (ipc)
	{
		// Store pointer, set flag
		data->request.ipc = ipc;
		data->request.flags |= SRF_IPC;
	}

	// IPC or signal bit supplied?
	else if (data->request.flags & (SRF_IPC | SRF_SIGNAL))
	{
		// Get IPC pointer
		data->request.ipc = (IPCData *)flagptr[ret++];
	}

	// Checkmark supplied?
	if (data->request.flags & SRF_CHECKMARK)
	{
		// Get checkmark pointers
		data->request.check_text = (char *)flagptr[ret++];
		data->request.check_ptr = (short *)flagptr[ret++];
	}

	// Go through gadgets
	while ((data->gad = (char *)flagptr[ret++]))
	{
		// Valid gadget?
		if (data->gad == (char *)-1)
			continue;

		// Store gadget in the array
		data->gadgets[data->gad_count] = data->gad;
		data->return_values[data->gad_count++] = data->ret_val++;

		// Maximum of 10 gadgets
		if (data->gad_count == 10)
			break;
	}

	// If we got more than 1 gadget, make the last gadget a "cancel" gadget
	if (data->gad_count > 1)
		data->return_values[data->gad_count - 1] = 0;

	// Zero the last gadget pointer
	data->gadgets[data->gad_count] = 0;

	// Do the request
	ret = DoSimpleRequest(parent, &data->request);

	// Free data
	FreeVec(data);
	return ret;
}

// Display error requester
short error_request_a(struct Window *window, short flags, char **text)
{
	short code;
	char *buffer, *ptr;
	BOOL quote_flag = 0;

	// Get error code
	code = IoErr();

	// Allocate memory for error
	if (!(buffer = AllocVec(1024, MEMF_CLEAR)))
		return 0;

	// Build error string
	if (*text)
	{
		strcpy(buffer, GetString(&locale, MSG_AN_ERROR_OCCURED));
		while (*text)
		{
			// -1 indicates next item is to be quoted
			if (*text == (char *)-1)
			{
				quote_flag = 1;
				++text;
			}

			// Otherwise, add item to string
			else
			{
				if (quote_flag)
					strcat(buffer, "'");
				strcat(buffer, *text);
				if (quote_flag)
					strcat(buffer, "'");

				// If next item is valid, add space
				++text;
				if (*text)
					strcat(buffer, " ");
				quote_flag = 0;
			}
		}
	}
	else
	{
		strcpy(buffer, GetString(&locale, MSG_ERROR));
	}

	// Get fault text
	Fault(code, "", buffer + 944, 80);

	// Get start of text
	if ((ptr = strchr(buffer + 944, ':')))
		++ptr;
	else
		ptr = buffer + 944;

	// Valid text?
	if (*ptr)
	{
		// Add LF if needed
		if (*text)
			strcat(buffer, "\n");

		// Add error text
		strcat(buffer, ptr);
	}

	// Async?
	if (flags & ERF_ASYNC)
		global_requester(buffer);

	// Display error
	else
		code = super_request_args(window,
								  buffer,
								  (flags & ERF_MOUSE) ? SRF_MOUSE_POS : 0,
								  GetString(&locale, MSG_RETRY),
								  GetString(&locale, MSG_CANCEL),
								  0);

	// Free buffer
	FreeVec(buffer);
	return code;
}

// Solicit a file
int request_file(struct Window *parent, char *title, char *buffer, char *def, ULONG flags, char *pattern)
{
	struct IBox coords;
	struct FileRequester *filereq;
	char *path = NULL;
	char *file = NULL;
	int ret;

	// Allocate path
	if (!(path = AllocVec(300, MEMF_ANY | MEMF_CLEAR)))
		return 0;

	// Get current path
	if (buffer[0])
		strcpy(path, buffer);
	else if (def)
		strcpy(path, def);
	if (path[0])
	{
		file = FilePart(path);
		if (file && file > path)
		{
			strcpy(path + 256, file);
			*file = 0;
			file = path + 256;
		}
	}

	// Get current requester coordinates
	GetSemaphore(&GUI->req_lock, SEMF_SHARED, 0);
	coords = GUI->req_coords;
	FreeSemaphore(&GUI->req_lock);

	// Allocate filerequester
	if (!(filereq = AllocAslRequestTags(ASL_FileRequest,
										(flags & (1 << 30)) ? ASLFR_Screen : ASLFR_Window,
										parent,
										ASLFR_TitleText,
										title,
										(file) ? ASLFR_InitialFile : TAG_IGNORE,
										file,
										(*path) ? ASLFR_InitialDrawer : TAG_IGNORE,
										path,
										ASLFR_Flags1,
										(flags | FRF_PRIVATEIDCMP) & ~((1 << 31) | (1 << 30) | (1 << 29)),
										ASLFR_Flags2,
										(flags & (1 << 31)) ? 0 : FRF_REJECTICONS,
										ASLFR_InitialLeftEdge,
										coords.Left,
										ASLFR_InitialTopEdge,
										coords.Top,
										ASLFR_InitialWidth,
										coords.Width,
										ASLFR_InitialHeight,
										coords.Height,
										(flags & (1 << 29)) ? ASLFR_InitialPattern : TAG_IGNORE,
										pattern,
										TAG_END)))
	{
		FreeVec(path);
		return 0;
	}

	// Display requester
	ret = AslRequest(filereq, 0);

	if (ret)
	{
		// Build path
		strcpy(buffer, filereq->fr_Drawer);
		AddPart(buffer, filereq->fr_File, 256);
		if (!*buffer || !*filereq->fr_File)
			ret = 0;
	}

	// Save coordinates
	GetSemaphore(&GUI->req_lock, SEMF_EXCLUSIVE, 0);
	GUI->req_coords = *((struct IBox *)&filereq->fr_LeftEdge);
	FreeSemaphore(&GUI->req_lock);

	// Free file requester
	FreeAslRequest(filereq);
	FreeVec(path);
	return ret;
}

// Solicit a font
short request_font(struct Window *parent,
				   char *title,
				   char *buffer,
				   short *size,
				   ULONG flags,
				   unsigned short font_pen_count,
				   unsigned char *font_pen_table,
				   short *fpen,
				   short *bpen)
{
	struct IBox coords;
	struct FontRequester *fontreq;
	short ret;

	// Get current requester coordinates
	GetSemaphore(&GUI->req_lock, SEMF_SHARED, 0);
	coords = GUI->req_coords;
	FreeSemaphore(&GUI->req_lock);

	// Allocate fontrequester
	if (!(fontreq = AllocAslRequestTags(ASL_FontRequest,
										ASLFO_Window,
										parent,
										ASLFO_TitleText,
										title,
										ASLFO_InitialName,
										buffer,
										ASLFO_InitialSize,
										*size,
										ASLFO_Flags,
										flags | FOF_PRIVATEIDCMP,
										ASLFO_InitialLeftEdge,
										coords.Left,
										ASLFO_InitialTopEdge,
										coords.Top,
										ASLFO_InitialWidth,
										coords.Width,
										ASLFO_InitialHeight,
										coords.Height,
										ASLFO_MaxFrontPen,
										font_pen_count,
										ASLFO_MaxBackPen,
										font_pen_count,
										ASLFO_FrontPens,
										font_pen_table,
										ASLFO_BackPens,
										font_pen_table,
										ASLFO_InitialFrontPen,
										(fpen) ? *fpen : 0,
										ASLFO_InitialBackPen,
										(bpen) ? *bpen : 0,
										TAG_END)))
	{
		return 0;
	}

	// Display requester
	ret = AslRequest(fontreq, 0);

	// Success?
	if (ret)
	{
		// Store new font
		strcpy(buffer, fontreq->fo_Attr.ta_Name);
		*size = fontreq->fo_Attr.ta_YSize;

		// Store colours
		if (fpen)
			*fpen = map_font_colour(fontreq->fo_FrontPen);
		if (bpen)
			*bpen = map_font_colour(fontreq->fo_BackPen);
	}

	// Save coordinates
	GetSemaphore(&GUI->req_lock, SEMF_EXCLUSIVE, 0);
	GUI->req_coords = *((struct IBox *)&fontreq->fo_LeftEdge);
	FreeSemaphore(&GUI->req_lock);

	// Free font requester
	FreeAslRequest(fontreq);
	return ret;
}

unsigned char map_font_colour(unsigned char col)
{
	unsigned char pen;

	// OS colours return as is
	if (col < 4 || col >= 252)
		return col;

	// Go through user pen array
	for (pen = 0; pen < environment->env->palette_count; pen++)
		if (GUI->pens[pen] == col)
			return (unsigned char)(pen + 4);
	return col;
}

// Allocate a filerequester
APTR alloc_filereq(void)
{
	struct IBox coords;

	// Get current requester coordinates
	GetSemaphore(&GUI->req_lock, SEMF_SHARED, 0);
	coords = GUI->req_coords;
	FreeSemaphore(&GUI->req_lock);

	// Allocate filerequester
	return AllocAslRequestTags(ASL_FileRequest,
							   ASLFR_InitialLeftEdge,
							   coords.Left,
							   ASLFR_InitialTopEdge,
							   coords.Top,
							   ASLFR_InitialWidth,
							   coords.Width,
							   ASLFR_InitialHeight,
							   coords.Height,
							   TAG_END);
}

// Show a requester
void global_requester(char *text)
{
	RexxRequester *req;
	char *ok_str;

	// Get OK string
	ok_str = GetString(&locale, MSG_OKAY);

	// Allocate requester argument packet
	if ((req = AllocVec(sizeof(RexxRequester) + strlen(text) + strlen(ok_str) + 5, MEMF_CLEAR)))
	{
		// Store message
		req->command = RXCMD_REQUEST;

		// Copy string
		strcpy(req->args, "\"");
		strcat(req->args, text);
		strcat(req->args, "\" ");
		strcat(req->args, ok_str);

		// Startup requester process
		if (!(misc_startup("dopus_rexx_request", REXX_REQUESTER, GUI->window, req, 0)))
			FreeVec(req);
	}
}
