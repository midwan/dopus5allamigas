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

#undef FULL_REPLACE

enum { REPLACE_ABORT = -1, REPLACE_LEAVE, REPLACE_REPLACE };

enum {
	REPLACE_GAD_ABORT,
	REPLACE_GAD_REPLACE,
	REPLACE_GAD_REPLACE_ALL,
	REPLACE_GAD_SKIP,
	REPLACE_GAD_SKIP_ALL,
	REPLACE_GAD_VERSION,
};

#define REPLACE_ALL 2

#define REPREQF_NOVERSION (1 << 16)	 // No 'version' button

long SmartAskReplace(struct Window *,
					 struct Screen *,
					 IPCData *,
					 BPTR,
					 struct FileInfoBlock *,
					 BPTR,
					 struct FileInfoBlock *,
					 long);
