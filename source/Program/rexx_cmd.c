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

enum {
	EXCOM_DESC,
	EXCOM_TEMPLATE,
	EXCOM_PROGRAM,
	EXCOM_SOURCE,
	EXCOM_DEST,
	EXCOM_MENU,
	EXCOM_TYPE,
	EXCOM_PRIVATE,
	EXCOM_HELP,
	EXCOM_REMOVE,
	EXCOM_HANDLER,
	EXCOM_TEMP,
};

// Add a new command
void rexx_add_cmd(char *args)
{
	char name[40], program[50], desc[80], template[128], menu[40], buf[60], helpfile[50], *helpptr = 0;
	unsigned long flags = 0, ext_flags = 0;
	short opt, remove = 0, temp_flag = 0;
	Att_List *type_list = 0;

	// Get command
	rexx_parse_word(&args, name, 40);
	rexx_skip_space(&args);

	// Clear buffers
	desc[0] = 0;
	template[0] = 0;
	program[0] = 0;
	menu[0] = 0;

	// Options
	while ((opt = rexx_match_keyword(&args, excommand_keys, 0)) != -1)
	{
		// Which keyword?
		switch (opt)
		{
		// Description
		case EXCOM_DESC:
			rexx_parse_word(&args, desc, 80);
			break;

		// Template
		case EXCOM_TEMPLATE:
			rexx_parse_word(&args, template, 128);
			break;

		// Program
		case EXCOM_PROGRAM:
			rexx_parse_word(&args, program, 40);
			if (program[0] && !sufcmp(program, ".dopus5"))
				strcat(program, ".dopus5");
			break;

		// Source
		case EXCOM_SOURCE:
			flags |= FUNCF_WANT_SOURCE | FUNCF_SINGLE_SOURCE;
			break;

		// Destination
		case EXCOM_DEST:
			flags |= FUNCF_WANT_DEST | FUNCF_SINGLE_DEST;
			break;

		// Menu
		case EXCOM_MENU:
			rexx_parse_word(&args, menu, 40);
			break;

		// Type
		case EXCOM_TYPE:

			// Allocate type list if not already allocated
			if (!type_list && !(type_list = Att_NewList(LISTF_POOL)))
				break;

			// Get type and add to list
			rexx_parse_word(&args, buf, 60);

			// Add type to the list
			rexx_add_cmd_type(type_list, buf);
			break;

		// Private
		case EXCOM_PRIVATE:
			flags |= FUNCF_PRIVATE;
			break;

		// Help
		case EXCOM_HELP:
			rexx_parse_word(&args, helpfile, 40);
			if (helpfile[0] && !sufcmp(helpfile, ".guide"))
				strcat(helpfile, ".guide");
			helpptr = helpfile;
			break;

		// Remove
		case EXCOM_REMOVE:
			remove = 1;
			break;

		// For custom handler only
		case EXCOM_HANDLER:
			ext_flags |= POPUPEXTF_HANDLER;
			break;

		// Temporary
		case EXCOM_TEMP:
			temp_flag = 1;
			strcpy(program, "!");
			break;
		}

		// Skip spaces
		rexx_skip_space(&args);
	}

	// Remove?
	if (remove)
	{
		if (temp_flag)
			command_remove(name);
		else
			remove_popup_ext(name);
	}

	// Add command
	else
		add_command(name, desc, template, program, flags, menu, type_list, helpptr, ext_flags);

	// Free type list
	Att_RemList(type_list, 0);
}

// Add a type to the command
void rexx_add_cmd_type(Att_List *list, char *type)
{
	Cfg_FiletypeList *ftlist;
	Cfg_Filetype *ft;
	char pattern[120];
	short count = 0;

	// See if filetype type exists
	if (filetype_find(type, 1))
	{
		// Add straight to the list
		Att_NewNode(list, type, 0, 0);
		return;
	}

	// Parse type as a pattern
	ParsePatternNoCase(type, pattern, sizeof(pattern));

	// Lock filetype lists
	lock_listlock(&GUI->filetypes, FALSE);

	// Go through filetype lists
	for (ftlist = (Cfg_FiletypeList *)GUI->filetypes.list.lh_Head; ftlist->node.ln_Succ;
		 ftlist = (Cfg_FiletypeList *)ftlist->node.ln_Succ)
	{
		// Go through filetypes in this list
		for (ft = (Cfg_Filetype *)ftlist->filetype_list.lh_Head; ft->node.ln_Succ;
			 ft = (Cfg_Filetype *)ft->node.ln_Succ)
		{
			// Compare ID with pattern
			if (MatchPatternNoCase(pattern, ft->type.id))
			{
				// Add to the list
				Att_NewNode(list, ft->type.id, 0, 0);
				++count;
			}
		}
	}

	// Unlock filetype lists
	unlock_listlock(&GUI->filetypes);

	// Didn't add any?
	if (count == 0)
	{
		// Just add the straight command
		Att_NewNode(list, type, 0, 0);
	}
}
