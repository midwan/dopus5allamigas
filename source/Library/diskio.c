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

// Open a disk for IO
DiskHandle *LIBFUNC L_OpenDisk(REG(a0, char *disk), REG(a1, struct MsgPort *port))
{
	DiskHandle *handle;
	struct DosList *dl;
	unsigned long blocks;
	char *ptr;

	// Allocate handle
	if (!(handle = AllocVec(sizeof(DiskHandle), MEMF_CLEAR)))
		return 0;

	// Copy name, strip colon
	stccpy(handle->dh_name, disk, 31);
	if ((ptr = strchr(handle->dh_name, ':')))
		*ptr = 0;

	// Lock DOS list
	dl = LockDosList(LDF_DEVICES | LDF_READ);

	// Find entry
	if (!(dl = FindDosEntry(dl, handle->dh_name, LDF_DEVICES)))
	{
		// Not found
		UnLockDosList(LDF_DEVICES | LDF_READ);
		FreeVec(handle);
		return 0;
	}

	// Add colon back on
	strcat(handle->dh_name, ":");

	// Get pointer to startup message and geometry
	handle->dh_startup = (struct FileSysStartupMsg *)BADDR(dl->dol_misc.dol_handler.dol_Startup);
	handle->dh_geo = (struct DosEnvec *)BADDR(handle->dh_startup->fssm_Environ);

	// Shortcuts to root block and blocksize
	blocks = handle->dh_geo->de_BlocksPerTrack * handle->dh_geo->de_Surfaces *
			 (handle->dh_geo->de_HighCyl - handle->dh_geo->de_LowCyl + 1);
	handle->dh_root = (blocks - 1 + handle->dh_geo->de_Reserved) >> 1;
	handle->dh_blocksize = handle->dh_geo->de_SizeBlock << 2;

	// Get real device name
	L_BtoCStr((BPTR)handle->dh_startup->fssm_Device, handle->dh_device, 32);

	// Get information
#ifdef __AROS__
	if (((struct Library *)DOSBase)->lib_Version < 50)
	{
		// handle->dh_result=Info(dl->dol_Lock,&handle->dh_info);
		BPTR lock;
		handle->dh_result = 0;
		if ((lock = Lock(handle->dh_name, SHARED_LOCK)))
		{
			handle->dh_result = Info(lock, &handle->dh_info);
			UnLock(lock);
		}
	}
	else
#endif
		handle->dh_result = DoPkt(dl->dol_Task, ACTION_DISK_INFO, MKBADDR(&handle->dh_info), 0, 0, 0, 0);

	// Unlock dos list
	UnLockDosList(LDF_DEVICES | LDF_READ);

	// Create message port if needed
	if (!port)
	{
		if (!(handle->dh_port = CreateMsgPort()))
		{
			FreeVec(handle);
			return 0;
		}
		port = handle->dh_port;
	}

	// Create IO request
	if (!(handle->dh_io = (struct IOExtTD *)CreateIORequest(port, sizeof(struct IOExtTD))))
	{
		if (handle->dh_port)
			DeleteMsgPort(handle->dh_port);
		FreeVec(handle);
		return 0;
	}

	// Open device
	if (OpenDevice(handle->dh_device,
				   handle->dh_startup->fssm_Unit,
				   (struct IORequest *)handle->dh_io,
				   handle->dh_startup->fssm_Flags))
	{
		// Failed to open
		L_CloseDisk(handle);
		return 0;
	}

	return handle;
}

// Close a disk
void LIBFUNC L_CloseDisk(REG(a0, DiskHandle *handle))
{
	if (handle)
	{
		// IO request?
		if (handle->dh_io)
		{
			// Device open?
			if (handle->dh_io->iotd_Req.io_Device)
				CloseDevice((struct IORequest *)handle->dh_io);

			// Delete IO request
			DeleteIORequest((struct IORequest *)handle->dh_io);

			// Delete message port
			if (handle->dh_port)
				DeleteMsgPort(handle->dh_port);
		}

		// Free handle
		FreeVec(handle);
	}
}
