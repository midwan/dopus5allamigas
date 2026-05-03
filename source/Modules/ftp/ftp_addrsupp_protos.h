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

/* Prototypes for functions defined in
ftp/ftp_addrsupp.c
 */

extern char const *logfile;

void Save_Config(struct display_globals *, BOOL);

BOOL save_sites(struct display_globals *dg, struct Window *win, short gadgetid);

BOOL save_options(struct window_params *, int, short);

Att_List *read_sites(struct window_params *, short);

Att_List *import_sites(struct window_params *, short);

struct ftp_config *read_options(struct window_params *, int);

void read_build_addressbook(struct opusftp_globals *, IPCData *);

struct connect_msg *get_blank_connectmsg(struct opusftp_globals *);

struct site_entry *get_blank_site_entry(struct opusftp_globals *og);

void sort_list(Att_List *);

int get_site_entry(struct opusftp_globals *og, struct site_entry *e, IPCData *ipc);

BOOL get_global_options(struct opusftp_globals *og);

void set_config_to_default(struct ftp_config *oc);

void copy_site_entry(struct opusftp_globals *og, struct site_entry *e, struct site_entry *old);

BPTR setup_config(struct opusftp_globals *);

void cleanup_config(BPTR);

BOOL export_sites(struct display_globals *dg, struct Window *win);
