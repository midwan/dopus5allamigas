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

#include "diskinfo.h"
#include "math_replace.h"

// Show disk information
int LIBFUNC L_Module_Entry(REG(a0, struct List *files),
						   REG(a1, struct Screen *screen),
						   REG(a2, IPCData *ipc),
						   REG(a3, IPCData *main_ipc),
						   REG(d0, ULONG mod_id),
						   REG(d1, EXT_FUNC(func_callback)))
{
	diskinfo_data *data;

	// Allocate data
	if (!(data = AllocVec(sizeof(diskinfo_data), MEMF_CLEAR)))
		return 0;

	// Path supplied?
	if (files && !(IsListEmpty(files)))
	{
		// Copy path
		strcpy(data->path, files->lh_Head->ln_Name);
	}

	// Otherwise
	else
	{
		// Get path
		if (!(func_callback(EXTCMD_GET_SOURCE, IPCDATA(ipc), data->path)))
		{
			FreeVec(data);
			return 0;
		}

		// Unlock listers
		func_callback(EXTCMD_UNLOCK_SOURCE, IPCDATA(ipc), 0);
	}

	// Get decimal separator
	data->decimal_sep = (locale->li_Locale) ? locale->li_Locale->loc_GroupSeparator[0] : ',';

	// Fill out new window
	data->new_win.parent = screen;
	data->new_win.dims = &diskinfo_win;
	data->new_win.title = 0;
	data->new_win.locale = locale;
	data->new_win.flags = WINDOW_SCREEN_PARENT | WINDOW_REQ_FILL | WINDOW_AUTO_KEYS | WINDOW_VISITOR;

	// Open window
	if (!(data->window = OpenConfigWindow(&data->new_win)) ||
		!(data->objlist = AddObjectList(data->window, diskinfo_objects)))
	{
		// Failed
		CloseConfigWindow(data->window);
		FreeVec(data);
		return 0;
	}

	// Initialise Area
	InitArea(&data->areainfo, data->areabuf, AREAVERTEX);
	data->window->RPort->AreaInfo = &data->areainfo;

	// OS 39?
	if (((struct Library *)GfxBase)->lib_Version >= 39)
	{
		short a, fail = 0;
		struct TagItem tags[2];

		// Set tags
		tags[0].ti_Tag = OBP_FailIfBad;
		tags[0].ti_Data = TRUE;
		tags[1].ti_Tag = TAG_END;

		// Try to allocate pens
		for (a = 0; a < 4; a++)
		{
			// Allocate it
			if ((a >= 2 && fail) || (data->pen_alloc[a] = ObtainBestPenA(data->window->WScreen->ViewPort.ColorMap,
																		 diskinfo_colours[a][0] << 24,
																		 diskinfo_colours[a][1] << 24,
																		 diskinfo_colours[a][2] << 24,
																		 tags)) == -1)
			{
				// Couldn't allocate; is this a shadow pen?
				if (a >= 2)
				{
					// Use main pen
					data->pens[a] = data->pens[a - 2];
				}

				// Otherwise, find best pen
				else
					data->pens[a] = FindColor(data->window->WScreen->ViewPort.ColorMap,
											  diskinfo_colours[a][0] << 24,
											  diskinfo_colours[a][1] << 24,
											  diskinfo_colours[a][2] << 24,
											  -1);

				// Clear allocation
				data->pen_alloc[a] = -1;

				// Set 'fail' flag
				fail = 1;
			}

			// Use it
			else
				data->pens[a] = data->pen_alloc[a];
		}
	}

	// Otherwise
	else
	{
		// Get default pens
		data->pens[USEDPEN] = PEN_C2;
		data->pens[FREEPEN] = PEN_C3;
		data->pens[USEDSHADOWPEN] = PEN_C2;
		data->pens[FREESHADOWPEN] = PEN_C3;
	}

	// Fill the key areas
	DisplayObject(data->window, GetObject(data->objlist, GAD_USED_KEY), 0, data->pens[USEDPEN], 0);
	DisplayObject(data->window, GetObject(data->objlist, GAD_FREE_KEY), 0, data->pens[FREEPEN], 0);

	// Show disk information
	diskinfo_info(data);

	// Loop for messages
	FOREVER
	{
		struct IntuiMessage *msg;
		IPCMessage *imsg;
		BOOL break_flag = 0;

		// IPC messages?
		while ((imsg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Quit?
			if (imsg->command == IPC_QUIT || imsg->command == IPC_HIDE || imsg->command == IPC_ABORT)
				break_flag = 1;
			IPC_Reply(imsg);
		}

		// Any messages?
		while ((msg = GetWindowMsg(data->window->UserPort)))
		{
			struct IntuiMessage msg_copy;

			// Copy message and reply
			msg_copy = *msg;
			ReplyWindowMsg(msg);

			// Look at message
			switch (msg_copy.Class)
			{
			case IDCMP_GADGETUP:

				// Ok?
				if (((struct Gadget *)msg_copy.IAddress)->GadgetID == GAD_OK)
				{
					// Get name
					strcpy(data->buffer, (char *)GetGadgetValue(data->objlist, GAD_NAME));

					// Has name changed?
					if (strcmp(data->buffer, data->volume))
					{
						// Try to relabel
						if (!(Relabel(data->path, data->buffer)))
							DisplayBeep(data->window->WScreen);
					}
				}

				// Not cancel?
				else if (((struct Gadget *)msg_copy.IAddress)->GadgetID != GAD_CANCEL)
					break;

			case IDCMP_CLOSEWINDOW:

				// Break out
				break_flag = 1;
				break;

			// Key press
			case IDCMP_RAWKEY:

				// Help?
				if (msg_copy.Code == 0x5f && !(msg_copy.Qualifier & VALID_QUALIFIERS))
				{
					// Valid main IPC?
					if (main_ipc)
					{
						// Set busy pointer
						SetWindowBusy(data->window);

						// Send help request
						IPC_Command(main_ipc, IPC_HELP, (1 << 31), "DiskInfo", 0, (struct MsgPort *)-1);

						// Clear busy pointer
						ClearWindowBusy(data->window);
					}
				}
				break;
			}
		}

		// Look at break flag
		if (break_flag)
			break;

		// Wait for messages
		Wait(1 << data->window->UserPort->mp_SigBit | 1 << ipc->command_port->mp_SigBit);
	}

	// Under 39?
	if (((struct Library *)GfxBase)->lib_Version >= 39)
	{
		short a;

		// Free pens
		for (a = 0; a < 4; a++)
			if (data->pen_alloc[a] > -1)
				ReleasePen(data->window->WScreen->ViewPort.ColorMap, data->pen_alloc[a]);
	}

	// Close window
	CloseConfigWindow(data->window);

	// Free data
	FreeVec(data);
	return 1;
}

// Show disk information
BOOL diskinfo_info(diskinfo_data *data)
{
	BPTR lock;
	struct DosList *doslist, *device;
	ULONG disktype, size;
#ifdef USE_64BIT
	UQUAD capacity, used;
#else
	ULONG capacity, used;
#endif
	struct Rectangle rect;
	short id;

	// Lock disk
	if (!(lock = Lock(data->path, ACCESS_READ)))
		return 0;

	// Get disk info
	Info(lock, &data->info);

	// Get volume node pointer
	doslist = (struct DosList *)BADDR(data->info.id_VolumeNode);

	// Get disk type from DOS list if it's set, otherwise get it from Info

#warning on some os3 setup info.id_DiskType show OFS always, so we use doslists ones first.
	// on some OS3 setup, data->info.id_DiskType always wrong and show that partitions are OFS (while the same code fine
	// on OS4)
	// TODO: investigate why.
	if (!(disktype = doslist->dol_misc.dol_volume.dol_DiskType))
	{
		disktype = data->info.id_DiskType;
	}

	// Get device name
	device = DeviceFromLock(lock, data->path);

	// Build title and set it
	lsprintf(data->title, GetString(locale, MSG_DISKINFO_TITLE), (IPTR)doslist->dol_Name, (IPTR)data->path);
	SetWindowTitles(data->window, data->title, (char *)-1);

	// Fill out name field
	lsprintf(data->volume, "%b", (IPTR)doslist->dol_Name);
	SetGadgetValue(data->objlist, GAD_NAME, (ULONG)data->volume);

	// Display type
	get_dostype_string(disktype, data->buffer);
	SetGadgetValue(data->objlist, GAD_TYPE, (ULONG)data->buffer);

	// Clear buffer
	*data->buffer = 0;

	// Got device?
	if (device)
	{
		struct FileSysStartupMsg *startup;

		// Got startup message?
		if ((startup = (struct FileSysStartupMsg *)BADDR(device->dol_misc.dol_handler.dol_Startup)))
		{
			// Get device name
			lsprintf(data->buffer + 128, "%b", (IPTR)startup->fssm_Device);
			lsprintf(data->buffer,
					 "%s, %s %ld",
					 (IPTR)data->buffer + 128,
					 (IPTR)GetString(locale, MSG_UNIT),
					 startup->fssm_Unit);
		}
	}

	// Didn't get anything?
	if (!*data->buffer)
	{
		// Get name of task
		if (device && ((struct Task *)device->dol_Task->mp_SigTask)->tc_Node.ln_Name)
			strcpy(data->buffer, ((struct Task *)device->dol_Task->mp_SigTask)->tc_Node.ln_Name);

		// Unknown handler
		else
			strcpy(data->buffer, GetString(locale, MSG_UNKNOWN));
	}

	// Display handler
	SetGadgetValue(data->objlist, GAD_HANDLER, (ULONG)data->buffer);

	// Get state
	switch (data->info.id_DiskState)
	{
	// Validating
	case ID_VALIDATING:
		id = MSG_VALIDATING;
		break;

	// Write protected
	case ID_WRITE_PROTECTED:
		id = MSG_READ_ONLY;

		// Disable name field
		DisableObject(data->objlist, GAD_NAME, TRUE);
		break;

	// Read/write
	default:
		id = MSG_READ_WRITE;
		break;
	}

	// Display state
	SetGadgetValue(data->objlist, GAD_STATE, (ULONG)GetString(locale, id));

	// Show used space
#ifdef USE_64BIT
	used = (UQUAD)data->info.id_NumBlocksUsed * (UQUAD)data->info.id_BytesPerBlock;
#else
	used = data->info.id_NumBlocksUsed * data->info.id_BytesPerBlock;
#endif
	diskinfo_show_space(data, used, GAD_USED, GAD_USED_MB);

	// Get capacity; for RAM we use available memory
	if (stricmp(data->path, "RAM:") == 0)
	{
		// Get total memory
		capacity = AvailMem(0);

		// Add on what we've already used
		capacity += used;
	}

	// Otherwise, use real data
	else
	{
		// Get total size
#ifdef USE_64BIT
		capacity = (UQUAD)data->info.id_NumBlocks * (UQUAD)data->info.id_BytesPerBlock;
#else
		capacity = data->info.id_NumBlocks * data->info.id_BytesPerBlock;
#endif
	}

	// Show free space
	diskinfo_show_space(data, capacity - used, GAD_FREE, GAD_FREE_MB);

	// Show capacity
	diskinfo_show_space(data, capacity, GAD_CAPACITY, GAD_CAPACITY_MB);

	// Get graph rectangle
	GetObjectRect(data->objlist, GAD_GRAPH, &rect);

	// Allocate TmpRas buffer
	size = RASSIZE((RECTWIDTH(&rect) + 16), (RECTHEIGHT(&rect) + 1));
	if ((data->rasbuf = AllocVec(size, MEMF_CHIP)))
	{
		// Initialise TmpRas
		InitTmpRas(&data->tmpras, data->rasbuf, size);
		data->window->RPort->TmpRas = &data->tmpras;

		// Show graph
		diskinfo_show_graph(data, &rect, used, capacity);

		// Free TmpRas
		data->window->RPort->TmpRas = 0;
		FreeVec(data->rasbuf);
	}

	// Any errors?
	if (data->info.id_NumSoftErrors > 0)
	{
		// Show error string
		lsprintf(data->buffer,
				 GetString(locale, (data->info.id_NumSoftErrors > 1) ? MSG_ERRORS : MSG_ERROR),
				 data->info.id_NumSoftErrors);
		SetGadgetValue(data->objlist, GAD_ERRORS, (ULONG)data->buffer);
	}

	// Release lock
	UnLock(lock);
	return 1;
}

// Get DOS type as a string
void get_dostype_string(ULONG disktype, char *buffer)
{
	short a;
	char c;

	D(bug("disktype : %lx (%lx %lx %lx %lx)\n",
		  disktype,
		  (disktype >> 24) & 0xff,
		  (disktype >> 16) & 0xff,
		  (disktype >> 8) & 0xff,
		  disktype & 0xff));
	// Go through lookup table
	for (a = 0; disktype_lookup[a]; a += 2)
	{
		// Match ID
		if (disktype == disktype_lookup[a])
		{
			// Get string
			strcpy(buffer, GetString(locale, disktype_lookup[a + 1]));
			return;
		}
	}

	// Get filesystem string
	buffer[0] = (char)((disktype >> 24) & 0xff);
	buffer[1] = (char)((disktype >> 16) & 0xff);
	buffer[2] = (char)((disktype >> 8) & 0xff);

	// Last character might be either a number or a letter
	c = (char)(disktype & 0xff);
	if (c < 10)
		c += '0';
	buffer[3] = c;
	buffer[4] = 0;
}

// Show space
#ifdef USE_64BIT
void diskinfo_show_space(diskinfo_data *data, UQUAD bytes, short id_bytes, short id_mb)
#else
void diskinfo_show_space(diskinfo_data *data, unsigned long bytes, short id_bytes, short id_mb)
#endif
{
	// Get bytes string
#ifdef USE_64BIT
	ItoaU64(&bytes, data->buffer, sizeof(data->buffer), data->decimal_sep);
#else
	ItoaU(bytes, data->buffer, data->decimal_sep);
#endif
	strcat(data->buffer, GetString(locale, MSG_BYTES));
	SetGadgetValue(data->objlist, id_bytes, (ULONG)data->buffer);

	// More than a kilobyte?
	if (bytes > 1023 || bytes == 0)
	{
		// Get mb string
#ifdef USE_64BIT
		if (bytes <= 1023)
			bytes = 0;
		BytesToString64(&bytes, data->buffer, sizeof(data->buffer), 1, data->decimal_sep);
#else
		BytesToString((bytes > 1023) ? bytes : 0, data->buffer, 1, data->decimal_sep);
#endif
	}

	// Use default 1K string
	else
		strcpy(data->buffer, "1K");
	SetGadgetValue(data->objlist, id_mb, (ULONG)data->buffer);
}

// Show graph
#ifdef USE_64BIT
void diskinfo_show_graph(diskinfo_data *data, struct Rectangle *rect, UQUAD size, UQUAD total)
#else
void diskinfo_show_graph(diskinfo_data *data, struct Rectangle *rect, ULONG size, ULONG total)
#endif
{
	long p, xp, yp, height, a, b, cx, cy, pc, y, chunkx = 0, chunky = 0;
	FLOAT rads, rsin, rcos, rx, ry, pcent;
	struct RastPort *rp;
	short step, stop;

	// Reduce values if more than 2gb to fix rounding problems
#ifdef USE_64BIT
	// while (total&(UQUAD)/*-2147483648*/0xffffffff80000000)
	while ((total >> 32) & 0xffffffff || total & (1 << 31))
#else
	if (total & (1 << 31))
#endif
	{
		total >>= 1;
		size >>= 1;
	}

	// Set pen
	rp = data->window->RPort;
	SetAPen(rp, DRAWINFO(data->window)->dri_Pens[TEXTPEN]);

	// Get height of pie, minimum 10
	height = rp->TxHeight;
	if (height < 10)
		height = 10;

	// Get center and radii
	a = (RECTWIDTH(rect)) >> 1;
	cx = rect->MinX + a;
	rx = SPFlt(a);
	b = ((RECTHEIGHT(rect) - height)) >> 1;
	cy = rect->MinY + b;
	ry = SPFlt(b);

	// Get size of chunk as a percentage of the total, and convert to degrees
	pcent = (total < 1) ? (FLOAT)0 : SPMul(SPDiv(SPFlt((LONG)total), SPFlt((LONG)size)), (FLOAT)360);
	pcent = SPSub(pcent, (FLOAT)180);

	// Get as long and round to nearest 2
	pc = SPFix(pcent);
	if (pc & 1)
		++pc;
	if (pc > 178)
		pc = -180;
	else if (pc < -177)
		pc = -179;

	// Set pen for areafill
	SetOPen(rp, rp->FgPen);
	SetAPen(rp, data->pens[(pc == -180) ? FREEPEN : USEDPEN]);

	// Draw start
	AreaMove(rp, (pc <= -179) ? rect->MinX : cx, cy);

	// Draw ellipse
	for (p = 180; p >= -180; p -= 2)
	{
		// Convert degrees to radians
		rads = SPDiv((FLOAT)180, SPMul((FLOAT)PI, SPFlt(p)));

		// Get sine and cosine
		rsin = SPSincos(&rcos, rads);

		// Get coordinates
		xp = SPFix(SPMul(rx, rcos));
		yp = SPFix(SPMul(ry, rsin));

		// Draw line
		AreaDraw(rp, cx + xp, cy + yp);

		// Is this the chunk position?
		if (p == pc)
		{
			// End this area
			AreaEnd(rp);

			// Save position
			chunkx = cx + xp;
			chunky = cy + yp;

			// All full?
			if (pc <= -179)
				break;

			// Begin for new area
			AreaMove(rp, cx, cy);
			AreaDraw(rp, chunkx, chunky);

			// Set new colour
			SetAPen(rp, data->pens[FREEPEN]);
		}
	}

	// End area
	AreaEnd(rp);

	// Start for bottom rim
	SetAPen(rp, data->pens[(pc == -180) ? FREESHADOWPEN : USEDSHADOWPEN]);
	AreaMove(rp, rect->MinX, cy);

	// Draw bottom rim of ellipse
	p = 180;
	step = -2;
	y = cy + height;
	stop = 180;
	while (1)
	{
		// Convert degrees to radians
		rads = SPDiv((FLOAT)180, SPMul((FLOAT)PI, SPFlt(p)));

		// Get sine and cosine
		rsin = SPSincos(&rcos, rads);

		// Get coordinates
		xp = SPFix(SPMul(rx, rcos));
		yp = SPFix(SPMul(ry, rsin));

		// Draw line
		AreaDraw(rp, cx + xp, yp + y);

		// Finished?
		if (step > 0 && p == stop)
		{
			// End this area
			AreaEnd(rp);

			// Done the second bit?
			if (stop < 180 || pc <= 0)
				break;

			// Set new stop position
			stop = pc;
			step = -2;

			// New chunk position is at the end
			p = pc;
			pc = 0;

			// Reset y-position
			y = cy + height;

			// Start for new chunk
			SetAPen(rp, data->pens[FREESHADOWPEN]);
			AreaMove(rp, chunkx, chunky);
			continue;
		}

		// Is this the chunk position?
		if (step < 0 && (p == pc || p <= 0))
		{
			// Move upwards and start going back
			y -= height;
			step = 2;
		}

		// Increment position
		else
			p += step;
	}
}
