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

// Display the progress indicator
void lister_progress_on(Lister *lister, ProgressPacket *packet)
{
	// Valid window?
	if (!lister->window)
		return;

	// Is the progress indicator already displayed?
	if (lister->progress_window)
	{
		// Close existing window
		CloseProgressWindow(lister->progress_window);
	}

	// Open progress indicator
	lister->progress_window = OpenProgressWindowTags(PW_Window,
													 lister->window,
													 PW_Title,
													 packet->operation,
													 PW_FileCount,
													 packet->total,
													 PW_Flags,
													 packet->flags,
													 PW_SigTask,
													 lister->ipc->proc,
													 PW_SigBit,
													 lister->abort_signal,
													 TAG_END);
}

// Remove progress indicator
void lister_progress_off(Lister *lister)
{
	CloseProgressWindow(lister->progress_window);
	lister->progress_window = 0;
}

// Set progress total
void lister_progress_total(Lister *lister, long total, long count)
{
	SetProgressWindowTags(lister->progress_window, PW_FileCount, total, PW_FileNum, count, TAG_END);
}

// Set file total
void lister_progress_filetotal(Lister *lister, long total)
{
	SetProgressWindowTags(lister->progress_window, PW_FileSize, total, TAG_END);
}

// Set 64bit file total
void lister_progress_filetotal64(Lister *lister, QUAD *total)
{
	SetProgressWindowTags(lister->progress_window, PW_FileSize64, (ULONG)total, TAG_END);
}

// Update file progress
void lister_progress_fileprogress(Lister *lister, long count)
{
	SetProgressWindowTags(lister->progress_window, PW_FileDone, count, TAG_END);
}

// Set file progress
void lister_progress_file(Lister *lister, long total, long count)
{
	SetProgressWindowTags(lister->progress_window, PW_FileSize, total, PW_FileDone, count, TAG_END);
}

// Update file progress info
void lister_progress_info(Lister *lister, char *info)
{
	SetProgressWindowTags(lister->progress_window, PW_Info, info, TAG_END);
}

// Update progress title
void lister_progress_title(Lister *lister, char *info)
{
	SetProgressWindowTags(lister->progress_window, PW_Title, info, TAG_END);
}
