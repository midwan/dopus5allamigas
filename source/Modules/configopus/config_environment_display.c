#include "config_lib.h"
#include "config_environment.h"
#include <proto/module.h>

// Display mode support
void _config_env_screenmode_init(config_env_data *data, BOOL refresh)
{
	struct NameInfo nameinfo;
	int sel;

	// Get current mode
	switch (data->config->screen_mode)
	{
	case MODE_WORKBENCHUSE:
		stccpy(data->mode_name, GetString(locale, MSG_ENVIRONMENT_SCREENMODE_WB_USE), sizeof(data->mode_name));
		break;

	case MODE_WORKBENCHCLONE:
		stccpy(data->mode_name, GetString(locale, MSG_ENVIRONMENT_SCREENMODE_WB_CLONE), sizeof(data->mode_name));
		break;

	case MODE_PUBLICSCREEN:
		lsprintf(data->mode_name, "%s:%s", data->config->pubscreen_name, GetString(locale, MSG_USE));
		break;

	default:
		// Get mode name (if available)
		if (GetDisplayInfoData(0, (char *)&nameinfo, sizeof(nameinfo), DTAG_NAME, data->config->screen_mode))
		{
			stccpy(data->mode_name, nameinfo.Name, sizeof(data->mode_name));
		}
		else
		{
			data->mode_name[0] = 0;
		}
		break;
	}

	// Set current mode selection
	if (refresh)
	{
		sel = Att_NodeNumber(data->mode_list, data->mode_name);
		SetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_MODE, sel);
	}

	// Initialise mode data
	_config_env_screenmode_init_mode(data, refresh);
}

void _config_env_screenmode_init_mode(config_env_data *data, BOOL save_depth)
{
	struct DimensionInfo diminfo;
	char buf[128];
	struct Screen *screen;
	int a;
	ULONG mode_id = 0;
	BOOL got_dims = 0;

	// Get current screenmode information
	switch (data->config->screen_mode)
	{
	// Existing screen
	case MODE_WORKBENCHUSE:
	case MODE_WORKBENCHCLONE:
	case MODE_PUBLICSCREEN: {
		// Strip Use/Clone from mode name
		stccpy(buf, data->mode_name, sizeof(buf));
		for (a = strlen(buf) - 1; a >= 0; a--)
		{
			if (buf[a] == ':')
			{
				buf[a] = 0;
				break;
			}
		}

		// Get screen
		if ((screen = LockPubScreen(buf)))
		{
			// Get screen mode
			mode_id = GetVPModeID(&screen->ViewPort);

			// Get minimum size
			if (data->config->screen_mode == MODE_WORKBENCHCLONE)
			{
				data->mode_size_limit.MinX = screen->Width;
				data->mode_size_limit.MinY = screen->Height;

				// Clone depth
				if (!save_depth)
					data->config->screen_depth = screen->RastPort.BitMap->Depth;
			}
			else
			{
				data->mode_size_limit.MinX = 640;
				data->mode_size_limit.MinY = 200;
			}

			// Get maximum size
			data->mode_size_limit.MaxX = screen->Width;
			data->mode_size_limit.MaxY = screen->Height;

			// Get default size
			data->mode_size_default.MaxX = screen->Width;
			data->mode_size_default.MaxY = screen->Height;

			// Got dimensions
			got_dims = 1;

			// Release screen
			UnlockPubScreen(0, screen);
		}
	}
	break;

	// Real screen mode
	default:
		mode_id = data->config->screen_mode;
		break;
	}

	// Get mode info (if available)
	if (!(ModeNotAvailable(mode_id)) && (GetDisplayInfoData(0, (char *)&diminfo, sizeof(diminfo), DTAG_DIMS, mode_id)))
	{
		// Not already got dimensions?
		if (!got_dims)
		{
			// Get minimum size
			data->mode_size_limit.MinX = diminfo.MinRasterWidth;
			data->mode_size_limit.MinY = diminfo.MinRasterHeight;

			// Get maximum size
			data->mode_size_limit.MaxX = diminfo.MaxRasterWidth;
			data->mode_size_limit.MaxY = diminfo.MaxRasterHeight;

			// Get default size
			data->mode_size_default.MaxX = diminfo.TxtOScan.MaxX + 1;
			data->mode_size_default.MaxY = diminfo.TxtOScan.MaxY + 1;
		}

		// Get maximum colours
		data->mode_max_colours = diminfo.MaxDepth;
	}

	// Check settings
	if (data->mode_size_limit.MinX < 640)
		data->mode_size_limit.MinX = 640;
	if (data->mode_size_limit.MinY < 200)
		data->mode_size_limit.MinY = 200;
	if (data->mode_size_limit.MaxX < data->mode_size_limit.MinX)
		data->mode_size_limit.MaxX = data->mode_size_limit.MinX;
	if (data->mode_size_limit.MaxY < data->mode_size_limit.MinY)
		data->mode_size_limit.MaxY = data->mode_size_limit.MinY;
	if (data->mode_size_default.MaxX < data->mode_size_limit.MinX)
		data->mode_size_default.MaxX = data->mode_size_limit.MinX;
	else if (data->mode_size_default.MaxX > data->mode_size_limit.MaxX)
		data->mode_size_default.MaxX = data->mode_size_limit.MaxX;
	if (data->mode_size_default.MaxY < data->mode_size_limit.MinY)
		data->mode_size_default.MaxY = data->mode_size_limit.MinY;
	else if (data->mode_size_default.MaxY > data->mode_size_limit.MaxY)
		data->mode_size_default.MaxY = data->mode_size_limit.MaxY;
	if (data->mode_max_colours < 1)
		data->mode_max_colours = 1;

	// Fix gadgets
	_config_env_screenmode_fix_gadgets(data);
}

void _config_env_screenmode_fix_gadgets(config_env_data *data)
{
	BOOL state = FALSE;

	// If screen mode is WORKBENCH_CLONE, select both default checks and disable them
	if (data->config->screen_mode == MODE_WORKBENCHCLONE)
	{
		// Set default size flags
		SetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_DEFWIDTH, 1);
		SetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_DEFHEIGHT, 1);
		state = TRUE;
	}

	// Or if mode is a USE mode, disable them
	else if (data->config->screen_mode == MODE_WORKBENCHUSE || data->config->screen_mode == MODE_PUBLICSCREEN)
		state = TRUE;

	// Set enable/disable state of default checks
	DisableObject(data->option_list, GAD_ENVIRONMENT_SCREENMODE_DEFWIDTH, state);
	DisableObject(data->option_list, GAD_ENVIRONMENT_SCREENMODE_DEFHEIGHT, state);

	// If default width is checked, fill out width field with default width
	// and disable it
	if (GetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_DEFWIDTH))
	{
		SetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_WIDTH, data->mode_size_default.MaxX);
		state = TRUE;
	}

	// Or if mode is a USE mode, disable it
	else if (data->config->screen_mode == MODE_WORKBENCHUSE || data->config->screen_mode == MODE_PUBLICSCREEN)
		state = TRUE;

	// Otherwise, make sure field is enabled
	else
		state = FALSE;

	// Set enable state
	DisableObject(data->option_list, GAD_ENVIRONMENT_SCREENMODE_WIDTH, state);

	// If default height is checked, fill out height field with default height
	// and disable it
	if (GetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_DEFHEIGHT))
	{
		SetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_HEIGHT, data->mode_size_default.MaxY);
		state = TRUE;
	}

	// Or if mode is a USE mode, disable it
	else if (data->config->screen_mode == MODE_WORKBENCHUSE || data->config->screen_mode == MODE_PUBLICSCREEN)
		state = TRUE;

	// Otherwise, make sure field is enabled
	else
		state = FALSE;

	// Set enable state
	DisableObject(data->option_list, GAD_ENVIRONMENT_SCREENMODE_HEIGHT, state);

	// Fix depth slider limits
	SetGadgetChoices(
		data->option_list, GAD_ENVIRONMENT_SCREENMODE_COLORS, (APTR)(((ULONG)data->mode_max_colours << 16) | 2));
	SetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_COLORS, data->config->screen_depth);

	// Is mode is a USE mode, disable colours selector and font gadgets
	state = (data->config->screen_mode == MODE_WORKBENCHUSE || data->config->screen_mode == MODE_PUBLICSCREEN);
	DisableObject(data->option_list, GAD_ENVIRONMENT_SCREENMODE_COLORS, state);
	DisableObject(data->option_list, GAD_ENVIRONMENT_SCREENMODE_FONT, state);
	DisableObject(data->option_list, GAD_ENVIRONMENT_SCREENMODE_FONTNAME, state);
	DisableObject(data->option_list, GAD_ENVIRONMENT_SCREENMODE_FONTSIZE, state);

	// Check maximum/minimum values
	_config_env_screenmode_check_values(data);
}

void _config_env_screenmode_set_mode(config_env_data *data, Att_Node *node)
{
	// Get mode ID
	data->config->screen_mode = node->data;

	// If this is a public screen use, get public screen name
	if (data->config->screen_mode == MODE_PUBLICSCREEN)
	{
		int a;

		// Copy to pubscreen name buffer
		stccpy(data->config->pubscreen_name, node->node.ln_Name, sizeof(data->config->pubscreen_name));

		// Remove :Use
		for (a = strlen(data->config->pubscreen_name); a >= 0; a--)
		{
			if (data->config->pubscreen_name[a] == ':')
			{
				data->config->pubscreen_name[a] = 0;
				break;
			}
		}
	}

	// Initialise settings
	_config_env_screenmode_init(data, 0);
}

void _config_env_screenmode_check_values(config_env_data *data)
{
	// Check width and height
	BoundsCheckGadget(
		data->option_list, GAD_ENVIRONMENT_SCREENMODE_WIDTH, data->mode_size_limit.MinX, data->mode_size_limit.MaxX);
	BoundsCheckGadget(
		data->option_list, GAD_ENVIRONMENT_SCREENMODE_HEIGHT, data->mode_size_limit.MinY, data->mode_size_limit.MaxY);

	// Check depth
	if (data->config->screen_depth < 1)
		data->config->screen_depth = 1;
	else if (data->config->screen_depth > data->mode_max_colours)
		data->config->screen_depth = data->mode_max_colours;

	// Set depth value
	SetGadgetValue(data->option_list, GAD_ENVIRONMENT_SCREENMODE_COLORS, data->config->screen_depth);

	// Display colours string
	_config_env_screenmode_show_depth(data);
}

// Display colours string
void _config_env_screenmode_show_depth(config_env_data *data)
{
	char string[20];

	// Build display string
	if (data->config->screen_depth <= 16)
		lsprintf(string, "%ld", 1 << data->config->screen_depth);
	else
		lsprintf(string, "%ld bit", data->config->screen_depth);

	// Show string
	SetGadgetValue(data->option_list, GAD_ENVIRONMENT_COLORS_DISPLAY, (ULONG)string);
}
