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

#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct device_data
{
	char name[80];			// Disk name
	char full[8];			// Percent full string
	char free[27];			// Free space string
	char used[27];			// Used space string
	struct DosList *dos;	// DOS list pointer
	struct DateStamp date;	// Date
	short valid;			// Validating?
	char fsys[6];			// File system
};
#ifndef __amigaos3__
	#pragma pack()
#endif

enum { ARG_NEW, ARG_FULL, ARG_BRIEF };

static void devlist_append_long(char *buffer, LONG value)
{
	char number[16];
	char *ptr;
	ULONG val;
	BOOL negative = 0;

	ptr = number + sizeof(number) - 1;
	*ptr = 0;

	if (value < 0)
	{
		negative = 1;
		val = (ULONG)(-(value + 1)) + 1;
	}
	else
		val = (ULONG)value;

	do
	{
		*--ptr = (char)('0' + (val % 10));
		val /= 10;
	} while (val);

	if (negative)
		*--ptr = '-';

	strcat(buffer, ptr);
}

static void devlist_append_column(char *buffer, LONG position)
{
	strcat(buffer, "\t\b");
	devlist_append_long(buffer, position);
	strcat(buffer, "\b");
}

// DEVICELIST internal function
DOPUS_FUNC(function_devicelist)
{
	Lister *lister;
	short type;
	struct DosList *doslist, *list_entry;
	static short type_order[3] = {LDF_DEVICES, LDF_VOLUMES, LDF_ASSIGNS};
	DirEntry *add_after = 0;
	char device_name[80];
	struct List list[3];
	struct Node *node;
	DirBuffer *custom = 0;

	// New lister?
	if (instruction->funcargs && instruction->funcargs->FA_Arguments[ARG_NEW] &&
		!(handle->result_flags & FRESULTF_MADE_LISTER))
	{
		// Open new lister
		if ((lister = lister_new(0)))
		{
			// Set flag for devicelist if appropriate
			if (command->function == FUNC_DEVICELIST)
			{
				// Set device list flag
				lister->lister->lister.flags |= DLSTF_DEVICE_LIST;

				// Which sort of device-list?
				if (instruction->funcargs)
				{
					// Full?
					if (instruction->funcargs->FA_Arguments[ARG_FULL])
						lister->lister->lister.flags |= DLSTF_DEV_FULL;

					// Brief?
					else if (instruction->funcargs->FA_Arguments[ARG_BRIEF])
						lister->lister->lister.flags |= DLSTF_DEV_BRIEF;
				}
			}

			// Or cache list
			else
				lister->lister->lister.flags |= DLSTF_CACHE_LIST;

			// Tell it to open
			IPC_Command(lister->ipc, LISTER_INIT, 0, GUI->screen_pointer, 0, 0);
		}

		// Return, lister will generate its own list
		return 1;
	}

	// Get current lister
	if (!(lister = function_lister_current(&handle->source_paths)))
		return 1;

	// Make window into a "special" buffer
	{
		LONG title_id = (command->function == FUNC_BUFFERLIST) ? MSG_BUFFER_LIST : MSG_DEVICE_LIST;

		IPC_Command(lister->ipc,
					LISTER_SHOW_SPECIAL_BUFFER,
					title_id,
					0,
					0,
					(struct MsgPort *)-1);
	}

	// Does lister have a handler?
	if (*lister->old_buffer->buf_CustomHandler)
		custom = lister->old_buffer;

	// Set "device list" flag
	lister->flags |= LISTERF_DEVICE_LIST;

	// Clear flag
	lister->cur_buffer->flags &= ~DWF_VALID;
	lister->cur_buffer->more_flags &= ~(DWF_DEVICE_LIST | DWF_DEV_FULL | DWF_DEV_BRIEF | DWF_CACHE_LIST);

	// Buffer list?
	if (command->function == FUNC_BUFFERLIST)
	{
		DirBuffer *buffer;
		Att_List *list;

		// Mark as a cache list
		lister->cur_buffer->more_flags |= DWF_CACHE_LIST;

		// Create temporary list
		if ((list = Att_NewList(0)))
		{
			Att_Node *node;
			DirEntry *entry = 0;

			// Lock buffer list
			lock_listlock(&GUI->buffer_list, FALSE);

			// Go through buffers
			for (buffer = (DirBuffer *)GUI->buffer_list.list.lh_Head; buffer->node.ln_Succ;
				 buffer = (DirBuffer *)buffer->node.ln_Succ)
			{
				// Valid directory?
				if (buffer->flags & DWF_VALID)
				{
					char *ptr;

					// Looking for custom buffer?
					if (custom)
					{
						// If this buffer doesn't match the handler, we ignore it
						if (stricmp(custom->buf_CustomHandler, buffer->buf_CustomHandler) != 0)
							continue;

						// Or, if the lister doesn't match the handler, we ignore it
						else if (custom->buf_OwnerLister != buffer->buf_OwnerLister)
							continue;
					}

					// Otherwise, if this is a custom buffer we ignore it
					else if (*buffer->buf_CustomHandler)
						continue;

					// Copy path, strip /
					strcpy(handle->work_buffer, buffer->buf_Path);
					if (*(ptr = handle->work_buffer + strlen(handle->work_buffer) - 1) == '/')
						*ptr = 0;

					// Add to list
					if (handle->work_buffer[0])
						Att_NewNode(list, handle->work_buffer, (IPTR)buffer, ADDNODE_SORT | ADDNODE_EXCLUSIVE);
				}
			}

			// Unlock buffer list
			unlock_listlock(&GUI->buffer_list);

			// Go through sorted list
			for (node = (Att_Node *)list->list.lh_Head; node->node.ln_Succ; node = (Att_Node *)node->node.ln_Succ)
			{
				// Add entry
				if ((entry = add_file_entry(lister->cur_buffer,
											create_file_entry(lister->cur_buffer,
															  0,
															  FilePart(node->node.ln_Name),
															  -1,
															  0,
															  0,
															  0,
															  0,
															  SUBENTRY_BUFFER,
															  node->node.ln_Name,
															  0,
															  0),
											entry)))
				{
					// set userdata to point to buffer
					entry->de_UserData = (IPTR)node->data;
				}
			}

			// Free temporary list
			Att_RemList(list, 0);
		}
	}

	// Device list
	else if ((doslist = LockDosList(LDF_DEVICES | LDF_VOLUMES | LDF_ASSIGNS | LDF_READ)))
	{
		short max_name_width = 0, max_dev_width = 0, max_full_width = 0, max_free_width = 0, max_used_width = 0,
			  need_vol = 0;
		char *dev_format, *asn_format, *dev_val_format;
		char dev_format_buf[256], dev_val_format_buf[128], asn_format_buf[128];
		short asn_first_x, asn_multi_x;
		BOOL full = 0, brief = 0;
		short num;

		// Mark as a device list
		lister->cur_buffer->more_flags |= DWF_DEVICE_LIST;

		// Which sort of device-list?
		if (instruction->funcargs)
		{
			// Full?
			if (instruction->funcargs->FA_Arguments[ARG_FULL])
			{
				full = 1;
				lister->cur_buffer->more_flags |= DWF_DEV_FULL;
			}

			// Brief?
			else if (instruction->funcargs->FA_Arguments[ARG_BRIEF])
			{
				brief = 1;
				lister->cur_buffer->more_flags |= DWF_DEV_BRIEF;
			}
		}

		// Brief device list?
		if (brief)
			num = 2;
		else
			num = 3;

		// Go through the types of entries
		for (type = 0; type < num; type++)
		{
			// Initialise list
			NewList(&list[type]);

			// Scan list
			list_entry = doslist;
			while ((list_entry = NextDosEntry(list_entry, type_order[type])))
			{
				// Valid device?
				if (list_entry->dol_Type == DLT_DIRECTORY || list_entry->dol_Task)
				{
					// Convert name
					BtoCStr(list_entry->dol_Name, device_name, 32);
					strcat(device_name, ":");

					// Get pathname
					if (full)
						DevNameFromLockDopus(list_entry->dol_Lock, handle->work_buffer, 256);
					else
						handle->work_buffer[0] = 0;

					// Create entry
					if ((node = AllocMemH(handle->entry_memory,
										  sizeof(struct Node) + 32 + strlen(handle->work_buffer) + 1)))
					{
						node->ln_Type = list_entry->dol_Type;
						strcpy((char *)(node + 1), device_name);
						strcpy(((char *)(node + 1)) + 32, handle->work_buffer);
						AddTail(&list[type], node);
					}

					// Multi-path assign?
					if (type_order[type] == LDF_ASSIGNS && list_entry->dol_misc.dol_assign.dol_List && full)
					{
						struct AssignList *assign;

						// Go through assign list
						for (assign = list_entry->dol_misc.dol_assign.dol_List; assign; assign = assign->al_Next)
						{
							// Get pathname
							DevNameFromLockDopus(assign->al_Lock, handle->work_buffer, 256);

							// Create entry
							if ((node = AllocMemH(handle->entry_memory,
												  sizeof(struct Node) + strlen(handle->work_buffer) + 1)))
							{
								node->ln_Type = 255;
								strcpy((char *)(node + 1), handle->work_buffer);
								AddTail(&list[type], node);
							}
						}
					}
				}
			}
		}

		// Unlock DOS list
		UnLockDosList(LDF_DEVICES | LDF_VOLUMES | LDF_ASSIGNS | LDF_READ);

		// Go through the lists to get maximum name width
		for (type = 0; type < num; type++)
		{
			// Go through the list we created
			for (node = list[type].lh_Head; node->ln_Succ; node = node->ln_Succ)
			{
				// Device or volume?
				if (type == 0 || type == 1)
				{
					D_S(struct InfoData, info)
					struct DevProc *proc;
					struct DosList *dos;
					struct device_data *data;
					LONG res = 0;

					// Get device process
					if (!(proc = GetDeviceProc((char *)(node + 1), 0)))
						return 0;

						// Get current information, check it's valid
#ifdef __AROS__
					if (((struct Library *)DOSBase)->lib_Version < 50)
					{
						// res=Info(proc->dvp_Lock,info);
						BPTR lock;
						if ((lock = Lock((char *)(node + 1), SHARED_LOCK)))
						{
							res = Info(lock, info);
							UnLock(lock);
						}
					}
					else
#endif
						res = DoPkt(proc->dvp_Port, ACTION_DISK_INFO, (SIPTR)MKBADDR(info), 0, 0, 0, 0);

					if (!res || !(dos = (struct DosList *)BADDR(info->id_VolumeNode)) ||
						info->id_DiskType == ID_NO_DISK_PRESENT || info->id_DiskType == ID_UNREADABLE_DISK ||
						info->id_DiskType == ID_NOT_REALLY_DOS || info->id_DiskType == ID_KICKSTART_DISK)
					{
						// Free device process and proceed to next entry
						FreeDeviceProc(proc);
						continue;
					}

					// If this is a volume, see if we already have it in the list
					if (type == 1)
					{
						struct Node *test;
						struct device_data *data;

						// Go through device list
						for (test = list[0].lh_Head; test->ln_Succ; test = test->ln_Succ)
						{
							// Get device data
							if ((data = (struct device_data *)test->ln_Name))
							{
								// Compare dos list pointer
								if (data->dos == dos)
									break;
							}
						}

						// Did it exist?
						if (test->ln_Succ)
						{
							// Free device process and proceed to next entry
							FreeDeviceProc(proc);
							continue;
						}
					}

					// Allocate some data
					if ((data = AllocMemH(handle->entry_memory, sizeof(struct device_data))))
					{
						short len;

						// Convert name, store doslist pointer
						BtoCStr(dos->dol_Name, data->name, 32);
						data->dos = dos;

						// Save date
						data->date = dos->dol_misc.dol_volume.dol_VolumeDate;

						// Is disk validating?
						if (info->id_DiskState == ID_VALIDATING)
						{
							data->valid = 1;
						}

						// Nope, it's ok
						else
						{
							ULONG disktype;

							// Get percent used. Widen through ULONG (not via direct (UQUAD)
							// sign-extension) and cap at 100% so misbehaving filesystems that
							// report id_NumBlocksUsed > id_NumBlocks don't produce "1208%".
							if (info->id_NumBlocks == 0)
							{
								strcpy(data->full, "100");
							}
							else
							{
#ifdef USE_64BIT
								UQUAD u_used = (UQUAD)(ULONG)info->id_NumBlocksUsed;
								UQUAD u_num = (UQUAD)(ULONG)info->id_NumBlocks;
								UQUAD percent;
								if (u_used > u_num)
									u_used = u_num;
								percent = (u_used * 100) / u_num;
								if (percent > 100)
									percent = 100;
								lsprintf(data->full, "%lu", (ULONG)percent);
#else
								ULONG u_used = (ULONG)info->id_NumBlocksUsed;
								ULONG u_num = (ULONG)info->id_NumBlocks;
								ULONG percent;
								if (u_used > u_num)
									u_used = u_num;
								percent = UMult32(u_used, 100) / u_num;
								if (percent > 100)
									percent = 100;
								lsprintf(data->full, "%lu", percent);
#endif
							}

							// Get space free and used. If the filesystem reports
							// id_NumBlocksUsed > id_NumBlocks (seen on amiberry
							// directory-mounts whose host-reported values are
							// inconsistent), subtraction wraps to a 64-bit garbage
							// value that would render as tens of terabytes. Show
							// "?" in that case rather than a nonsense size.
							if ((ULONG)info->id_NumBlocksUsed > (ULONG)info->id_NumBlocks)
							{
								strcpy(data->free, "?");
								strcpy(data->used, "?");
							}
							else
							{
#ifdef USE_64BIT
								UQUAD tmp;
								tmp = ((UQUAD)(ULONG)info->id_NumBlocks - (UQUAD)(ULONG)info->id_NumBlocksUsed) *
									  (UQUAD)(ULONG)info->id_BytesPerBlock;
								BytesToString64(
									&tmp,
									data->free,
									sizeof(data->free),
									1,
									(environment->env->settings.date_flags & DATE_1000SEP) ? GUI->decimal_sep : 0);
								tmp = (UQUAD)(ULONG)info->id_NumBlocksUsed * (UQUAD)(ULONG)info->id_BytesPerBlock;
								BytesToString64(
									&tmp,
									data->used,
									sizeof(data->used),
									1,
									(environment->env->settings.date_flags & DATE_1000SEP) ? GUI->decimal_sep : 0);
#else
								BytesToString(
									((ULONG)info->id_NumBlocks - (ULONG)info->id_NumBlocksUsed) * (ULONG)info->id_BytesPerBlock,
									data->free,
									1,
									(environment->env->settings.date_flags & DATE_1000SEP) ? GUI->decimal_sep : 0);
								BytesToString(
									(ULONG)info->id_NumBlocksUsed * (ULONG)info->id_BytesPerBlock,
									data->used,
									1,
									(environment->env->settings.date_flags & DATE_1000SEP) ? GUI->decimal_sep : 0);
#endif
							}

							// Check widths
							if ((len = lister_get_length(lister, data->full)) > max_full_width)
								max_full_width = len;
							if ((len = lister_get_length(lister, data->free)) > max_free_width)
								max_free_width = len;
							if ((len = lister_get_length(lister, data->used)) > max_used_width)
								max_used_width = len;

							// Get disk type from DOS list if it's set
							disktype = dos->dol_misc.dol_volume.dol_DiskType;

							// If it's not, get it from Info
							if (!disktype)
								disktype = info->id_DiskType;

							// Get filesystem string
							data->fsys[0] = (char)((disktype >> 24) & 0xff);
							data->fsys[1] = (char)((disktype >> 16) & 0xff);
							data->fsys[2] = (char)((disktype >> 8) & 0xff);

							// Last character might be either a number or a letter
							data->fsys[3] = (char)(disktype & 0xff);
							if (data->fsys[3] < 10)
								data->fsys[3] += '0';
							data->fsys[4] = 0;
						}

						// Store pointer to data
						node->ln_Name = (char *)data;

						// Check widths
						if ((len = lister_get_length(lister, data->name)) > max_name_width)
							max_name_width = len;
						if (type == 0 && (len = lister_get_length(lister, (char *)(node + 1))) > max_dev_width)
							max_dev_width = len;
						else if (type != 0)
							need_vol = 1;
					}

					// Unlock device process
					FreeDeviceProc(proc);
				}

				// Assign
				else
				{
					short len;

					// Check name width
					if (node->ln_Type != 255 && (len = lister_get_length(lister, (char *)(node + 1))) > max_name_width)
						max_name_width = len;
				}
			}
		}

		// Get format string pointers
		dev_format = dev_format_buf;
		dev_val_format = dev_val_format_buf;
		asn_format = asn_format_buf;

		// Need to use volume string in device list?
		if (need_vol)
		{
			// Check maximum device width against volume string
			if (max_dev_width < (type = lister_get_length(lister, GetString(&locale, MSG_VOLUME))))
				max_dev_width = type;
			else
				need_vol = 0;
		}

		// Build format strings
		{
			short name_w, dev_w, full_w, free_w, used_w, pad;

			// Calculate column positions
			name_w = max_name_width + (pad = lister_get_length(lister, "   "));
			dev_w = name_w + max_dev_width + ((need_vol) ? pad : (lister_get_length(lister, "()") + pad));
			full_w = dev_w + max_full_width + lister_get_length(lister, "% , ") +
					 lister_get_length(lister, GetString(&locale, MSG_FULL));
			free_w = full_w + max_free_width + lister_get_length(lister, " , ") +
					 lister_get_length(lister, GetString(&locale, MSG_FREE));
			used_w = free_w + max_used_width + pad + lister_get_length(lister, GetString(&locale, MSG_USED));

			// Build format for main device list. Do this without RawDoFmt
			// because AROS x64 can consume literal "%%s" fragments as
			// format arguments when building a second-stage format string.
			strcpy(dev_format, "%s");
			devlist_append_column(dev_format, name_w);
			strcat(dev_format, "%s");
			devlist_append_column(dev_format, dev_w);
			strcat(dev_format, "%s%% ");
			strcat(dev_format, GetString(&locale, MSG_FULL));
			strcat(dev_format, ",");
			devlist_append_column(dev_format, full_w);
			strcat(dev_format, "%s ");
			strcat(dev_format, GetString(&locale, MSG_FREE));
			strcat(dev_format, ",");
			devlist_append_column(dev_format, free_w);
			strcat(dev_format, "%s ");
			strcat(dev_format, GetString(&locale, MSG_USED));
			devlist_append_column(dev_format, used_w);
			strcat(dev_format, "[%s]");

			// Used if a device is being validated
			strcpy(dev_val_format, "%s");
			devlist_append_column(dev_val_format, name_w);
			strcat(dev_val_format, "%s");
			devlist_append_column(dev_val_format, dev_w);
			strcat(dev_val_format, GetString(&locale, MSG_VALIDATING));

			// Used for assign list
			strcpy(asn_format, "%s");
			devlist_append_column(asn_format, name_w);
			strcat(asn_format, GetString(&locale, MSG_ASSIGN));

			// Get assign coordinates
			asn_first_x = dev_w;
			asn_multi_x = dev_w - lister_get_length(lister, "+ ");
		}

		// Go through the lists again to build device list
		for (type = 0; type < num; type++)
		{
			DirEntry *last_entry = 0;

			// Go through the list we created
			for (node = list[type].lh_Head; node->ln_Succ; node = node->ln_Succ)
			{
				char *comment = 0;
				struct DateStamp *date = 0;

				// Device?
				if (type == 0 || type == 1)
				{
					struct device_data *data;

					// Valid data entry?
					if ((data = (struct device_data *)node->ln_Name))
					{
						char buf[120];

						// Build device name
						if (type == 0)
						{
							buf[0] = '(';
							stccpy(buf + 1, (char *)(node + 1), sizeof(buf) - 1);
							if (buf[1])
								buf[strlen(buf) - 1] = ')';
						}
						else
							strcpy(buf, GetString(&locale, MSG_VOLUME));

						// Valid?
						if (!data->valid)
						{
							// Build display string
							lsprintf(handle->work_buffer,
									 dev_format,
									 (IPTR)data->name,
									 (IPTR)buf,
									 (IPTR)data->full,
									 (IPTR)data->free,
									 (IPTR)data->used,
									 (IPTR)data->fsys);
						}

						// Validating
						else
						{
							// Build display string
							lsprintf(handle->work_buffer, dev_val_format, (IPTR)data->name, (IPTR)buf);
						}

						// Use device name as comment
						comment = data->name;

						// Get date pointer
						date = &data->date;
					}

					// No valid entry, continue to next
					else
						continue;
				}

				// Assign
				else
				{
					// Multi-directory assign?
					if (node->ln_Type == 255)
					{
						handle->work_buffer[0] = 0;
						devlist_append_column(handle->work_buffer, asn_multi_x);
						strcat(handle->work_buffer, "+ ");
						strcat(handle->work_buffer, (char *)(node + 1));
						if ((comment = AllocMemH(handle->entry_memory, strlen((char *)(node + 1)) + 1)))
							strcpy(comment, (char *)(node + 1));
					}

					// Build display string
					else
					{
						lsprintf(handle->work_buffer, asn_format, (IPTR)(char *)(node + 1));
						if (full)
						{
							devlist_append_column(handle->work_buffer, asn_first_x);
							strcat(handle->work_buffer, ((char *)(node + 1)) + 32);
						}
					}
				}

				// Create an entry for this node
				last_entry = add_file_entry(
					lister->cur_buffer,
					create_file_entry(
						lister->cur_buffer,
						0,
						(char *)(node + 1),
						(node->ln_Type == DLT_DIRECTORY || node->ln_Type == 255) ? DLT_DIRECTORY : DLT_DEVICE,
						ENTRY_DEVICE,
						date,
						comment,
						0,
						(node->ln_Type == 255) ? SUBENTRY_BUFFER : 0,
						handle->work_buffer,
						0,
						0),
					(node->ln_Type == 255 && last_entry) ? last_entry : add_after);
			}

			// Find entry at the end of the list
			add_after = (DirEntry *)lister->cur_buffer->entry_list.mlh_TailPred;
			if (!add_after->de_Node.dn_Pred)
				add_after = 0;
		}
	}

	// Get icons
	if (lister->flags & LISTERF_VIEW_ICONS || lister->flags & LISTERF_ICON_ACTION)
		lister_get_icons(handle, lister, 0, GETICONSF_CLEAN | GETICONSF_NO_REFRESH);

	return 1;
}
