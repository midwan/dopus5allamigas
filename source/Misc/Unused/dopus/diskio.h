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

#ifndef _DOPUS_DISKIO
#define _DOPUS_DISKIO

/*****************************************************************************

 Disk I/O

 *****************************************************************************/

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct MsgPort *dh_Port;			   // Message port
	struct IOExtTD *dh_IO;				   // IO request
	struct FileSysStartupMsg *dh_Startup;  // Startup message
	struct DosEnvec *dh_Geo;			   // Disk geometry
	char dh_Name[32];					   // Disk name
	char dh_Device[32];					   // Device name
	struct InfoData dh_Info;			   // Disk information
	unsigned long dh_Result;			   // dh_Info is valid
	unsigned long dh_Root;				   // Root block
	unsigned long dh_BlockSize;			   // Block size
	struct DateStamp dh_Stamp;			   // not used
} DiskHandle;
#ifndef __amigaos3__
	#pragma pack()
#endif

// Some third-party DOS types
#define ID_AFS_PRO 0x41465301
#define ID_AFS_USER 0x41465302
#define ID_AFS_MULTI 0x6D754146
#define ID_PFS_FLOPPY 0x50465300
#define ID_PFS_HARD 0x50465301

DiskHandle *OpenDisk(char *, struct MsgPort *);
void CloseDisk(DiskHandle *);

#endif
