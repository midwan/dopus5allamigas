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
#ifndef _DISKCHANGE_H
#define _DISKCHANGE_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	struct Interrupt interrupt;
	struct MsgPort *port;
	struct IOExtTD *req[4];
	unsigned long count[4];
	unsigned long flags;
} DiskChangeData;

#ifndef __amigaos3__
	#pragma pack()
#endif

#define DCF_DISK0 (1 << 0)
#define DCF_DISK1 (1 << 1)
#define DCF_DISK2 (1 << 2)
#define DCF_DISK3 (1 << 3)

void init_diskchange(DiskChangeData *);
void free_diskchange(DiskChangeData *);
void __asm diskchange_code(register __a1 struct Task *);
unsigned long diskchange_check(DiskChangeData *);

#endif