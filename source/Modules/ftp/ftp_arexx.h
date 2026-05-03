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

#ifndef _AREXX_H
#define _AREXX_H

// For rexx_lst_prog_init()
#define PROGRESS_LISTER (0)
#define PROGRESS_FREE (1)
#define PROGRESS_NO_NAME (NULL)
#define PROGRESS_FILE_ON (TRUE)
#define PROGRESS_FILE_OFF (FALSE)
#define PROGRESS_BAR_ON (TRUE)
#define PROGRESS_BAR_OFF (FALSE)

// for rexx_lst_refresh()
enum { REFRESH_NODATE, REFRESH_DATE };

enum {
	REXX_REPLY_NONE,
	REXX_REPLY_RESULT,
	REXX_REPLY_RC,
};

void rexx_doubleclick(const char *opus, const char *path);
char *rexx_getstring(const char *opus, ULONG handle, char *text, BOOL secure, int length, char *deflt, char *buttons);
void rexx_lst_add(const char *opus,
				  ULONG handle,
				  char *name,
				  unsigned int size,
				  int type,
				  ULONG seconds,
				  LONG prot,
				  char *comment);
void rexx_lst_refresh(const char *opus, ULONG handle, int date);
int rexx_lst_findcache(const char *opus, ULONG handle, char *path);
void rexx_lst_busy(const char *opus, ULONG handle, int val);
void rexx_lst_empty(const char *opus, ULONG handle);
void rexx_lst_clear(const char *opus, ULONG handle);
void rexx_lst_close(const char *opus, ULONG handle);
void rexx_lst_label(const char *opus, ULONG handle, char *pref, char *label, char *suff);
void rexx_lst_lock(const char *opus, ULONG handle);
ULONG rexx_lst_new(const char *opus, ULONG handle, char *host, const char *toolbar);
ULONG rexx_lst_query_dest1(const char *opus);
char *rexx_lst_query_entry(const char *opus, ULONG handle, char *entry);
BOOL rexx_lst_query_handler(const char *opus, ULONG handle);
int rexx_lst_query_numentries(const char *opus, ULONG handle);
int rexx_lst_query_numfiles(const char *opus, ULONG handle);
int rexx_lst_query_numseldirs(const char *opus, ULONG handle);
int rexx_lst_query_numselentries(const char *opus, ULONG handle);
int rexx_lst_query_numselfiles(const char *opus, ULONG handle);
char *rexx_lst_query_path(const char *opus, ULONG handle);
ULONG rexx_lst_query_src1(const char *opus);
int rexx_lst_query_visible(const char *opus, ULONG handle);
void rexx_lst_remove(const char *opus, ULONG handle, char *name);
void rexx_lst_select(const char *opus, ULONG handle, char *name, int state);
void rexx_lst_set_path(const char *opus, ULONG handle, char *path);
void rexx_lst_title(const char *opus, ULONG handle, char *host);
char *rexx_lst_title_swap(const char *opus, ULONG handle, char *host);
void rexx_prog_bar(const char *opus, ULONG handle, int type, int total, int count);
void rexx_prog_bytes(const char *opus, ULONG handle, int type, int total, int count);
void rexx_prog_clear(const char *opus, ULONG handle, int type);
void rexx_prog_info(const char *opus, ULONG handle, int type, char *info);
void rexx_prog_info2(const char *opus, ULONG handle, int type, char *info2);
void rexx_prog_info3(const char *opus, ULONG handle, int type, char *info3);
ULONG rexx_prog_init(const char *opus, ULONG handle, int type, char *title, char *info, char *name, int file, int bar);
ULONG rexx_prog_init3(const char *opus,
					  ULONG handle,
					  int type,
					  char *title,
					  char *info,
					  char *info2,
					  char *info3,
					  char *name,
					  int file,
					  int bar);
void rexx_prog_name(const char *opus, ULONG handle, int type, char *name);

#define rexx_lst_unlock(o, h) rexx_lst_busy(o, h, 0)

// Send an arexx command, optionally get a result
LONG send_rexx(const char *port, int reply, const char *cmd);
STDARGS LONG send_rexxa(const char *port, int reply, const char *fmt, ...);

// Reply to an ARexx message we received
void reply_rexx(struct RexxMsg *msg, LONG r1, LONG r2);

// Reply to all messages in an ARexx port
void flush_arexxport(struct MsgPort *port);

#endif
