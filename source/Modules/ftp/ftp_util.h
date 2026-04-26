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

#ifndef _OF_UTIL_H
#define _OF_UTIL_H

// Flags for display_msg
#define DSPMSG_RETRYABORT (1 << 0)

// Display a string, with optional Retry/Abort buttons
long display_msg(struct opusftp_globals *, IPCData *, struct Window *, ULONG flags, char *);

// Get the name of the logged-in user on the Amiga
char *getlogname(char *);

// Get the hostname of the user's Amiga (for anonymous FTPs)
char *getuseraddress(char *);

// Initialize a ListLock and counter
void ad_InitListLock(struct ListLock *, int *count);

// Remove a node from a ListLock
void ListLockRemove(struct ListLock *, struct Node *, int *count);

// Add a node to a ListLock
void ListLockAddHead(struct ListLock *, struct Node *, int *count);

// BOOL IsListLockEmpty( struct ListLock *l );

// Copy a string which is locked with a semaphore
void copy_locked_string(struct SignalSemaphore *, char *dest, char *src, int len);

// Convert a DateStamp to a number of seconds
ULONG datestamp_to_seconds(struct DateStamp *);

// Convert a number of seconds to a DateStamp
void seconds_to_datestamp(struct DateStamp *, ULONG secs);

// convert the number of seconds since Jan 1 1978 to an AmigaDOS time string
void seconds_to_datestring(ULONG seconds, char *buf);

int setfiledate_secs(char *path, ULONG secs);

ULONG getfiledate_secs(char *path);

ULONG get_curr_secs(void);

char **alloc_name_array(char *initstring);

// Convert value to a string in bytes, K, or megs
void cat_bytes(char *dest, unsigned long value);

// Count the number of quoted entries as passed via ARexx
int count_entries(char *);

// Convert a Unix line output by ls to an entry_info structure
int unix_line_to_entryinfo(struct entry_info *entry, const char *line, ULONG flags);

// Convert a Windows NT line output by ls to an entry_info structure
int nt_line_to_entryinfo(struct entry_info *entry, const char *line, ULONG flags);

// Convert an Amiga fileinfoblock to an entry_info structure
void fileinfoblock_to_entryinfo(struct entry_info *ei, struct FileInfoBlock *fib);

// Convert an MLSD line output by ls to an entry_info structure
int mlsd_line_to_entryinfo(struct entry_info *entry, const char *line, ULONG flags);

// Get final path from a pathname
void final_path(char *path, char *buf);

// Find an entry in a list
struct entry_info *rec_find_entry(struct rec_entry_list *, struct entry_info *, ULONG cmpflags);

#define BUILDURLF_INCLUDE_PASSWORD (1 << 0)

// Convert site details to a standard text URL (not including the ftp:// prefix)
void build_url(char *buffer,
			   const char *user,
			   const char *pass,
			   const char *host,
			   int port,
			   const char *path,
			   const char *entry);
void build_url_flags(char *buffer,
					 const char *user,
					 const char *pass,
					 const char *host,
					 int port,
					 const char *path,
					 const char *entry,
					 ULONG flags);

// Split a URL into its component parts
int split_url(const char *url, char *user, char *pass, char *host, long *port, char *path);

// Create a desktop function file
BOOL CreateFunctionFile(char *name, short type, char *instruction, char *icon);

// Generic requester
int ftpmod_request(struct opusftp_globals *og, IPCData *ipc, struct TagItem *taglist);
#define ftpmod_request_tags(og, ipc, ...)                  \
	({                                                     \
		IPTR _tags[] = {__VA_ARGS__};                      \
		ftpmod_request(og, ipc, (struct TagItem *)&_tags); \
	})

// Tags for ftpmod_request
#define FR_IPC (TAG_USER + 1000)		   // Mainly to override lister IPC
#define FR_FormatString (TAG_USER + 1001)  // printf-like formatting
#define FR_MsgNum (TAG_USER + 1002)		   // String number from catalog
#define FR_ButtonNum (TAG_USER + 1003)	   // As above for button text

// Check certain Opus options
unsigned long ftpmod_options(struct opusftp_globals *, int type);

// Open a temporary file
BPTR open_temp_file(char *filename, IPCData *ipc);

// Count the number of components in a path
int count_path_parts(const char *path);

// Forward an IPCMessage
int ipc_forward(IPCData *, IPCMessage *, ULONG flags);

// Convert Amiga protection flags to Unix style
ULONG prot_amiga_to_unix(LONG bits);

// Convert Unix protection flags to Amiga style
LONG prot_unix_to_amiga(ULONG mode);

// Get protection value from a string
long prot_from_string(char *buf);

#endif
