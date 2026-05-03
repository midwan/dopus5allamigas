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

#ifndef _DOPUS_BUFIO
#define _DOPUS_BUFIO

/*****************************************************************************

 Buffered IO

 *****************************************************************************/

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

#ifndef DOS_DOS_H
	#include <dos/dos.h>
#endif

long CloseBuf(APTR file);
long ExamineBuf(APTR file, struct FileInfoBlock *fib);
BPTR FHFromBuf(APTR file);
long FlushBuf(APTR file);
APTR OpenBuf(char *name, long mode, long buffer_size);
long ReadBuf(APTR file, char *data, long size);
long ReadBufLine(APTR file, char *data, long size);
long SeekBuf(APTR file, long offset, long mode);
long WriteBuf(APTR file, char *data, long size);

#endif
