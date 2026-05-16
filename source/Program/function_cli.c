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
#include "eliza.h"

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	BPTR old_output;
	BPTR old_input;
	struct MsgPort *old_console;
	BPTR output;
	BPTR input;
	char *name;
	struct MsgPort *reply_port;
} CLIData;

#ifndef __amigaos3__
	#pragma pack()
#endif

void print_string(char *);
BOOL cli_open(CLIData *);
void cli_close(CLIData *);
void cli_free(CLIData *);

static void cli_ulong_to_string(ULONG value, char *buffer, int buffer_size)
{
	int pos = buffer_size - 1;

	if (buffer_size < 2)
		return;

	buffer[pos] = 0;
	do
	{
		buffer[--pos] = '0' + (value % 10);
		value /= 10;
	} while (value && pos > 0);

	strcpy(buffer, buffer + pos);
}

static void cli_build_window_name(char *buffer, int buffer_size)
{
	char top[12];
	char *screen = get_our_pubscreen();
	ULONG top_edge = (GUI->screen_pointer) ? GUI->screen_pointer->BarHeight + 1 : 20;

	cli_ulong_to_string(top_edge, top, sizeof(top));

	buffer[0] = 0;
	StrConcat(buffer, environment->env->output_device, buffer_size);
	StrConcat(buffer, "0/", buffer_size);
	StrConcat(buffer, top, buffer_size);
	StrConcat(buffer, "/512/150/DOpus 5 CLI/CLOSE/SCREEN ", buffer_size);
	if (screen)
		StrConcat(buffer, screen, buffer_size);
}

// Internal command line interpreter
DOPUS_FUNC(function_cli)
{
	CLIData data = {0};
	short pos = 0;
	char ch;
	CommandList *last_cmd = 0;
	struct eliza_data *edata;
	short eliza_state = 0;

	// Output filename
	cli_build_window_name(handle->temp_buffer, sizeof(handle->temp_buffer));

	// Allocate name copy
	if (!(data.name = AllocVec(strlen(handle->temp_buffer) + 1, 0)))
		return 0;

	// Copy name
	strcpy(data.name, handle->temp_buffer);

	// Create reply port for functions
	if (!(data.reply_port = CreateMsgPort()))
	{
		cli_free(&data);
		return 0;
	}

	// Open CLI file
	if (!(cli_open(&data)))
	{
		cli_free(&data);
		return 0;
	}

	// Output information
	print_string(
		"\nDirectory Opus 5 Command Line Interpreter v0.04\n"
		"© Copyright 1998 by Jonathan Potter\n\n");
	print_string(GetString(&locale, MSG_CLI_TYPE_HELP));

	// Output prompt string
	print_string("> ");

	// Initialise Eliza
	if ((edata = AllocVec(sizeof(struct eliza_data), MEMF_CLEAR)) && init_eliza(edata))
		eliza_state = 1;

	// Wait for characters
	while (1)
	{
		BOOL ok = 1;
		IPCMessage *msg;

		// Any messages?
		while ((msg = (IPCMessage *)GetMsg(handle->ipc->command_port)))
		{
			// Quit?
			if (msg->command == IPC_ABORT || msg->command == IPC_QUIT)
				ok = 0;

			// Hide?
			else if (msg->command == IPC_HIDE && data.input)
				cli_close(&data);

			// Show?
			else if (msg->command == IPC_SHOW && !data.input)
			{
				if (!(cli_open(&data)))
					ok = 0;
				else
					print_string("\n> ");
			}

			// Reply to message
			IPC_Reply(msg);
		}

		// Quit?
		if (!ok)
			break;

		// Input handle?
		if (data.input)
		{
			// Wait for characters
			ok = WaitForChar(data.input, 500000);

			// Pending quit?
			if (GUI->flags & GUIF_PENDING_QUIT)
				break;

			// If no characters waiting, loop around
			if (!ok)
				continue;
		}

		// Otherwise
		else
		{
			// Wait for a message
			WaitPort(handle->ipc->command_port);
			continue;
		}

		// Read character
		if (Read(data.input, &ch, 1) != 1)
		{
			// End of file
			break;
		}

		// End of line?
		if (ch == '\n')
		{
			// Null-terminate buffer
			handle->work_buffer[pos] = 0;

			// Anything in buffer?
			if (pos > 0 || last_cmd)
			{
				BOOL wait_reply = 1;
				char *ptr;

				// Response to a command?
				if (last_cmd)
				{
					// Copy input string to temporary area
					strcpy(handle->work_buffer + 512, handle->work_buffer);

					// Build command string
					lsprintf(handle->work_buffer, "%s %s", last_cmd->name, handle->work_buffer + 512);
				}

				// Quit?
				if (stricmp(handle->work_buffer, "quit") == 0)
					break;

				// Fun
				if (stricmp(handle->work_buffer, "joshua") == 0)
				{
					print_string("Greetings Professor Falken.\n");
				}

				// In eliza mode?
				else if (eliza_state == 2)
				{
					// Pass the line to Eliza
					if (!eliza_line(edata, handle->work_buffer))
					{
						// Not in eliza any more
						eliza_state = 1;
					}
				}

				// Otherwise, go into Eliza?
				else if (eliza_state == 1 && stricmp(handle->work_buffer, "help me eliza!") == 0)
				{
					print_string("How can I help you?\n\n");
					eliza_state = 2;
				}

				// Help?
				else if (strnicmp(handle->work_buffer, "help", 4) == 0)
				{
					CommandList *command;

					// Get pointer to word after "help"
					ptr = handle->work_buffer + 4;
					rexx_skip_space(&ptr);

					// Lock command list
					lock_listlock(&GUI->command_list, FALSE);

					// Match internal command?
					if ((command = function_find_internal(&ptr, 0)))
					{
						// Template?
						if (command->template && *command->template)
							print_string(command->template);
						else
							print_string(GetString(&locale, MSG_CLI_NO_TEMPLATE));
						print_string("\n");
					}

					// Otherwise, show list of commands?
					else if (stricmp(ptr, "list") == 0)
					{
						for (command = (CommandList *)GUI->command_list.list.lh_Head; command->node.mln_Succ;
							 command = (CommandList *)command->node.mln_Succ)
						{
							// Not private?
							if (!(command->flags & FUNCF_PRIVATE))
							{
								lsprintf(handle->work_buffer, "%s", command->name);
								if (command->flags & FUNCF_EXTERNAL_FUNCTION)
								{
									strcat(handle->work_buffer, " (");
									if (strcmp(command->stuff.module_name, "!") == 0)
										strcat(handle->work_buffer, "temp");
									else
										strcat(handle->work_buffer, command->stuff.module_name);
									strcat(handle->work_buffer, ")");
								}
								strcat(handle->work_buffer, "\n");
								print_string(handle->work_buffer);
							}
						}
					}

					// General help
					else if (!*ptr)
					{
						print_string(GetString(&locale, MSG_CLI_HELP));
					}

					// Unknown command
					else
					{
						print_string(GetString(&locale, MSG_CLI_BAD_CMD));
					}

					// Unlock command list
					unlock_listlock(&GUI->command_list);

					// Output a blank line
					print_string("\n");
				}

				// Otherwise
				else
				{
					CommandList *cmd;

					// Asynchronous function?
					if (handle->work_buffer[pos - 1] == '&')
					{
						// Yep
						handle->work_buffer[pos - 1] = 0;
						wait_reply = 0;
					}

					// Get pointer to start of string, bump past whitespaces
					ptr = handle->work_buffer;
					rexx_skip_space(&ptr);

					// Rexx message?
					if (handle->work_buffer[0] == '+')
					{
						// Got ARexx?
						if (RexxSysBase)
						{
							struct RexxMsg *msg;
							struct MsgPort *rexx_port;
							BOOL free = 1;

							// Allocate message
							if ((msg = BuildRexxMsgExTags(
									 data.reply_port, 0, 0, RexxTag_Arg0, handle->work_buffer + 1, TAG_END)))
							{
								// Asynchronous?
								if (!wait_reply)
									msg->rm_Node.mn_ReplyPort = 0;
								else
									msg->rm_Action |= RXFF_RESULT;

								// Find rexx port
								Forbid();
								if ((rexx_port = FindPort(GUI->rexx_port_name)))
								{
									// Send message
									PutMsg(rexx_port, (struct Message *)msg);
									Permit();

									// Wait for reply?
									if (wait_reply)
									{
										// Get reply
										WaitPort(data.reply_port);
										GetMsg(data.reply_port);

										// Got result string?
										if (msg->rm_Result2)
										{
											// Output result
											print_string("--> ");
											print_string((char *)msg->rm_Result2);

											// Free result
											DeleteArgstring((STRPTR)msg->rm_Result2);
										}

										// Output result code
										else
										{
											char buf[16];

											// Build result string
											lsprintf(buf, "RC: %ld", msg->rm_Result1);
											print_string(buf);
										}

										// Add a blank line
										print_string("\n\n");
									}

									// Otherwise don't free message
									else
										free = 0;
								}
								else
									Permit();

								// Free the message
								if (free)
									FreeRexxMsgEx(msg);
							}
						}
					}

					// Otherwise, internal function?
					else if ((cmd = function_find_internal(&ptr, 0)))
					{
						Cfg_Function *function;

						// Display template?
						if (ptr && *ptr == '?' && *(ptr + 1) == 0 && !last_cmd)
						{
							// Print template if valid
							if (cmd->template)
								print_string(cmd->template);
							print_string(": ");

							// Save command
							last_cmd = cmd;
						}

						// Create dummy function
						else if ((function = new_default_function(handle->work_buffer, handle->memory)))
						{
							struct Message reply_msg;

							// Clear 'last command' pointer
							last_cmd = 0;

							// Set flag to free function
							function->function.flags2 |= FUNCF2_FREE_FUNCTION;

							// Initialise reply message
							reply_msg.mn_ReplyPort = data.reply_port;

							// Execute function
							function_launch(
								FUNCTION_RUN_FUNCTION, function, 0, 0, 0, 0, 0, 0, 0, (wait_reply) ? &reply_msg : 0, 0);

							// Wait for reply?
							if (wait_reply)
							{
								// Get reply
								WaitPort(data.reply_port);
								GetMsg(data.reply_port);
							}
						}

						// Clear 'last command' pointer
						else
							last_cmd = 0;
					}

					// Otherwise pass to dos
					else
					{
						CLI_Launch(handle->work_buffer,
								   (struct Screen *)-1,
								   0,
								   Open("console:", MODE_OLDFILE),
								   0,
								   wait_reply | LAUNCHF_USE_STACK,
								   environment->env->default_stack);
					}
				}

				// Restart buffer
				pos = 0;
			}

			// Output prompt string
			if (!last_cmd)
				print_string("> ");
		}

		// Store in buffer (unless full)
		else if (pos < 255)
			handle->work_buffer[pos++] = (char)ch;
	}

	// Clean up
	cli_close(&data);
	cli_free(&data);
	if (edata)
	{
		free_eliza(edata);
		FreeVec(edata);
	}

	return 0;
}

void print_string(char *str)
{
	PutStr(str);
	Flush(Output());
}

// Open CLI
BOOL cli_open(CLIData *data)
{
	// Initialise
	data->old_output = Output();
	data->old_input = Input();
	data->old_console = GetConsoleTask();
	data->input = 0;

	// Try and open console window
	if (!(data->output = Open(data->name, MODE_NEWFILE)))
		return 0;

	// Make this the output channel
	SelectOutput(data->output);

	// Make this the console handler
	SetConsoleTask(((struct FileHandle *)BADDR(data->output))->fh_Type);

	// Duplicate output channel for input
	if (!(data->input = Open("console:", MODE_OLDFILE)))
	{
		SelectOutput(data->old_output);
		SelectInput(data->old_input);
		SetConsoleTask(data->old_console);
		Close(data->output);
		data->output = 0;
		return 0;
	}

	// Set input channel
	SelectInput(data->input);

	return 1;
}

// Close CLI
void cli_close(CLIData *data)
{
	// Set things back the way they were
	SelectOutput(data->old_output);
	SelectInput(data->old_input);
	SetConsoleTask(data->old_console);

	// Close input/output channels
	if (data->input)
		Close(data->input);
	if (data->output)
		Close(data->output);

	// Clear handles
	data->input = 0;
	data->output = 0;
}

// Free CLI
void cli_free(CLIData *data)
{
	if (data->reply_port)
		DeleteMsgPort(data->reply_port);
	if (data->name)
		FreeVec(data->name);
}
