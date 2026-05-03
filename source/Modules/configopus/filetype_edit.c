#include "config_lib.h"
#include "config_filetypes.h"
#include <Program/main_commands.h>

// Edit a filetype (library function)

Cfg_Filetype *LIBFUNC L_EditFiletype(REG(a0, Cfg_Filetype *type),
									 REG(a1, struct Window *window),
									 REG(a2, IPCData *ipc),
									 REG(a3, IPCData *dopus_ipc),
									 REG(d0, ULONG flags))
{
	filetype_ed_data *data;
	IPCData *editor;
	IPCMessage *imsg;
	Cfg_Filetype *ret_type = 0;

	// Valid filetype?
	if (!type)
		return 0;

	// Allocate data for the editor, copy filetype
	if (!(data = AllocVec(sizeof(filetype_ed_data), MEMF_CLEAR)) || !(data->type = CopyFiletype(type, 0)))
	{
		FreeVec(data);
		return 0;
	}

	// Fill out data
	data->owner_ipc = ipc;
	data->action_lookup = filetype_action_lookup;

	// Supply libraries
	/*data->func_startup.dopus_base=DOpusBase;
	data->func_startup.dos_base=(struct Library *)DOSBase;
	data->func_startup.int_base=(struct Library *)IntuitionBase;
	data->func_startup.util_base=(struct Library *)UtilityBase;
	data->func_startup.cx_base=CxBase;
	data->func_startup.wb_base=WorkbenchBase;
	data->func_startup.gfx_base=(struct Library *)GfxBase;
	data->func_startup.asl_base=AslBase;
	data->func_startup.layers_base=LayersBase;*/

	// Supply locale
	data->func_startup.locale = locale;

	// Supply gui data pointers
	data->new_win.parent = window;
	data->new_win.dims = &_filetype_editor_window;
	data->new_win.locale = locale;
	data->obj_def = _filetype_editor_objects;

	// Supply data pointers for function editor
	data->func_startup.win_def = &_function_editor_window;
	data->func_startup.obj_def = _function_editor_objects;
	data->func_startup.func_labels = _function_type_labels;
	data->func_startup.flag_list = _funced_flaglist;
	data->func_startup.func_list = IPC_Command(dopus_ipc, MAINCMD_GET_LIST, 0, 0, 0, REPLY_NO_PORT);
	data->func_startup.flags = FUNCEDF_NO_KEY;
	data->func_startup.main_owner = ipc;

	// Supply data pointers for the fileclass editor
	data->class_win = &_fileclass_editor_window;
	data->class_obj = _fileclass_editor_objects;
	data->class_lookup = fileclass_match_lookup;
	data->class_strings = matchtype_labels;

	// Open class editor automatically?
	if (flags & EFTF_EDIT_CLASS)
		data->edit_flag = 1;

	// Launch editor
	if (!(IPC_Launch(0,
					 &editor,
					 "dopus_filetype_editor",
					 (ULONG)IPC_NATIVE(FiletypeEditor),
					 STACK_DEFAULT,
					 (ULONG)data,
					 (struct Library *)DOSBase)) ||
		!editor)
	{
		// Failed; free data
		FreeFiletype(data->type);
		FreeVec(data);
		return 0;
	}

	// Event loop
	FOREVER
	{
		BOOL quit = 0;

		// IPC messages
		while ((imsg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Filetype returned?
			if (imsg->command == FILETYPEEDIT_RETURN)
			{
				// Copy the filetype
				ret_type = CopyFiletype((Cfg_Filetype *)imsg->flags, 0);
			}

			// Said goodbye?
			else if (imsg->command == IPC_GOODBYE && imsg->data == editor)
				quit = 1;

			// Abort?
			else if (imsg->command == IPC_QUIT)
			{
				// Send abort message
				IPC_Quit(editor, imsg->flags, TRUE);
			}

			// Activate?
			else if (imsg->command == IPC_ACTIVATE)
			{
				// Send message
				IPC_Command(editor, IPC_ACTIVATE, imsg->flags, imsg->data, 0, 0);
			}

			// Identify?
			else if (imsg->command == IPC_IDENTIFY)
			{
				// Pass to DOpus
				IPC_Command(dopus_ipc, IPC_IDENTIFY, imsg->flags, imsg->data, 0, REPLY_NO_PORT);
			}

			// Reply to message
			IPC_Reply(imsg);
		}

		// Check quit flag
		if (quit)
			break;

		// Wait for message
		WaitPort(ipc->command_port);
	}

	// Return new filetype (if any)
	return ret_type;
}
