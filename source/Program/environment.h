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

#ifndef DOPUS_ENVIRONMENT
#define DOPUS_ENVIRONMENT

Cfg_Environment *environment_new(void);
void environment_free(Cfg_Environment *env);
BOOL environment_open(Cfg_Environment *env, char *name, BOOL, APTR);
int environment_save(Cfg_Environment *env, char *name, short, CFG_ENVR *);
void environment_resolve_missing_banks(Cfg_Environment *env, struct Window *parent);
IPC_EntryProto(environment_proc, extern);
ButtonBankNode *env_read_open_bank(APTR, Cfg_Environment *, ULONG);
OpenListerNode *env_read_open_lister(APTR, Cfg_Environment *, ULONG);
void free_user_menus(void);
void env_free_desktop(struct MinList *list);
void env_update_pathlist(void);
void env_fix_mufs(void);
void env_fix_themes(void);

#define ENVSAVE_LAYOUT (1 << 0)
#define ENVSAVE_WINDOW (1 << 1)

#endif
