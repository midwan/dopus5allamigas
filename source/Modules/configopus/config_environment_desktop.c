#include "config_lib.h"
#include "config_environment.h"
#include <proto/module.h>

// Fix font pens
void _config_env_fix_font_pens(config_env_data *data)
{
	short pen;

	// Initially we have four pens
	data->font_pen_count = 4;

	// Fill in base four pens
	for (pen = 0; pen < 4; pen++)
		data->font_pen_table[pen] = pen;

	// Under 39 OS has 8 pens
	if (((struct Library *)GfxBase)->lib_Version >= 39)
	{
		// Increase count
		data->font_pen_count += 4;

		// Add top four bens
		for (; pen < 8; pen++)
			data->font_pen_table[pen] = 248 + pen;
	}

	// User pens?
	if (data->palette_table)
	{
		short a;

		// Increase count
		data->font_pen_count += data->config->palette_count;

		// Add pens
		for (a = 0; a < data->config->palette_count; a++, pen++)
			data->font_pen_table[pen] = data->palette_table[a];
	}
}

unsigned char _env_map_font_colour(config_env_data *data, unsigned char col)
{
	unsigned char pen;

	// OS colours return as is
	if (col < 4 || col >= 252 || !data->palette_table)
		return col;

	// Go through user pen array
	for (pen = 0; pen < data->config->palette_count; pen++)
		if (data->palette_table[pen] == col)
			return (unsigned char)(pen + 4);

	return col;
}

// Build drives list
void _config_env_build_drive_list(config_env_data *data, short which)
{
	Att_List *list, *list2;
	Att_Node *node, *next;
	struct DosList *dos;

	// Create lists
	if (!(list = Att_NewList(0)) || !(list2 = Att_NewList(0)))
	{
		Att_RemList(list, 0);
		return;
	}

	// Lock DOS list
	dos = LockDosList(LDF_DEVICES | LDF_READ);

	// Go through DOS list
	while ((dos = NextDosEntry(dos, LDF_DEVICES)))
	{
		char devicename[34];

		// Valid device?
		if (!IsDiskDevice(dos->dol_Task))
			continue;

		// Build device name
		BtoCStr(dos->dol_Name, devicename, 32);
		strcat(devicename, ":");

#ifdef __AROS__
		// Many non-disk devices have garbage in their dol_Name, so the above
		// is going to segfault on linux-hosted. On the other hand, if I just
		// get a volume list, I won't be able to hide the disks.
		if (((struct Library *)DOSBase)->lib_Version < 50)
		{
			D_S(struct InfoData, info)
			BPTR lock;

			if (!(lock = Lock(devicename, SHARED_LOCK)))
				continue;

			if (!Info(lock, info) || (info->id_DiskType == 0))
			{
				UnLock(lock);
				continue;
			}

			UnLock(lock);
		}
#endif

		// Add to list
		Att_NewNode(list, devicename, 0, ADDNODE_SORT);
	}

	// Unlock DOS list
	UnLockDosList(LDF_DEVICES | LDF_READ);

	// Go through list
	for (node = (Att_Node *)list->list.lh_Head; node->node.ln_Succ; node = next)
	{
		char entry[160], volumename[40];
		BPTR lock;

		// Cache next
		next = (Att_Node *)node->node.ln_Succ;

		// Clear volume name
		volumename[0] = 0;

		// Lock device
		if ((lock = Lock(node->node.ln_Name, ACCESS_READ)))
		{
			struct FileLock *fl;
			struct DosList *volume;

			// Get filelock pointer
			fl = (struct FileLock *)BADDR(lock);

			// Get volume entry
			if ((volume = (struct DosList *)BADDR(fl->fl_Volume)))
			{
				// Get name
				lsprintf(volumename, "%b", volume->dol_Name);
			}

			// Unlock it
			UnLock(lock);
		}

		// No volume?
		if (!*volumename)
		{
			// Remove from this list, add to other
			Remove((struct Node *)node);
			AddTail((struct List *)list2, (struct Node *)node);
		}

		// Otherwise
		else
		{
			// Build full entry name
			lsprintf(entry, "%s\a\xa%s", node->node.ln_Name, volumename);

			// Change name
			Att_ChangeNodeName(node, entry);
		}
	}

	// Go through secondary list
	for (node = (Att_Node *)list2->list.lh_Head; node->node.ln_Succ; node = next)
	{
		// Get next
		next = (Att_Node *)node->node.ln_Succ;

		// Remove from this list
		Remove((struct Node *)node);

		// Add to main list
		AddTail((struct List *)list, (struct Node *)node);
	}

	// Free secondary list
	Att_RemList(list2, 0);

	// Store list pointer
	data->desktop_drives[which] = list;
}

// Fix drives list
void _config_env_fix_drive_list(config_env_data *data, short hide_type)
{
	Att_Node *node;
	Cfg_Desktop *desk;
	short which;

	// Get list
	which = (hide_type == DESKTOP_HIDE) ? 0 : 1;

	// No list?
	if (!data->desktop_drives[which])
		return;

	// Go through list
	for (node = (Att_Node *)data->desktop_drives[which]->list.lh_Head; node->node.ln_Succ;
		 node = (Att_Node *)node->node.ln_Succ)
	{
		// Turn off
		node->node.lve_Flags &= ~LVEF_SELECTED;
		node->node.lve_Flags |= LVEF_TEMP;
	}

	// Go through desktop list
	for (desk = (Cfg_Desktop *)data->env->desktop.mlh_Head; desk->node.mln_Succ;
		 desk = (Cfg_Desktop *)desk->node.mln_Succ)
	{
		// Hidden drive?
		if (desk->data.dt_Type == hide_type)
		{
			// Find name in list
			if ((node =
					 (Att_Node *)FindNameI((struct List *)data->desktop_drives[which], (char *)(&desk->data.dt_Data))))
			{
				// Turn on
				node->node.lve_Flags |= LVEF_SELECTED;
			}
		}
	}
}

// Update hidden drives list
BOOL _config_env_update_drives_list(config_env_data *data, short hide_type)
{
	Att_Node *node;
	Cfg_Desktop *desk, *next;
	BOOL change = 0;
	short which;

	// Get list
	which = (hide_type == DESKTOP_HIDE) ? 0 : 1;

	// No list?
	if (!data->desktop_drives[which])
		return 0;

	// Lock desktop list
	GetSemaphore(&data->env->desktop_lock, SEMF_EXCLUSIVE, 0);

	// Go through desktop list
	for (desk = (Cfg_Desktop *)data->env->desktop.mlh_Head; desk->node.mln_Succ; desk = next)
	{
		// Get next
		next = (Cfg_Desktop *)desk->node.mln_Succ;

		// Hidden drive?
		if (desk->data.dt_Type == hide_type)
		{
			BOOL keep = 1;

			// Find name in list
			if ((node =
					 (Att_Node *)FindNameI((struct List *)data->desktop_drives[which], (char *)(&desk->data.dt_Data))))
			{
				// Deselected in list?
				if (!(node->node.lve_Flags & LVEF_SELECTED))
					keep = 0;

				// Remove so we won't pick it up next time
				node->node.lve_Flags &= ~LVEF_TEMP;
			}

			// Don't want to keep this?
			if (!keep)
			{
				// Remove from list and free
				Remove((struct Node *)desk);
				FreeMemH(desk);
				change = 1;
			}
		}
	}

	// Go through drives list
	for (node = (Att_Node *)data->desktop_drives[which]->list.lh_Head; node->node.ln_Succ;
		 node = (Att_Node *)node->node.ln_Succ)
	{
		// Selected and not used yet?
		if (node->node.lve_Flags & LVEF_SELECTED && node->node.lve_Flags & LVEF_TEMP)
		{
			char devicename[34], *ptr;
			short a = 0;

			// Copy device name
			ptr = node->node.ln_Name;
			while (*ptr && a < 33)
			{
				devicename[a++] = *ptr;
				if (*ptr == ':')
					break;
				++ptr;
			}
			devicename[a] = 0;

			// Allocate desktop node
			if ((desk = AllocMemH(data->env->desktop_memory, sizeof(Cfg_Desktop) + a)))
			{
				// Fill out desktop node
				desk->data.dt_Type = hide_type;
				desk->data.dt_Size = sizeof(CFG_DESK) + a;
				strcpy((char *)&desk->data.dt_Data, devicename);

				// Add to list
				AddTail((struct List *)&data->env->desktop, (struct Node *)desk);
				change = 1;
			}
		}
	}

	// Unlock desktop list
	FreeSemaphore(&data->env->desktop_lock);

	return change;
}
