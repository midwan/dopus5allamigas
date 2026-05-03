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
#include "progress_win.h"
#include <Program/pattern.h>

void SAVEDS progress_task(void);
void progress_open(ProgressWindow *);
void progress_close(ProgressWindow *);
void progress_draw(ProgressWindow *, unsigned long);
void progress_text(ProgressWindow *, char *, struct IBox *, BOOL);
void progress_set(ProgressWindow *, struct TagItem *);
void progress_get(ProgressWindow *, struct TagItem *);
void progress_bar(ProgressWindow *, unsigned long, long, long);

// Open a progress window
ProgressWindow *LIBFUNC L_OpenProgressWindow(REG(a0, struct TagItem *tags), REG(a6, struct MyLibrary *lib))
{
	ProgressWindow *prog;
	char *ptr;
	struct TagItem *tag = NULL;

#ifdef __amigaos4__
	lib = dopuslibbase_global;
#endif

	// Allocate control structure
	if (!(prog = AllocVec(sizeof(ProgressWindow), MEMF_CLEAR)))
		return 0;

	// Fill out control parameters
	prog->pw_Screen = (struct Screen *)GetTagData(PW_Screen, 0, tags);
	prog->pw_OwnerWindow = (struct Window *)GetTagData(PW_Window, 0, tags);
	if ((ptr = (char *)GetTagData(PW_Title, 0, tags)))
		stccpy(prog->pw_Title, ptr, 39);
	if ((ptr = (char *)GetTagData(PW_Info, 0, tags)))
		stccpy(prog->pw_Information, ptr, 79);
	if ((ptr = (char *)GetTagData(PW_Info2, 0, tags)))
		stccpy(prog->pw_Information2, ptr, 79);
	if ((ptr = (char *)GetTagData(PW_Info3, 0, tags)))
		stccpy(prog->pw_Information3, ptr, 79);
	prog->pw_SigTask = (struct Task *)GetTagData(PW_SigTask, 0, tags);
	prog->pw_SigBit = GetTagData(PW_SigBit, 0, tags);
	prog->pw_Flags = GetTagData(PW_Flags, 0, tags);
	prog->pw_FileCount = GetTagData(PW_FileCount, 0, tags);
	prog->pw_FileSize = GetTagData(PW_FileSize, 0, tags);
	if ((ptr = (char *)GetTagData(PW_FileName, 0, tags)))
		stccpy(prog->pw_FileName, ptr, 79);
	prog->pw_Scale = FALSE;	 // Identifies 32bit size for update functions

	// Check for 64bit file size
	if ((tag = FindTagItem(PW_FileSize64, tags)))
	{
		UQUAD filesize = *(UQUAD *)tag->ti_Data;
		if (filesize > 0x7FFFFFFF)
		{
			prog->pw_FileSize = (long)(filesize >> 8);
			prog->pw_Scale = TRUE;	// Identifies 64bit size for updates
		}
		else
			prog->pw_FileSize = (long)(filesize);
	}

	// If we have both size and the bar set, we swap them around
	if (prog->pw_Flags & PWF_FILESIZE && prog->pw_Flags & PWF_GRAPH)
		prog->pw_Flags |= PWF_SWAP;

	// Save library pointer
	prog->pw_Lib = lib;

	// Initialise task name
	strcpy(prog->pw_TaskName, "dopus_progressbar");

	// Launch progress task
	L_IPC_Launch(0,
				 &prog->pw_IPC,
				 prog->pw_TaskName,
				 IPC_NATIVE(progress_task),
				 STACK_DEFAULT,
				 (ULONG)prog,
				 (struct Library *)DOSBase,
				 lib);

	// Failed?
	if (!prog->pw_IPC)
	{
		FreeVec(prog);
		return 0;
	}

	// Return pointer
	return prog;
}

// Close a progress window
void LIBFUNC L_CloseProgressWindow(REG(a0, ProgressWindow *prog))
{
	// Valid window?
	if (prog)
	{
		// Send quit command
		L_IPC_Quit(prog->pw_IPC, 0, TRUE);
	}
}

// Hide progress window
void LIBFUNC L_HideProgressWindow(REG(a0, ProgressWindow *prog))
{
	// Valid window?
	if (prog)
	{
		// Clear owner window pointer
		prog->pw_OwnerWindow = 0;

		// Send hide command
		L_IPC_Command(
			prog->pw_IPC, IPC_HIDE, 0, 0, 0, (prog->pw_Flags & PWF_NOIPC) ? REPLY_NO_PORT_IPC : REPLY_NO_PORT);
	}
}

// Show progress window
void LIBFUNC L_ShowProgressWindow(REG(a0, ProgressWindow *prog),
								  REG(a1, struct Screen *screen),
								  REG(a2, struct Window *window))
{
	// Valid window?
	if (prog)
	{
		// Set new screen/window pointers (if either one is given)
		if (screen || window)
		{
			prog->pw_Screen = screen;
			prog->pw_OwnerWindow = window;
		}

		// Send show command
		L_IPC_Command(
			prog->pw_IPC, IPC_SHOW, 0, 0, 0, (prog->pw_Flags & PWF_NOIPC) ? REPLY_NO_PORT_IPC : REPLY_NO_PORT);
	}
}

// Set progress window parameters
void LIBFUNC L_SetProgressWindow(REG(a0, ProgressWindow *prog), REG(a1, struct TagItem *tags))
{
	// Valid window?
	if (prog)
	{
		// Send parameters
		L_IPC_Command(
			prog->pw_IPC, PROGRESS_SET, 0, tags, 0, (prog->pw_Flags & PWF_NOIPC) ? REPLY_NO_PORT_IPC : REPLY_NO_PORT);
	}
}

// Get progress window parameters
void LIBFUNC L_GetProgressWindow(REG(a0, ProgressWindow *prog), REG(a1, struct TagItem *tags))
{
	// Valid window?
	if (prog)
	{
		// Send query
		L_IPC_Command(
			prog->pw_IPC, PROGRESS_GET, 0, tags, 0, (prog->pw_Flags & PWF_NOIPC) ? REPLY_NO_PORT_IPC : REPLY_NO_PORT);
	}
}

// Check progress abort
BOOL LIBFUNC L_CheckProgressAbort(REG(a0, ProgressWindow *prog))
{
	BOOL abort = 0;

	// Valid window?
	if (prog)
	{
		// Has it aborted?
		if (prog->pw_Flags & PWF_ABORTED)
		{
			// Clear flag
			prog->pw_Flags &= ~PWF_ABORTED;
			abort = 1;
		}
	}

	return abort;
}

#define DOpusBase (prog->pw_Lib)

// Progress window task
void SAVEDS progress_task(void)
{
	IPCData *ipc;
	ProgressWindow *prog;
	IPCMessage *msg;

	// Do startup
	if (!(ipc = L_IPC_ProcStartup((APTR)&prog, 0)))
		return;

	/*
		// Debug?
		if (prog->pw_Flags&PWF_DEBUG)
			D(bug("progress task : code entry %lx\n",(ULONG)progress_task));
	*/

	// Open invisibly?
	if (prog->pw_Flags & PWF_INVISIBLE)
		prog->pw_Flags &= ~PWF_INVISIBLE;

	// Open progress window
	else
		progress_open(prog);

	// Loop for messages
	FOREVER
	{
		BOOL quit = 0;

		// Window open?
		if (prog->pw_Window)
		{
			struct IntuiMessage *msg;

			// Look for messages
			while ((msg = (struct IntuiMessage *)GetMsg(prog->pw_Window->UserPort)))
			{
				// Look at message
				switch (msg->Class)
				{
				// Key press
				case IDCMP_RAWKEY:

					// If not escape, break
					if (msg->Code != 0x45)
						break;

				// Abort
				case IDCMP_CLOSEWINDOW:
				case IDCMP_GADGETUP:

					// Task to signal?
					if (prog->pw_SigTask)
						Signal(prog->pw_SigTask, 1 << prog->pw_SigBit);

					// Set flag
					prog->pw_Flags |= PWF_ABORTED;
					break;

				// Refresh
				case IDCMP_REFRESHWINDOW:

					// Refresh window
					BeginRefresh(prog->pw_Window);
					progress_draw(prog, PWF_ALL);
					EndRefresh(prog->pw_Window, TRUE);
					break;
				}

				// Reply the message
				ReplyMsg((struct Message *)msg);
			}
		}

		// Any messages?
		while ((msg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Look at message
			switch (msg->command)
			{
			// Hide
			case IPC_HIDE:
				progress_close(prog);
				break;

			// Show
			case IPC_SHOW:
				progress_open(prog);
				break;

			// Quit
			case IPC_QUIT:
				quit = 1;
				break;

			// Set parameters
			case PROGRESS_SET:
				progress_set(prog, (struct TagItem *)msg->data);
				break;

			// Get parameters
			case PROGRESS_GET:
				progress_get(prog, (struct TagItem *)msg->data);
				break;
			}

			// Reply to the message
			IPC_Reply(msg);
		}

		// Quit?
		if (quit)
			break;

		// Wait for messages
		Wait(1 << ipc->command_port->mp_SigBit | ((prog->pw_Window) ? 1 << prog->pw_Window->UserPort->mp_SigBit : 0));
	}

	// Close window
	progress_close(prog);

	// Free IPC data
	IPC_Free(ipc);

	// Free control structure
	FreeVec(prog);
}

// Open progress window
void progress_open(ProgressWindow *prog)
{
	struct IBox win;
	struct TextFont *font;
	struct Screen *wbscreen = 0, *screen;
	struct LibData *libdata;
	short a, last = -1;

	// Already open?
	if (prog->pw_Window)
	{
		// If we have an owner window, move in front of it
		if (prog->pw_OwnerWindow)
			MoveWindowInFrontOf(prog->pw_Window, prog->pw_OwnerWindow);
		return;
	}

	// Get library data pointer
	libdata = (struct LibData *)prog->pw_Lib->ml_UserData;

	// Got a screen to open on?
	if (!(screen = prog->pw_Screen) && !(prog->pw_OwnerWindow && (screen = prog->pw_OwnerWindow->WScreen)))
	{
		// Lock public screen
		if (!(wbscreen = LockPubScreen(0)))
			return;

		// Use this screen
		screen = wbscreen;
	}

	// Get font pointer
	font = screen->RastPort.Font;

	// Initial size
	win.Width = 0;
	win.Height = 0;

	// Clear coordinates
	for (a = 0; a < PROG_LAST; a++)
	{
		prog->pw_Coords[a].Left = 0;
		prog->pw_Coords[a].Top = 0;
		prog->pw_Coords[a].Width = 0;
		prog->pw_Coords[a].Height = 0;
	}

	// Want filename display?
	if (prog->pw_Flags & PWF_FILENAME)
	{
		// Get position
		prog->pw_Coords[PROG_NAME].Width = font->tf_XSize * 24;
		prog->pw_Coords[PROG_NAME].Height = font->tf_YSize;

		// Want size display?
		if (prog->pw_Flags & PWF_FILESIZE)
		{
			char *ptr;
			short len;

			// Get appropriate string
			ptr = GetString(&libdata->locale, (prog->pw_Flags & PWF_SWAP) ? MSG_FILE_FIT : MSG_SIZE_FIT);

			// Get length of byte string
			len = TextLength(&screen->RastPort, ptr, strlen(ptr));

			// Get position
			prog->pw_Coords[PROG_SIZE].Left = prog->pw_Coords[PROG_NAME].Left + prog->pw_Coords[PROG_NAME].Width + 32;
			prog->pw_Coords[PROG_SIZE].Width = len;
			prog->pw_Coords[PROG_SIZE].Height = font->tf_YSize;

			// Cache size string
			prog->pw_SizeString = GetString(&libdata->locale, (prog->pw_Flags & PWF_SWAP) ? MSG_FILE : MSG_SIZE);
			last = PROG_SIZE;
		}

		// Set last id
		else
			last = PROG_NAME;
	}

	// Want information display?
	if (prog->pw_Flags & PWF_INFO)
	{
		// Get position
		prog->pw_Coords[PROG_INFO].Top = (prog->pw_Flags & PWF_FILENAME) ? font->tf_YSize + 2 : 0;
		prog->pw_Coords[PROG_INFO].Width = 40 * font->tf_XSize;
		prog->pw_Coords[PROG_INFO].Height = font->tf_YSize;
		last = PROG_INFO;
	}

	// Want information display?
	if (prog->pw_Flags & PWF_INFO2)
	{
		// Get position
		prog->pw_Coords[PROG_INFO2].Top = prog->pw_Coords[last].Top + prog->pw_Coords[last].Height + 2;
		prog->pw_Coords[PROG_INFO2].Width = 40 * font->tf_XSize;
		prog->pw_Coords[PROG_INFO2].Height = font->tf_YSize;
		last = PROG_INFO2;
	}

	// Want information display?
	if (prog->pw_Flags & PWF_INFO3)
	{
		// Get position
		prog->pw_Coords[PROG_INFO3].Top = prog->pw_Coords[last].Top + prog->pw_Coords[last].Height + 2;
		prog->pw_Coords[PROG_INFO3].Width = 40 * font->tf_XSize;
		prog->pw_Coords[PROG_INFO3].Height = font->tf_YSize;
		last = PROG_INFO3;
	}

	// Want bar graph?
	if (prog->pw_Flags & PWF_GRAPH)
	{
		// Get position
		if (last > -1)
			prog->pw_Coords[PROG_GRAPH].Top = prog->pw_Coords[last].Top + prog->pw_Coords[last].Height + 3;
		prog->pw_Coords[PROG_GRAPH].Width = 40 * font->tf_XSize;
		prog->pw_Coords[PROG_GRAPH].Height = font->tf_YSize + 6;
		last = PROG_GRAPH;
	}

	// Get window size
	for (a = 0; a <= last; a++)
	{
		short x, y;

		// Get bottom-left coordinates of this item
		x = prog->pw_Coords[a].Left + prog->pw_Coords[a].Width - 1;
		y = prog->pw_Coords[a].Top + prog->pw_Coords[a].Height - 1;

		// Check against current window size
		if (win.Width < x)
			win.Width = x;
		if (win.Height < y)
			win.Height = y;
	}

	// Info can grow to fill window
	if (prog->pw_Flags & PWF_INFO)
	{
		// Check against width
		if (prog->pw_Coords[PROG_INFO].Width < win.Width)
			prog->pw_Coords[PROG_INFO].Width = win.Width;
	}

	// Info can grow to fill window
	if (prog->pw_Flags & PWF_INFO2)
	{
		// Check against width
		if (prog->pw_Coords[PROG_INFO2].Width < win.Width)
			prog->pw_Coords[PROG_INFO2].Width = win.Width;
	}

	// Info can grow to fill window
	if (prog->pw_Flags & PWF_INFO3)
	{
		// Check against width
		if (prog->pw_Coords[PROG_INFO3].Width < win.Width)
			prog->pw_Coords[PROG_INFO3].Width = win.Width;
	}

	// As can the graph
	if (prog->pw_Flags & PWF_GRAPH)
	{
		// Check against width
		if (prog->pw_Coords[PROG_GRAPH].Width < win.Width)
			prog->pw_Coords[PROG_GRAPH].Width = win.Width;
	}

	// Filename can too if there's no filesize
	if ((prog->pw_Flags & (PWF_FILENAME | PWF_FILESIZE)) == PWF_FILENAME)
	{
		// Check against width
		if (prog->pw_Coords[PROG_NAME].Width < win.Width)
			prog->pw_Coords[PROG_NAME].Width = win.Width;
	}

	// If there's a filesize, it should be right-justified
	if (prog->pw_Flags & PWF_FILESIZE)
	{
		// Right-justify
		prog->pw_Coords[PROG_SIZE].Left = win.Width - prog->pw_Coords[PROG_SIZE].Width;
	}

	// Want abort gadget?
	if ((prog->pw_SigTask || prog->pw_Flags & PWF_ABORT) && !(prog->pw_Flags & PWF_NOABORT))
	{
		char *ptr = GetString(&libdata->locale, MSG_ABORT);
		short len;

		// Get length of abort string
		len = TextLength(&screen->RastPort, ptr, strlen(ptr));

		// Get gadget size
		prog->pw_Coords[PROG_LAST].Width = len + 16;
		prog->pw_Coords[PROG_LAST].Height = font->tf_YSize + 6;

		// Got a graph?
		if (prog->pw_Flags & PWF_GRAPH)
		{
			// Shrink graph to fit progress gadget
			prog->pw_Coords[PROG_GRAPH].Width -= prog->pw_Coords[PROG_LAST].Width + 8;
		}

		// Otherwise, grow window by height of gadget
		else
		{
			win.Height += prog->pw_Coords[PROG_LAST].Height;
			if (win.Width < prog->pw_Coords[PROG_LAST].Width)
				win.Width = prog->pw_Coords[PROG_LAST].Width;
		}

		// Position gadget
		prog->pw_Coords[PROG_LAST].Left = win.Width - prog->pw_Coords[PROG_LAST].Width;
		prog->pw_Coords[PROG_LAST].Top = win.Height - prog->pw_Coords[PROG_LAST].Height + 1;
	}

	// Get border sizes
	prog->pw_Offset.x = 6;
	prog->pw_Offset.y = 6;

	// Add additional space to window
	win.Width += prog->pw_Offset.x << 1;
	win.Height += prog->pw_Offset.y << 1;

	// Add system border sizes
	win.Width += screen->WBorLeft + screen->WBorRight;
	win.Height += screen->WBorTop + screen->WBorBottom + font->tf_YSize + 1;

	// Is window position valid?
	if (prog->pw_PosValid)
	{
		// Get saved position
		win.Left = prog->pw_WindowPos.x;
		win.Top = prog->pw_WindowPos.y;
	}

	// Calculate window position
	else if (prog->pw_OwnerWindow)
	{
		// Center in owner window
		win.Left = prog->pw_OwnerWindow->LeftEdge + ((prog->pw_OwnerWindow->Width - win.Width) >> 1);
		win.Top = prog->pw_OwnerWindow->TopEdge + ((prog->pw_OwnerWindow->Height - win.Height) >> 1);
	}

	// No owner window
	else
	{
		// Center in screen
		win.Left = (screen->Width - win.Width) >> 1;
		win.Top = (screen->Height - win.Height) >> 1;
	}

	// Get backfill hook
	prog->pw_Backfill = L_LockReqBackFill(screen, prog->pw_Lib);

	// Open window
	prog->pw_Window = OpenWindowTags(0,
									 WA_Left,
									 win.Left,
									 WA_Top,
									 win.Top,
									 WA_Width,
									 win.Width,
									 WA_Height,
									 win.Height,
									 WA_Flags,
									 WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_RMBTRAP,
									 WA_IDCMP,
									 IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_REFRESHWINDOW | IDCMP_RAWKEY,
									 WA_SimpleRefresh,
									 TRUE,
									 WA_PubScreen,
									 screen,
									 WA_Title,
									 prog->pw_Title,
									 WA_WindowName,
									 DOPUS_WIN_NAME,
									 WA_AutoAdjust,
									 TRUE,
									 (prog->pw_Backfill) ? WA_BackFill : TAG_IGNORE,
									 prog->pw_Backfill,
									 TAG_DONE);

	// Unlock public screen if locked
	if (wbscreen)
		UnlockPubScreen(0, wbscreen);

	// Failed?
	if (!prog->pw_Window)
	{
		// Unlock backfill
		if (prog->pw_Backfill)
		{
			L_UnlockReqBackFill(prog->pw_Lib);
			prog->pw_Backfill = 0;
			return;
		}
	}

	// Set ID
	SetWindowID(prog->pw_Window, &prog->pw_ID, WINDOW_UNDEFINED, 0);

	// Activate window if owner was active
	if (prog->pw_OwnerWindow && prog->pw_OwnerWindow->Flags & WFLG_WINDOWACTIVE)
		ActivateWindow(prog->pw_Window);

	// Add offset to coordinates
	for (a = 0; a <= PROG_LAST; a++)
	{
		prog->pw_Coords[a].Left += prog->pw_Offset.x + prog->pw_Window->BorderLeft;
		prog->pw_Coords[a].Top += prog->pw_Offset.y + prog->pw_Window->BorderTop;
	}

	// Want abort gadget?
	if ((prog->pw_SigTask || prog->pw_Flags & PWF_ABORT) && !(prog->pw_Flags & PWF_NOABORT))
	{
		// Create abort gadget
		if ((prog->pw_Abort = (struct Gadget *)NewObject(0,
														 "dopusbuttongclass",
														 GA_Left,
														 prog->pw_Coords[PROG_LAST].Left,
														 GA_Top,
														 prog->pw_Coords[PROG_LAST].Top,
														 GA_Width,
														 prog->pw_Coords[PROG_LAST].Width,
														 GA_Height,
														 prog->pw_Coords[PROG_LAST].Height,
														 GA_RelVerify,
														 TRUE,
														 GA_Text,
														 GetString(&libdata->locale, MSG_ABORT),
														 GTCustom_ThinBorders,
														 TRUE,
														 ICA_TARGET,
														 ICTARGET_IDCMP,
														 TAG_DONE)))
		{
			// Add gadget to window
			AddGList(prog->pw_Window, prog->pw_Abort, -1, 1, 0);
			RefreshGList(prog->pw_Abort, prog->pw_Window, 0, 1);
		}
	}

	// Get DrawInfo pointer
	prog->pw_DrawInfo = GetScreenDrawInfo(prog->pw_Window->WScreen);

	// Set draw mode and font in window
	SetFont(prog->pw_Window->RPort, font);
	if (prog->pw_Backfill && ((PatternInstance *)prog->pw_Backfill)->pattern &&
		((PatternInstance *)prog->pw_Backfill)->pattern->valid)
	{
		// JAM1 mode
		SetDrMd(prog->pw_Window->RPort, JAM1);
	}

	// No backfill
	else
	{
		// JAM2 mode
		SetDrMd(prog->pw_Window->RPort, JAM2);
	}

	// Draw display
	progress_draw(prog, PWF_ALL);
}

// Close progress window
void progress_close(ProgressWindow *prog)
{
	// Window open?
	if (prog->pw_Window)
	{
		// Save position
		prog->pw_WindowPos.x = prog->pw_Window->LeftEdge;
		prog->pw_WindowPos.y = prog->pw_Window->TopEdge;
		prog->pw_PosValid = 1;

		// Free drawinfo
		FreeScreenDrawInfo(prog->pw_Window->WScreen, prog->pw_DrawInfo);

		// Close window
		CloseWindow(prog->pw_Window);
		prog->pw_Window = 0;

		// Free backfill hook
		if (prog->pw_Backfill)
		{
			// Unlock hook
			L_UnlockReqBackFill(prog->pw_Lib);
			prog->pw_Backfill = 0;
		}

		// Free gadget
		if (prog->pw_Abort)
		{
			DisposeObject((Object *)prog->pw_Abort);
			prog->pw_Abort = 0;
		}
	}
}

// Draw progress display
void progress_draw(ProgressWindow *prog, unsigned long flags)
{
	// Set pen for text
	SetAPen(prog->pw_Window->RPort, prog->pw_DrawInfo->dri_Pens[TEXTPEN]);

	// Filename display?
	if (prog->pw_Flags & PWF_FILENAME && flags & PWF_FILENAME)
	{
		// Do text
		progress_text(prog, prog->pw_FileName, &prog->pw_Coords[PROG_NAME], 0);
	}

	// File size display?
	if (prog->pw_Flags & PWF_FILESIZE && flags & PWF_FILESIZE && prog->pw_SizeString)
	{
		// Swapped with bar graph?
		if (prog->pw_Flags & PWF_SWAP)
		{
			// Do the bar
			progress_bar(prog, flags, prog->pw_FileSize, prog->pw_FileDone);

			// Set pen for text
			SetAPen(prog->pw_Window->RPort, prog->pw_DrawInfo->dri_Pens[TEXTPEN]);
		}

		// Normal display
		else
		{
			char buf[40];
			unsigned long pcent = 0;

			// Calculate file percentage
			if (prog->pw_FileSize > 0)
			{
				// Maximum?
				if (prog->pw_FileDone == (ULONG)-1)
					pcent = 1;

				// Calculate
				else if ((pcent = L_CalcPercent(prog->pw_FileDone, prog->pw_FileSize, UtilityBase)) > 100)
					pcent = 100;
			}

			// Build filesize string
			lsprintf(buf, prog->pw_SizeString, pcent);

			// Do text
			progress_text(prog, buf, &prog->pw_Coords[PROG_SIZE], 1);
		}
	}

	// Info display?
	if (prog->pw_Flags & PWF_INFO && flags & PWF_INFO)
	{
		// Do text
		progress_text(prog, prog->pw_Information, &prog->pw_Coords[PROG_INFO], 0);
	}

	// Info display?
	if (prog->pw_Flags & PWF_INFO2 && flags & PWF_INFO2)
	{
		// Do text
		progress_text(prog, prog->pw_Information2, &prog->pw_Coords[PROG_INFO2], 0);
	}

	// Info display?
	if (prog->pw_Flags & PWF_INFO3 && flags & PWF_INFO3)
	{
		// Do text
		progress_text(prog, prog->pw_Information3, &prog->pw_Coords[PROG_INFO3], 0);
	}

	// Bar graph?
	if (prog->pw_Flags & PWF_GRAPH && flags & PWF_GRAPH)
	{
		// Swapped with size display?
		if (prog->pw_Flags & PWF_SWAP)
		{
			char buf[40];

			// Build filesize string
			lsprintf(buf, prog->pw_SizeString, prog->pw_FileNum, prog->pw_FileCount);

			// Do text
			progress_text(prog, buf, &prog->pw_Coords[PROG_SIZE], 1);
		}

		// Normal display
		else
		{
			// Do the bar
			progress_bar(prog, flags, prog->pw_FileCount, prog->pw_FileNum);
		}
	}
}

// Text display
void progress_text(ProgressWindow *prog, char *text, struct IBox *coords, BOOL rightjust)
{
	struct TextExtent extent;
	struct RastPort *rp;
	short len, x;

	// Cache rastport pointer
	rp = prog->pw_Window->RPort;

	// Get length that will fit
	len = TextFit(rp, text, strlen(text), &extent, 0, 1, coords->Width, coords->Height);

	// Get x-position
	x = (rightjust) ? (coords->Left + coords->Width - extent.te_Width) : coords->Left;

	// Draw text
	if (len > 0)
	{
		// If in JAM1 mode, erase under text
		if (rp->DrawMode == JAM1)
		{
			// Erase under text
			EraseRect(rp, x, coords->Top, x + extent.te_Width - 1, coords->Top + rp->TxHeight - 1);
		}

		// Draw text
		Move(rp, x, coords->Top + rp->TxBaseline);
		Text(rp, text, len);
	}

	// Erase to end of area
	if (extent.te_Width < coords->Width)
	{
		EraseRect(rp,
				  (rightjust) ? coords->Left : coords->Left + extent.te_Width,
				  coords->Top,
				  coords->Left + coords->Width - ((rightjust) ? extent.te_Width + 1 : 1),
				  coords->Top + coords->Height - 1);
	}
}

// Set progress things
void progress_set(ProgressWindow *prog, struct TagItem *tags)
{
	unsigned long change = 0;
	struct TagItem *tag;
	char *ptr;
	UQUAD temp = 0LL;

	// Title change?
	if ((ptr = (char *)GetTagData(PW_Title, 0, tags)))
	{
		stccpy(prog->pw_Title, ptr, 39);
		if (prog->pw_Window)
			SetWindowTitles(prog->pw_Window, prog->pw_Title, (CONST_STRPTR)-1);
	}

	// Filename?
	if ((ptr = (char *)GetTagData(PW_FileName, 0, tags)))
	{
		stccpy(prog->pw_FileName, ptr, 79);
		change |= PWF_FILENAME;
	}

	// File size?
	if ((tag = FindTagItem(PW_FileSize, tags)))
	{
		prog->pw_FileSize = tag->ti_Data;
		change |= PWF_FILESIZE;
	}

	// 64bit file size?
	if ((tag = FindTagItem(PW_FileSize64, tags)))
	{
		temp = *(UQUAD *)tag->ti_Data;
		if (temp > 0x7FFFFFFF)
		{
			prog->pw_FileSize = (long)(temp >> 8);
			prog->pw_Scale = TRUE;
		}
		else
		{
			prog->pw_FileSize = (long)temp;
			prog->pw_Scale = FALSE;
		}
		change |= PWF_FILESIZE;
	}

	// File done?
	if ((tag = FindTagItem(PW_FileDone, tags)))
	{
		prog->pw_FileDone = tag->ti_Data;
		change |= PWF_FILESIZE;
		if (prog->pw_Flags & PWF_DEBUG)
			lsprintf(prog->pw_TaskName, "dopus_progressbar - %ld", prog->pw_FileDone);
	}

	if ((tag = FindTagItem(PW_FileDone64, tags)))
	{
		temp = *(UQUAD *)tag->ti_Data;
		if (prog->pw_Scale)
		{
			prog->pw_FileDone = (long)(temp >> 8);
		}
		else
		{
			prog->pw_FileDone = (long)temp;
		}
		change |= PWF_FILESIZE;
		if (prog->pw_Flags & PWF_DEBUG)
			lsprintf(prog->pw_TaskName, "dopus_progressbar - %lld", temp);	// prog->pw_FileDone);
	}

	// Information?
	if ((ptr = (char *)GetTagData(PW_Info, 0, tags)))
	{
		stccpy(prog->pw_Information, ptr, 79);
		change |= PWF_INFO;
	}

	// Information?
	if ((ptr = (char *)GetTagData(PW_Info2, 0, tags)))
	{
		stccpy(prog->pw_Information2, ptr, 79);
		change |= PWF_INFO2;
	}

	// Information?
	if ((ptr = (char *)GetTagData(PW_Info3, 0, tags)))
	{
		stccpy(prog->pw_Information3, ptr, 79);
		change |= PWF_INFO3;
	}

	// File count?
	if ((tag = FindTagItem(PW_FileCount, tags)))
	{
		prog->pw_FileCount = tag->ti_Data;
		prog->pw_ProgWidth = -1;
	}

	// File number?
	if ((tag = FindTagItem(PW_FileNum, tags)))
	{
		prog->pw_FileNum = tag->ti_Data;
		change |= PWF_GRAPH;
	}
	else if ((tag = FindTagItem(PW_FileInc, tags)))
	{
		prog->pw_FileNum += tag->ti_Data;
		change |= PWF_GRAPH;
	}

	// Change to display needed?
	if (change && prog->pw_Window)
	{
		// Refresh display
		progress_draw(prog, change);
	}
}

// Get progress things
void progress_get(ProgressWindow *prog, struct TagItem *tags)
{
	struct TagItem *tag;

	// All we can get so far is window pointer
	if ((tag = FindTagItem(PW_Window, tags)))
	{
		// Fill out window pointer
		if (tag->ti_Data)
			*((ULONG *)tag->ti_Data) = (ULONG)prog->pw_Window;
	}
}

// Bar display
void progress_bar(ProgressWindow *prog, unsigned long flags, long count, long num)
{
	unsigned long width = 0;
	struct RastPort *rp;
	unsigned long full;

	// Cache rastport pointer
	rp = prog->pw_Window->RPort;

	// Full-refresh, draw border too
	if (flags == PWF_ALL)
	{
		struct Rectangle rect;

		// Get rectangle
		rect.MinX = prog->pw_Coords[PROG_GRAPH].Left;
		rect.MinY = prog->pw_Coords[PROG_GRAPH].Top;
		rect.MaxX = rect.MinX + prog->pw_Coords[PROG_GRAPH].Width - 1;
		rect.MaxY = rect.MinY + prog->pw_Coords[PROG_GRAPH].Height - 1;

		// Draw border
		DrawBox(rp, &rect, prog->pw_DrawInfo, TRUE);
		prog->pw_ProgWidth = -1;
	}

	// Any files?
	if (count > 0)
	{
		// Get percentage
		if ((full = L_CalcPercent(num, count, UtilityBase)) > 0)
		{
			// Get width of fill area
			width = UDivMod32((prog->pw_Coords[PROG_GRAPH].Width - 2) * full, 100);

			// Bounds check width
			if (width > prog->pw_Coords[PROG_GRAPH].Width - 2)
				width = prog->pw_Coords[PROG_GRAPH].Width - 2;
			else if (width < 1)
				width = 1;
		}
	}

	// Has width changed?
	if (width != prog->pw_ProgWidth)
	{
		// Set pen for fill
		SetAPen(rp, prog->pw_DrawInfo->dri_Pens[FILLPEN]);

		// Fill bar
		if (width > 0)
			RectFill(rp,
					 prog->pw_Coords[PROG_GRAPH].Left + 1,
					 prog->pw_Coords[PROG_GRAPH].Top + 1,
					 prog->pw_Coords[PROG_GRAPH].Left + width,
					 prog->pw_Coords[PROG_GRAPH].Top + prog->pw_Coords[PROG_GRAPH].Height - 2);

		// Fill to end of graph
		if (prog->pw_Coords[PROG_GRAPH].Width > width + 4)
			EraseRect(rp,
					  prog->pw_Coords[PROG_GRAPH].Left + width + 1,
					  prog->pw_Coords[PROG_GRAPH].Top + 1,
					  prog->pw_Coords[PROG_GRAPH].Left + prog->pw_Coords[PROG_GRAPH].Width - 2,
					  prog->pw_Coords[PROG_GRAPH].Top + prog->pw_Coords[PROG_GRAPH].Height - 2);

		// Save width
		prog->pw_ProgWidth = width;
	}
}

// Calculate a percentage
long LIBFUNC L_CalcPercent(REG(d0, ULONG amount), REG(d1, ULONG total), REG(a0, struct Library *UtilityBase))
{
	long pcent;

	// If values are too large, cut them down
	if (amount > 1000000)
	{
		// Divide by 1000
		amount >>= 10;
		total >>= 10;
	}

	// Do calculation
	if (total == 0)
		pcent = 0;
	else
		pcent = UDivMod32((amount * 100), total);

	return pcent;
}
