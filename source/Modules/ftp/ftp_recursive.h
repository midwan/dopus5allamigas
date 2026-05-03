/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

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

#ifndef _FTP_RECURSIVE_H
#define _FTP_RECURSIVE_H

#include "ftp_lister.h"

// End of a connection for recursive transfer - FTP or file system
#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct _endpoint
{
	int ep_type;				  // FTP or file system?
	BOOL ep_freenode;			  // ftp_node needs to be freed?
	struct ftp_node *ep_ftpnode;  // FTP (and filesys) info
	struct rec_entry_list *(*ep_list)(struct _endpoint *, char *);
	int (*ep_cwd)(struct _endpoint *, char *);
	int (*ep_cdup)(struct _endpoint *);
	int (*ep_mkdir)(struct _endpoint *, char *);
	int (*ep_dele)(struct _endpoint *, struct entry_info *);
	int (*ep_rmd)(struct _endpoint *, struct entry_info *);
	int (*ep_port)(struct _endpoint *, struct sockaddr_in *, ULONG flags);
	struct sockaddr_in *(*ep_pasv)(struct _endpoint *);
	int (*ep_rest)(struct _endpoint *, unsigned int);
	int (*ep_retr)(struct _endpoint *, char *filename);
	int (*ep_stor)(struct _endpoint *, char *filename);
	int (*ep_abor)(struct _endpoint *);
	int (*ep_setdate)(struct _endpoint *, char *name, ULONG seconds);
	int (*ep_chmod)(struct _endpoint *, char *name, ULONG mode);
	int (*ep_setnote)(struct _endpoint *, char *name, char *comment);
	int (*ep_setprot)(struct _endpoint *, char *name, LONG prot);
	int (*ep_select)(struct _endpoint *);
	int (*ep_getreply)(struct _endpoint *, ULONG flags);
	struct entry_info *(*ep_getentry)(struct _endpoint *, char *name);
	int (*ep_errorreq)(struct _endpoint *, struct ftp_node *, ULONG flags);
	unsigned long (*ep_opts)(struct _endpoint *, int type);

} endpoint;
#ifndef __amigaos3__
	#pragma pack()
#endif

extern const char *months[];

// List of entries in a directory
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct rec_entry_list
{
	struct List rl_list;
	int rl_entry_count;	 // How many entries in the list
};
#ifndef __amigaos3__
	#pragma pack()
#endif

// Tags for create_endpoint()
#define EP_TYPE (TAG_USER + 0)
#define EP_FTPNODE (TAG_USER + 1)
#define EP_PATH (TAG_USER + 2)

// Endpoint types
enum {
	ENDPOINT_FILESYS,	// Local filesystem
	ENDPOINT_FTP,		// FTP server
	ENDPOINT_HTTPPROXY	// Just dreaming here!
};

//
//	Structure used by all recursive hook functions
//
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct hook_rec_data
{
	struct opusftp_globals *hc_og;
	endpoint *hc_source;
	endpoint *hc_dest;
	int (*hc_pre)(struct hook_rec_data *, char *dirname, struct rec_entry_list *dest_list, struct entry_info *);
	BOOL hc_need_dest;	// Do we need to LIST the destination?

	struct update_info hc_ui;  // Used for xfer bar update

	char *hc_opus;				   // Opus port name for ARexx calls
	struct ftp_node *hc_prognode;  // FTP node of lister with progress bar
	ULONG hc_proghandle;		   // Handle of lister with progress bar
	struct ftp_environment *hc_env;
	char hc_fromdirname[FILENAMELEN + 1];  // From 'xxx'
	char hc_todirname[FILENAMELEN + 1];	   // To 'yyy'
	char *hc_basedirname;				   // Used to check if source entry is visible in lister

	char *hc_newname;  // New name for CopyAs

	unsigned long hc_misc_bytes;  // Resume point and GetSizes accumulator

	ULONG hc_copy_flags;	   // Copy of xm_flags
	ULONG hc_ored_ftp_flags;   // Flags set for 1 any FTP servers
	ULONG hc_anded_ftp_flags;  // Flags set for all FTP servers
	ULONG hc_recur_flags;
	ULONG hc_options;  // Copy/Delete options for ftp site (src for getput)

	char hc_url[1024 + 1];	// Buffer for building URLs in

	char hc_pattern[FILENAMELEN * 2 + 1];  // For pattern matchine in FindFile
	char hc_resultpath[PATHLEN + 1];	   // Where we found a match
	short hc_match_comment;
};
#ifndef __amigaos3__
	#pragma pack()
#endif

struct rec_favour_xfer
{
	int (*updatefn)(void *, unsigned int, unsigned int);
	void *updateinfo;
	char *remote_path;
	char *local_path;
	unsigned int restart;
};

//
//	Flags for recursive stuff
//
enum {
	RECURF_REPLACEALL = 1 << 0,	 // Like pressing 'Replace' on each requester
	RECURF_SKIPALL = 1 << 1,	 // Like pressing 'Skip' on each requester
	RECURF_RESUMEALL = 1 << 2,	 // Like pressing 'Resume' on each requester
	RECURF_NORECUR = 1 << 3,	 // Act on dir only, not subentries
	RECURF_BROKEN_LS = 1 << 4,	 // Workaround for server where names containing spaces break their ls
};

//
//	Special return values for recursive_getput
//
enum {
	REC_GETPUT_OK,			 // getput succeeded
	REC_GETPUT_ABORTED,		 // getput aborted
	REC_GETPUT_ERROR_START,	 // getput error at start of transfer
	REC_GETPUT_ERROR_END,	 // getput error at end of transfer
};

endpoint *create_endpoint(struct opusftp_globals *og, struct TagItem *tags);
#define create_endpoint_tags(og, ...)                  \
	({                                                 \
		IPTR _tags[] = {__VA_ARGS__};                  \
		create_endpoint(og, (struct TagItem *)&_tags); \
	})
void delete_endpoint(endpoint *);

void free_entry_list(struct rec_entry_list *);

int recursive_copy(struct hook_rec_data *,
				   char *dirname,
				   char *fulldirname,
				   struct entry_info *,
				   struct rec_entry_list *dest_list);
int recursive_delete(struct hook_rec_data *, char *dirname, struct entry_info *);
int recursive_protect(struct hook_rec_data *, struct entry_info *, LONG set, LONG clr);
int recursive_getsizes(struct hook_rec_data *, struct entry_info *);
int recursive_findfile(struct hook_rec_data *, char *fulldirname, struct entry_info *);

struct rec_entry_list *rec_filesys_list(endpoint *, char *);
int rec_filesys_cwd(endpoint *, char *);
int rec_filesys_cdup(endpoint *);
int rec_filesys_mkdir(endpoint *, char *);
int rec_filesys_dele(endpoint *, struct entry_info *);
int rec_filesys_rmd(endpoint *, struct entry_info *);
int rec_filesys_port(endpoint *, struct sockaddr_in *, ULONG flags);
struct sockaddr_in *rec_filesys_pasv(endpoint *);
int rec_filesys_rest(endpoint *, unsigned int);
int rec_filesys_retr(endpoint *, char *filename);
int rec_filesys_stor(endpoint *, char *filename);
int rec_filesys_setdate(endpoint *, char *name, ULONG seconds);
int rec_filesys_chmod(endpoint *, char *name, ULONG mode);
int rec_filesys_setnote(endpoint *, char *name, char *comment);
int rec_filesys_setprot(endpoint *, char *name, LONG prot);
int rec_filesys_getreply(endpoint *, ULONG flags);
struct entry_info *rec_filesys_getentry(endpoint *, char *name);
int rec_filesys_errorreq(endpoint *, struct ftp_node *, ULONG flags);
unsigned long rec_filesys_opts(endpoint *, int type);
int rec_filesys_dummy(endpoint *, struct entry_info *);
struct rec_entry_list *rec_ftp_list(endpoint *, char *);
int rec_ftp_cwd(endpoint *, char *);
int rec_ftp_cdup(endpoint *);
int rec_ftp_mkdir(endpoint *, char *);
int rec_ftp_dele(endpoint *, struct entry_info *);
int rec_ftp_rmd(endpoint *, struct entry_info *);
int rec_ftp_port(endpoint *, struct sockaddr_in *, ULONG flags);
struct sockaddr_in *rec_ftp_pasv(endpoint *);
int rec_ftp_rest(endpoint *, unsigned int);
int rec_ftp_retr(endpoint *, char *filename);
int rec_ftp_stor(endpoint *, char *filename);
int rec_ftp_abor(endpoint *);
int rec_ftp_chmod(endpoint *, char *name, ULONG mode);
int rec_ftp_select(endpoint *);
int rec_ftp_getreply(endpoint *, ULONG flags);
struct entry_info *rec_ftp_getentry(endpoint *, char *name);
int rec_ftp_errorreq(endpoint *, struct ftp_node *, ULONG flags);
unsigned long rec_ftp_opts(endpoint *, int type);
int rec_ftp_dummy(endpoint *, struct entry_info *);

#endif
