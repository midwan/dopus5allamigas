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
#ifndef _DOS_PATCH_H
#define _DOS_PATCH_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

struct FileInfoBlock *dospatch_fib(BPTR, struct MyLibrary *, BOOL);

struct FileHandleWrapper
{
	struct MinNode fhw_Node;
	BPTR fhw_FileHandle;
	ULONG fhw_Flags;
	BPTR fhw_Parent;
};

#define FHWF_WRITTEN (1 << 0)  // File has been written to

struct FileHandleWrapper *find_filehandle(BPTR file, struct LibData *data);

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
