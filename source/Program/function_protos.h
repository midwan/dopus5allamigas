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

#ifndef _DOPUS_FUNCTION_PROTOS
#define _DOPUS_FUNCTION_PROTOS

#define DOPUS_FUNC(name) \
	int name(struct _CommandList *command, FunctionHandle *handle, char *args, struct _InstructionParsed *instruction)

DOPUS_FUNC(function_all);
DOPUS_FUNC(function_none);
DOPUS_FUNC(function_toggle);
DOPUS_FUNC(function_copy);
DOPUS_FUNC(function_copy);
DOPUS_FUNC(function_delete);
DOPUS_FUNC(function_makedir);
DOPUS_FUNC(function_select);
DOPUS_FUNC(function_devicelist);
DOPUS_FUNC(function_change);
DOPUS_FUNC(function_scandir);
DOPUS_FUNC(function_rename);
DOPUS_FUNC(function_dirtree);
DOPUS_FUNC(function_clearbuffers);
DOPUS_FUNC(function_show);
DOPUS_FUNC(function_iconinfo);
DOPUS_FUNC(function_disk);
DOPUS_FUNC(function_loadbuttons);
DOPUS_FUNC(function_leaveout);
DOPUS_FUNC(function_runprog);
DOPUS_FUNC(function_addicon);
DOPUS_FUNC(function_search);
DOPUS_FUNC(function_bufferlist);
DOPUS_FUNC(function_getsizes);
DOPUS_FUNC(function_hunt);
DOPUS_FUNC(function_loadenvironment);
DOPUS_FUNC(function_parent);
DOPUS_FUNC(function_user);
DOPUS_FUNC(function_verify);
DOPUS_FUNC(function_cli);
DOPUS_FUNC(function_printdir);
DOPUS_FUNC(function_set);
DOPUS_FUNC(function_closebuttons);
DOPUS_FUNC(function_special);
DOPUS_FUNC(function_reselect);
DOPUS_FUNC(function_assign);
DOPUS_FUNC(function_configure);
DOPUS_FUNC(function_deletefile);
DOPUS_FUNC(function_setbackground);
DOPUS_FUNC(function_runcommand);
DOPUS_FUNC(function_editcommand);
DOPUS_FUNC(function_reset);
DOPUS_FUNC(function_loadfonts);
DOPUS_FUNC(function_stopsniffer);

BOOL rename_get_wild(char *, char *, char *, char *);
ExternalEntry *new_external_entry(FunctionHandle *handle, char *path);

void network_get_info(NetworkInfo *, DirBuffer *, struct DevProc *, UWORD, UWORD, ULONG);

void function_user_run(FunctionHandle *handle, Cfg_Function *function);

long DOpusDeleteFile(char *name, FunctionHandle *, struct _CommandList *, BOOL);

#endif
