#include "config_lib.h"
#include "config_environment.h"
#include <proto/module.h>

void config_env_show_sound(config_env_data *data)
{
	short num;
	Cfg_SoundEntry *sound;

	// Get selection
	if ((num = GetGadgetValue(data->option_list, GAD_SETTINGS_SOUNDLIST)) > -1 &&
		(sound = (Cfg_SoundEntry *)Att_FindNode((Att_List *)&data->sound_list, num)) && sound->dse_Sound[0])
	{
		SetGadgetValue(data->option_list, GAD_SETTINGS_SOUNDLIST_PATH, (IPTR)sound->dse_Sound);
		SetGadgetValue(data->option_list, GAD_SETTINGS_VOLUME, sound->dse_Volume);
		SetGadgetValue(data->option_list, GAD_SETTINGS_VOLUME_SLIDER, sound->dse_Volume);
	}
	else
	{
		SetGadgetValue(data->option_list, GAD_SETTINGS_SOUNDLIST_PATH, 0);
		SetGadgetValue(data->option_list, GAD_SETTINGS_VOLUME, 64);
		SetGadgetValue(data->option_list, GAD_SETTINGS_VOLUME_SLIDER, 64);
	}
}

void config_env_store_sound(config_env_data *data)
{
	short num;
	Cfg_SoundEntry *sound;

	// Get selection
	if ((num = GetGadgetValue(data->option_list, GAD_SETTINGS_SOUNDLIST)) > -1 &&
		(sound = (Cfg_SoundEntry *)Att_FindNode((Att_List *)&data->sound_list, num)))
	{
		short flags, old_volume;
		char old_sound[256];

		strcpy(old_sound, sound->dse_Sound);
		old_volume = sound->dse_Volume;

		strcpy(sound->dse_Sound, (char *)GetGadgetValue(data->option_list, GAD_SETTINGS_SOUNDLIST_PATH));
		UpdateGadgetValue(data->option_list, 0, GAD_SETTINGS_VOLUME);
		if ((sound->dse_Volume = GetGadgetValue(data->option_list, GAD_SETTINGS_VOLUME)) < 0)
			sound->dse_Volume = 0;
		else if (sound->dse_Volume > 64)
			sound->dse_Volume = 64;
		SetGadgetValue(data->option_list, GAD_SETTINGS_VOLUME, sound->dse_Volume);
		SetGadgetValue(data->option_list, GAD_SETTINGS_VOLUME_SLIDER, sound->dse_Volume);

		// Did sound change?
		if (strcmp(old_sound, sound->dse_Sound) != 0)
		{
			// Fix pen usage
			flags = sound->dse_Node.lve_Flags;
			if (sound->dse_Sound[0])
				sound->dse_Node.lve_Flags |= LVEF_USE_PEN;
			else
				sound->dse_Node.lve_Flags &= ~LVEF_USE_PEN;
			if (flags != sound->dse_Node.lve_Flags)
			{
				SetGadgetChoices(data->objlist, GAD_SETTINGS_SOUNDLIST, (APTR)~0);
				SetGadgetChoices(data->objlist, GAD_SETTINGS_SOUNDLIST, &data->sound_list);
			}
			sound->dse_Node.lve_Flags |= LVEF_TEMP;
		}

		// Or volume?
		else if (old_volume != sound->dse_Volume)
			sound->dse_Node.lve_Flags |= LVEF_TEMP;
	}
}

void config_env_test_sound(config_env_data *data)
{
	struct Library *ModuleBase;
#ifdef __amigaos4__
	struct ModuleIFace *IModule;
#endif
	BOOL ok = 0;

	// Busy the window
	SetWindowBusy(data->window);

	// Open play.module
	if ((ModuleBase = OpenLibrary("DOpus5:Modules/play.module", LIB_VERSION)) && GETINTERFACE(IModule, ModuleBase))
	{
		short num;
		Cfg_SoundEntry *sound;

		// Get selection
		if ((num = GetGadgetValue(data->option_list, GAD_SETTINGS_SOUNDLIST)) > -1 &&
			(sound = (Cfg_SoundEntry *)Att_FindNode((Att_List *)&data->sound_list, num)) && sound->dse_Sound[0])
		{
			struct Node node;
			struct List list;

			// Build file list
			NewList(&list);
			node.ln_Name = sound->dse_Sound;
			AddTail(&list, &node);

			// Play the sound
			Module_Entry(
				&list, data->window->WScreen, data->ipc, data->main_ipc, (IPTR)data->window, sound->dse_Volume << 8);
			ok = 1;
		}

// Close module
#ifdef __amigaos4__
		DropInterface((struct Interface *)IModule);
#endif
		CloseLibrary(ModuleBase);
	}
	else
		CloseLibrary(ModuleBase);

	// Unbusy the window
	ClearWindowBusy(data->window);
	if (!ok)
		DisplayBeep(data->window->WScreen);
}
