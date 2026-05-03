#include "config_lib.h"
#include "config_menus.h"
#include "config_buttons.h"

// Save menu
BOOL config_menus_save(config_menus_data *data, BOOL save_as)
{
	short loop = 1;
	BOOL ret = 0;
	char path[256];

	// Valid bank?
	if (!data->bank)
		return 0;

	// Make window busy
	SetWindowBusy(data->window);

	// Get initial path
	strcpy(path, data->bank->path);

	// Loop until successful
	while (loop)
	{
		// Need to ask for filename?
		if (save_as || !*path)
		{
			char *ptr = 0;
			char file[40];

			// No path given?
			if (!*path)
				strcpy(path, "dopus5:buttons/");

			// Get filename
			else if ((ptr = FilePart(path)))
			{
				// Store filename, clear from path
				strcpy(file, ptr);
				*ptr = 0;
				ptr = file;
			}

			// Display file requester
			if ((loop = AslRequestTags(DATA(data->window)->request,
									   ASLFR_Window,
									   data->window,
									   ASLFR_TitleText,
									   GetString(locale, MSG_LISTER_MENU_ENTER_NAME),
									   (ptr) ? ASLFR_InitialFile : TAG_IGNORE,
									   ptr,
									   ASLFR_InitialDrawer,
									   path,
									   ASLFR_Flags1,
									   FRF_PRIVATEIDCMP | FRF_DOSAVEMODE,
									   TAG_END)))
			{
				BPTR lock;

				// Get new path
				strcpy(path, DATA(data->window)->request->fr_Drawer);
				AddPart(path, DATA(data->window)->request->fr_File, 256);

				// See if file exists
				if ((lock = Lock(path, ACCESS_READ)))
				{
					// It does
					UnLock(lock);

					// Show error
					loop = SimpleRequestTags(data->window,
											 0,
											 GetString(locale, MSG_REPLACE_CANCEL),
											 GetString(locale, MSG_FILE_EXISTS),
											 DATA(data->window)->request->fr_File);
				}
			}
		}

		// Ok to save?
		if (loop)
		{
			// Fix menu flag
			data->bank->window.flags |= BTNWF_FIX_MENU;

			// Save buttons
			if (!(loop = SaveButtonBank(data->bank, path)))
			{
				// Clear change flag
				data->change = 0;

				// Set return change flag
				data->ret_change = 1;

				// Get path
				strcpy(data->last_saved, data->bank->path);
				ret = 1;
			}

			// Error
			else
			{
				// Show error
				loop = error_saving(loop, data->window);

				// Clear 'save as' flag
				save_as = 0;
			}
		}
	}

	// Make window unbusy
	ClearWindowBusy(data->window);

	return ret;
}

// Load a new menu
void config_menus_load(config_menus_data *data, short type)
{
	char path[256];
	BOOL ok = 1;

	// Store current path for 'open'
	if (type == MENU_LISTER_MENU_OPEN && data->bank)
		strcpy(path, data->bank->path);

	// Get last path for 'last saved'
	else if (type == MENU_LISTER_LAST_SAVED)
		strcpy(path, data->last_saved);

	// Otherwise clear path buffer
	else
		*path = 0;

	// Check if current menu has changed
	if (!(config_menus_check_change(data, (type != MENU_LISTER_LAST_SAVED))))
		return;

	// Make window busy
	SetWindowBusy(data->window);

	// Defaults?
	if (type == MENU_LISTER_RESET_DEFAULTS)
	{
		// Default name
		strcpy(path, config_menus_default[data->type]);
	}

	// Load new menu?
	else if (type == MENU_LISTER_MENU_OPEN)
	{
		char file[32], *ptr;

		// Get filename
		if ((ptr = PathPart(path)) && *ptr)
		{
			strcpy(file, ptr + 1);
			if (*ptr == '/' || *ptr == ':')
				*ptr = 0;
		}
		else
			file[0] = 0;

		// If no path specified, use default
		if (!*path)
			strcpy(path, "dopus5:buttons/");

		// Display file requester
		if (AslRequestTags(DATA(data->window)->request,
						   ASLFR_Window,
						   data->window,
						   ASLFR_TitleText,
						   GetString(locale, MSG_LISTER_MENU_SELECT_FILE),
						   ASLFR_InitialFile,
						   file,
						   ASLFR_InitialDrawer,
						   path,
						   ASLFR_Flags1,
						   FRF_PRIVATEIDCMP,
						   TAG_END))
		{
			// Get new path
			strcpy(path, DATA(data->window)->request->fr_Drawer);
			AddPart(path, DATA(data->window)->request->fr_File, 256);
		}
		else
			*path = 0;

		// Cancelled?
		if (!*path)
			ok = 0;
	}

	// Ok to load?
	if (ok)
	{
		// Detach menu list
		SetGadgetChoices(data->objlist, GAD_MENUS_MENU, (APTR)~0);

		// Load new bank
		config_menus_load_bank(data, 0, path);

		// Initialise display
		config_menus_init(data);
	}

	// Make window unbusy
	ClearWindowBusy(data->window);
}

// Open bank
Cfg_ButtonBank *config_menus_load_bank(config_menus_data *data, Cfg_ButtonBank *bank, char *path)
{
	short a;
	Cfg_Button *button;
	BOOL need_title = 1;

	// Free existing bank
	CloseButtonBank(data->bank);
	data->bank = 0;

	// Clear change flag
	data->change = 0;

	// Free lists, recreate
	for (a = 0; a < 3; a++)
	{
		// Free list
		Att_RemList(data->menu_list[a], REMLIST_FREEDATA);

		// Create new one
		if (!(data->menu_list[a] = Att_NewList(LISTF_POOL)))
			return 0;
	}

	// Bank supplied?
	if (bank)
		data->bank = CopyButtonBank(bank);

	// No valid bank?
	if (!data->bank && path)
	{
		// Try to load bank
		if ((data->bank = OpenButtonBank(path)))
		{
			// Lister menu?
			if (data->type == MENUTYPE_LISTER)
			{
				// Convert bank if it needs it
				if (!(data->bank->window.flags & BTNWF_FIX_MENU))
				{
					ConvertStartMenu(data->bank);
					data->bank->window.flags |= BTNWF_FIX_MENU;
				}
			}
		}
	}

	// No valid bank?
	if (!data->bank)
	{
		// Create new bank
		if ((data->bank = NewButtonBank(0, 0)))
		{
			// Initialise bank
			strcpy(data->bank->window.name, GetString(locale, config_menus_title[(data->type << 1) + 1]));
			data->bank->window.pos.Width = 128;
			data->bank->window.pos.Height = -1;
			data->bank->window.columns = 1;
			data->bank->window.rows = 0;
		}

		// Failed
		else
			return 0;
	}

	// Go through buttons
	for (button = (Cfg_Button *)data->bank->buttons.lh_Head; button->node.ln_Succ;
		 button = (Cfg_Button *)button->node.ln_Succ)
	{
		Cfg_ButtonFunction *func;

		// Find first function
		func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;
		if (!func->node.ln_Succ)
			func = 0;

		// Menu title?
		if (button->button.flags & BUTNF_TITLE)
		{
			// Add node
			if (config_menus_new_node(data->menu_list[MENU_MENU], button, func))
				need_title = 0;
		}
	}

	// Need default title?
	if (need_title && !(IsListEmpty(&data->bank->buttons)))
	{
		// Create a button
		if ((button = NewButtonWithFunc(
				 data->bank->memory, GetString(locale, config_menus_title[(data->type << 1) + 1]), FTYPE_LEFT_BUTTON)))
		{
			// Set title flag
			button->button.flags |= BUTNF_TITLE;

			// Add to head of list
			AddHead(&data->bank->buttons, &button->node);

			// Increment button row count
			data->bank->window.rows++;

			// Add to menu list
			config_menus_new_node(
				data->menu_list[MENU_MENU],
				button,
				(Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON));
		}
	}

	return data->bank;
}

// Restore menu
void config_menus_restore(config_menus_data *data)
{
	// Got backed up bank?
	if (!data->restore_bank)
		return;

	// Check change
	if (!(config_menus_check_change(data, TRUE)))
		return;

	// Make window busy
	SetWindowBusy(data->window);

	// Detach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU, (APTR)~0);

	// Use backed-up bank
	config_menus_load_bank(data, data->restore_bank, 0);

	// Initialise display
	config_menus_init(data);

	// Make window unbusy
	ClearWindowBusy(data->window);
}
