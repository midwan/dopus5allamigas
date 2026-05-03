#include "config_lib.h"

Cfg_Function *LIBFUNC L_Config_EditFunction(REG(a0, IPCData *ipc),
											REG(a1, IPCData *opus_ipc),
											REG(a2, struct Window *window),
											REG(a3, Cfg_Function *function),
                                            REG(d0, APTR memory),
                                            REG(d1, IPTR command_list))
{
	FunctionStartup *startup;
	Cfg_Function *edit_func;
	IPCData *editor = 0;
	IPCMessage *msg;

	// Allocate startup data
	if (!(startup = AllocVec(sizeof(FunctionStartup), MEMF_CLEAR)))
		return 0;

	// Copy function (if it is supplied)
	if (function)
		edit_func = CopyFunction(function, 0, 0);

	// Otherwise, allocate a new function
	else
		edit_func = NewFunction(0, 0);

	// Invalid function to edit?
	if (!edit_func)
	{
		FreeVec(startup);
		return 0;
	}

	// Initialise startup data
	function_editor_init(startup, command_list);
	startup->window = window;
	startup->function = edit_func;
	startup->owner_ipc = ipc;
	startup->main_owner = opus_ipc;
	startup->flags |= FUNCEDF_CENTER | FUNCEDF_SAVE;

	// Launch editor
	if (!(IPC_Launch(0,
					 &editor,
					 "dopus_function_editor",
					 IPC_NATIVE(FunctionEditor),
					 STACK_DEFAULT,
					 (IPTR)startup,
					 (struct Library *)DOSBase)) ||
		!editor)
	{
		// Failed
		FreeVec(startup);
		FreeFunction(edit_func);
		return 0;
	}

	// Clear function pointer
	function = 0;

	// Wait on message port
	FOREVER
	{
		BOOL quit_flag = 0;

		// Get messages
		while ((msg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Command comes back
			if (msg->command == FUNCTIONEDIT_RETURN)
			{
				FunctionReturn *ret;

				// Get return packet
				ret = (FunctionReturn *)msg->data;

				// Copy returned function
				function = CopyFunction(ret->function, memory, 0);
			}

			// Function editor returned
			else if (msg->command == IPC_GOODBYE)
			{
				// Editor?
				if (msg->data == (IPCData *)editor)
					quit_flag = 1;
			}

			// Quit/hide/activate/show - pass on
			else if (msg->command == IPC_QUIT || msg->command == IPC_HIDE || msg->command == IPC_SHOW ||
					 msg->command == IPC_ACTIVATE)
			{
				// Pass through
				IPC_Command(editor, msg->command, msg->flags, msg->data, 0, 0);
			}

			// Reply to message
			IPC_Reply(msg);
		}

		// Check quit flag
		if (quit_flag)
			break;

		// Wait for message
		Wait(1 << ipc->command_port->mp_SigBit);
	}

	// Return the new function
	return function;
}
