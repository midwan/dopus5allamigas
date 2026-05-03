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

struct DosList *LIBFUNC L_DeviceFromLock(REG(a0, BPTR lock), REG(a1, char *name), REG(a6, struct MyLibrary *libbase))
{
	struct FileLock *fl;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Clear buffer
	if (name)
		*name = 0;

	// Get filelock pointer
	if (!(fl = (struct FileLock *)BADDR(lock)))
		return 0;

	// Pass to DeviceFromHandler
	return L_DeviceFromHandler(fl->fl_Task, name, libbase);
}

struct DosList *LIBFUNC L_DeviceFromHandler(REG(a0, struct MsgPort *port),
											REG(a1, char *name),
											REG(a6, struct MyLibrary *libbase))
{
	struct DosList *dos, *dos_list = 0;
	DeviceNode *node, *match_node = 0;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	struct LibData *data = (struct LibData *)libbase->ml_UserData;

	// No port?
	if (!port)
		return 0;

	// Clear buffer
	if (name)
		*name = 0;

	// Lock device list
	ObtainSemaphore(&data->device_list.lock);

	// Go through device list
	for (node = (DeviceNode *)data->device_list.list.lh_Head; node->node.ln_Succ;
		 node = (DeviceNode *)node->node.ln_Succ)
	{
		// Match message port
		if (node->dol_Task == port)
			break;
	}

	// Matched?
	if (node->node.ln_Succ)
		match_node = node;

	// Didn't match
	else
	{
		// Lock DOS list
		dos = LockDosList(LDF_DEVICES | LDF_READ);

		// Go through DOS list
		while ((dos = NextDosEntry(dos, LDF_DEVICES)))
		{
			char namebuf[34];

			// Valid device?
			if (!dos->dol_Task)
				continue;

			// Build name, tack on colon
			L_BtoCStr(dos->dol_Name, namebuf, 32);
			strcat(namebuf, ":");

			// Not already in device list?
			if (!(node = (DeviceNode *)FindName(&data->device_list.list, namebuf)))
			{
				// Allocate new device node
				if ((node = L_AllocMemH(data->memory, sizeof(DeviceNode))))
				{
					// Fill out node
					node->node.ln_Name = node->dol_Name;
					node->dos_ptr = dos;
					node->dol_Task = dos->dol_Task;
					strcpy(node->dol_Name, namebuf);

					// Add to list
					AddTail(&data->device_list.list, (struct Node *)node);

					// Is this actually the one we're looking for?
					if (node->dol_Task == port)
						match_node = node;
				}
			}

			// Mark node as ok
			if (node)
				node->flags |= DNF_OK;
		}

		// Unlock DOS list
		UnLockDosList(LDF_DEVICES | LDF_READ);

		// Go through device list again
		for (node = (DeviceNode *)data->device_list.list.lh_Head; node->node.ln_Succ;)
		{
			DeviceNode *next = (DeviceNode *)node->node.ln_Succ;

			// Anything not marked as OK gets removed
			if (!(node->flags & DNF_OK))
			{
				// Remove from list
				Remove((struct Node *)node);

				// Free entry
				L_FreeMemH(node);
			}

			// Get next entry
			node = next;
		}
	}

	// Got a match?
	if (match_node)
	{
		// Name pointer supplied?
		if (name)
		{
			// Copy name
			stccpy(name, match_node->dol_Name, 31);
		}

		// Save DosList pointer
		dos_list = match_node->dos_ptr;
	}

	// Unlock device list
	ReleaseSemaphore(&data->device_list.lock);

	// Return DosList pointer
	return dos_list;
}

BOOL LIBFUNC L_DevNameFromLockDopus(REG(d1, BPTR lock),
									REG(d2, char *buffer),
									REG(d3, long len),
									REG(a6, struct MyLibrary *libbase))
{
	char devicename[34], *temp, *ptr;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Clear buffer
	if (buffer)
		*buffer = 0;

	// No lock?
	if (!lock || !buffer)
		return 0;

	// Call DOS NameFromLock
	if (!(NameFromLock(lock, buffer, len)))
		return 0;

	// Get device name
	if (!(L_DeviceFromLock(lock, devicename, libbase)))
		return 1;

	// Allocate temporary buffer
	if (!(temp = AllocVec(len + strlen(devicename) + 1, MEMF_CLEAR)))
		return 1;

	// Copy device name to temporary buffer
	strcpy(temp, devicename);

	// Get pointer to path after device name
	if ((ptr = strchr(buffer, ':')))
	{
		// Copy path back onto end of device name
		strcat(temp, ptr + 1);

		// Copy into user buffer
		stccpy(buffer, temp, len);
	}

	// Free buffer
	FreeVec(temp);

	return 1;
}

BOOL LIBFUNC L_IsDiskDevice(REG(a0, struct MsgPort *port))
{
	D_S(struct InfoData, info)

	// Valid port?
	if (!port)
		return 0;

	// Send ACTION_DISKINFO packet to handler
	if (DoPkt(port, ACTION_DISK_INFO, MKBADDR(info), 0, 0, 0, 0))
	{
		// Check for valid DOS type
		if (info->id_DiskType != 0)
			return 1;
	}

	return 0;
}

BOOL LIBFUNC L_GetDeviceUnit(REG(a0, BPTR dol_Startup), REG(a1, char *device), REG(a2, short *unit))
{
	struct FileSysStartupMsg *fssm;
	struct DosEnvec *de;
	char *devptr;

	// Clear buffer
	if (device)
		*device = 0;
	if (unit)
		*unit = 0;

	// Get real pointer
	if (!(fssm = (struct FileSysStartupMsg *)BADDR(dol_Startup)))
		return 0;

	// Valid memory?
	if (TypeOfMem(fssm) == 0)
		return 0;

	// Get device pointer
	devptr = (char *)BADDR(fssm->fssm_Device);

	// Valid memory?
	if (TypeOfMem(devptr) == 0 || *devptr == 0)
		return 0;

	// Get envec pointer
	de = BADDR(fssm->fssm_Environ);

	// Check if it's valid
	if (*(UBYTE *)fssm == 0 && de && (de->de_TableSize & 0xffffff00) == 0)
	{
		// Skip invalid characters
		while (*devptr && *devptr < ' ')
			++devptr;

		// Assume it's valid
		if (device)
			strcpy(device, devptr);
		if (unit)
			*unit = fssm->fssm_Unit;

		return TRUE;
	}

	return 0;
}
