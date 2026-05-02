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

/*****************************************************
 * gp 26.01.97	removed IPC_FRWITE
 *
 *
 */

#ifndef _IPC_H
#define _IPC_H

#include "ftp_opusftp.h"

// Sub-process data used when launching new processes
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct subproc_data
{
	struct opusftp_globals *spd_ogp;
	IPCData *spd_ipc;
	IPCData *spd_owner_ipc;
};
#ifndef __amigaos3__
	#pragma pack()
#endif

// Custom IPC message
enum {
	IPC_PRINT = 0x100,
	IPC_LIST,  // Asynchronous list update

	IPC_ADDRBOOK,  // Show address book requester
	IPC_CONNECT,   // Connect lister to an FTP site

	IPC_GET,	  // Retrieve files from an FTP site
	IPC_PUT,	  // Store files onto an FTP site
	IPC_GETPUT,	  // Transfer files from one FTP site to another
	IPC_MAKEDIR,  // Make a new directory on an FTP site
	IPC_RENAME,	  // Rename files and directories on an FTP site
	IPC_DELETE,	  // Delete files from an FTP site

	IPC_REREAD,	 // Re-read the current directory
	IPC_PARENT,	 // Change FTP directory to its parent
	IPC_ROOT,	 // Change FTP directory to its root
	IPC_CD,		 // Change to a new FTP directory

	IPC_DOUBLECLICK,  // Doubleclick on a file, directory or link
	IPC_TRAPTEMP,	  // Issue an Opus command that uses a temporary local file

	IPC_FTPCOMMAND,	 // Send a literal command to the FTP server
	IPC_SETVAR,		 // Set a variable (logfile, debug, timeout, list command)

	IPC_UPDATECONFIG,  // special - addressbook changed logfile description
	IPC_CDCACHE,	   // special for CD after a cache display

	IPC_FAVOUR,	   // Do FTP command as a favour
	IPC_EDIT,	   // Inline editing
	IPC_FINDFILE,  // FindFile command
	IPC_GETSIZES,  // GetSizes command
	IPC_PROTECT,   // Protect command

	IPC_SNAPSHOT,
	IPC_UNSNAPSHOT,

	IPC_ADD,  // FTPAdd - Add current lister to address book

	IPC_CONNECTVISUAL,	// Display the Connect requester
	IPC_OPTIONS,		// Display the current options

	IPC_CONFIGURE,	// Show Lister Format requester
	IPC_HURRYUP,	// Address book replies to this once it's all set up

	IPC_REMEMBERPATH,  // Get address book to store this path in the site's entry

	IPC_SELECTPATTERN,	// Select all matching files in the lister

	IPC_ALIVE,	// is lister conenction still valid or has timedout

};

// Old generic IPC Command flags
enum {
	CMD_HAS_ARGS = 1 << 0,
	CMD_QUIET = 1 << 1,
};

// MakeDir flags
// These start at 2 to avoid clashing with the old flags above
enum { MKDIR_SELECT = 1 << 2, MKDIR_NEW = 1 << 3, MKDIR_READ = 1 << 4 };

// Generic FTP message with ARexx forwarding
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct ftp_msg
{
	APTR fm_function_handle;   // Useful for callback functions
	struct RexxMsg *fm_rxmsg;  // Last function to use replies - makes ARexx usable
	char *fm_names;			   // Many functions need one or more names

	ULONG fm_flags;
	int fm_filecount;
	int fm_dircount;
	struct entry_info *fm_entries;
};
#ifndef __amigaos3__
	#pragma pack()
#endif
// Delete flags (Don't use bits 1-3)
#define DELE_SPECIAL (1 << 4)	 // Caused by 'FTPDelete' command - has special qualities
#define DELE_OPT_NAME (1 << 5)	 // NAME option specified
#define DELE_OPT_QUIET (1 << 6)	 // QUIET option specified

// Options flags (Don't use bits 1-3)
#define OPTIONS_OPT_DEFAULT (1 << 4)  // DEFAULT option of FTPOptions

// Quit message
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct quit_msg
{
	struct RexxMsg *qm_rxmsg;  // ARexx message to reply when done quitting
	char *qm_command;		   // Command to execute after quitting
};

// Addressbook message
struct addrbook_msg
{
	APTR am_function_handle;
	char am_opus[PORTNAMELEN + 1];	// Instance of Opus to call
};

// Site message - used for FTPConnect, FTPAdd, FTPOptions
struct connect_msg
{
	APTR cm_function_handle;		// Useful for callback functions
	char cm_opus[PORTNAMELEN + 1];	// Port name of this copy of Opus (DOPUS.1, DOPUS.2, ...)
	struct Window *cm_window;		// Window to open requester over
	IPTR cm_handle;				// Lister handle if called with one
	int cm_protocol;				// FTP_PROTOCOL_*
	struct site_entry cm_site;
};
#ifndef __amigaos3__
	#pragma pack()
#endif

#define CONN_OPT_GUI (1 << 4)	  // Show GUI even if other details are complete
#define CONN_OPT_SITE (1 << 5)	  // Find site in Address book
#define CONN_OPT_PATH (1 << 6)	  // Path (can override SITE or URL)
#define CONN_OPT_NOSCAN (1 << 7)  // Don't scan the directory on connection
#define CONN_OPT_RECON (1 << 8)	  // Reconnecting (private)
#define CONN_OPT_TLS_MODE (1 << 9)	  // TLS mode was specified on FTPConnect
#define CONN_OPT_TLS_VERIFY (1 << 10)  // TLS verification was specified on FTPConnect
#define CONN_OPT_PROTOCOL (1 << 11)	  // Protocol was specified on FTPConnect

// FTPCommand flags
#define CMD_OPT_QUIET (1 << 4)	// Don't show a result requester
#define CMD_OPT_LONG (1 << 5)	// Show long result

// FindFile message
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct findfile_msg
{
	struct RexxMsg *fm_rxmsg;
	char *fm_names;
	ULONG fm_flags;
};

// GetSizes message
struct getsizes_msg
{
	struct RexxMsg *gs_rxmsg;
	char *gs_names;	 // The entries whose sizes to get
	ULONG gs_flags;
};

// GetSizes flags
#define GS_OPT_FORCE (1 << 4)  // FORCE option specified

// Protect message
struct protect_msg
{
	struct RexxMsg *pm_rxmsg;
	char *pm_names;	 // The entries to protect
	ULONG pm_flags;
	ULONG pm_set_mask;
	ULONG pm_clear_mask;
};

// Protect flags
#define PROT_OPT_NAME (1 << 4)	   // NAME option specified
#define PROT_OPT_RECURSE (1 << 5)  // RECURSE option specified
#define PROT_OPT_SET (1 << 6)	   // SET option specified
#define PROT_OPT_CLEAR (1 << 7)	   // CLEAR option specified

// Transfer message (now between directory and FTP server or FTP and FTP)
struct xfer_msg
{
	struct RexxMsg *xm_rxmsg;
	ULONG xm_otherhandle;  // The handle of the other lister
	ULONG xm_flags;		   // See below
	char *xm_names;		   // The files to transfer (may be NULL)
	char *xm_srcpath;	   // Whether or not one is specified with the 'TO' switch
	char *xm_dstpath;
	char *xm_newname;			  // If one is specified with the 'NEWNAME' switch
	struct ftp_node *xm_rm_src;	  // Source FTP server
	struct ftp_node *xm_rm_dest;  // Destination FTP server
	char *xm_rm_file;			  // For ftp-ftp transfer
	struct SignalSemaphore xm_rm_semaphore;
};
#ifndef __amigaos3__
	#pragma pack()
#endif

// Transfer flags (not all used yet)
#define XFER_AS (1 << 4)						  // CopyAs requires name change
#define XFER_MOVE (1 << 5)						  // Move requires deleting original
#define XFER_SPECIAL (1 << 6)					  // Caused by 'FTPCopy' command - has special qualities
#define XFER_OPT_NAME (1 << 7)					  // NAME option specified
#define XFER_OPT_TO (1 << 8)					  // TO option specified
#define XFER_OPT_QUIET (1 << 9)					  // QUIET option specified
#define XFER_OPT_UPDATE (1 << 10)				  // UPDATE option specified
#define XFER_OPT_MOVEWHENSAME (1 << 11)			  // MOVEWHENSAME option specified
#define XFER_OPT_NEWER (1 << 12)				  // NEWER option specified
#define XFER_OPT_NEWNAME (1 << 13)				  // NEWNAME option specified
#define XFER_DROP (1 << 14)						  // Caused by trapped 'drop'
#define XFER_DROP_FROM_DESKTOP (1 << 15)		  // 'drop' source was desktop (some names may be appicon labels)
#define XFER_DROPFROM (1 << 16)					  // Caused by trapped 'dropfrom'
#define XFER_DROPFROM_POPUP_DISABLED (1 << 17)	  // Drop from FTP to desktop, popup disabled
#define XFER_DROPFROM_POPUP_NO_DEFAULT (1 << 18)  // Drop from FTP to desktop, no default action
#define XFER_DROPFROM_CREATE_LEFTOUT (1 << 19)	  // Drop from FTP to desktop, create leftout
#define XFER_DROPFROM_DESKTOP_MOVE (1 << 20)	  // Drop from FTP to desktop, move to desktop
#define XFER_DROPFROM_DESKTOP_COPY (1 << 21)	  // Drop from FTP to desktop, copy to desktop
#define XFER_DROPFROM_TO_ICON (1 << 22)			  // Drop from FTP to disk/drawer icon on desktop
#define XFER_SUBDIR (1 << 23)					  // Drop onto a subdirectory

// Trapped Opus command that uses a temporary file
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct traptemp_msg
{
	struct RexxMsg *tm_rxmsg;
	char tm_command[TRAPTEMPCMDLEN + 1];  // The Opus command to send
	char *tm_names;						  // The files to act on
};

// One lister doing a favour for another
struct favour_msg
{
	int fm_ftp_command;				// Which FTP command is needed?
	struct _endpoint *fm_endpoint;	// Endpoint of the lister being asked
	void *fm_arg1;					// Argument to command (which entry to act on)
	void *fm_arg2;
};
#ifndef __amigaos3__
	#pragma pack()
#endif
// Favours one FTP lister can ask another
enum {
	FAVOUR_LIST = 1,
	FAVOUR_CWD,
	FAVOUR_CDUP,
	FAVOUR_MKD,
	FAVOUR_DELE,
	FAVOUR_RMD,

	FAVOUR_PORT,
	FAVOUR_PASV,
	FAVOUR_REST,
	FAVOUR_RETR,
	FAVOUR_STOR,
	FAVOUR_ABOR,
	FAVOUR_CHMOD,
	FAVOUR_SELECT,
	FAVOUR_GETREPLY,
	FAVOUR_GETENTRY,
	FAVOUR_ERRORREQ,
	FAVOUR_GET_FILE,
	FAVOUR_PUT_FILE,
	FAVOUR_ENDLIST,
};

// Edit message
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct edit_msg
{
	struct RexxMsg *em_rxmsg;
	char *em_field;
	char *em_entry;
	char *em_value;
};

// Rename message (not yet used)
struct rename_msg
{
	char *rm_names;
	char *rm_newname;
};
#ifndef __amigaos3__
	#pragma pack()
#endif
/*
 *	For the IPC_SET command, one of these values goes in the flags field
 *	 The new value of the variable goes in the data field
 */

enum {
	SET_NONE,  //	No variable specified - use requester
	SET_LOGFILE,
	SET_DEBUG,
	SET_TIMEOUT,
	SET_LISTCMD,  //	Change command used for listing
	SET_QUIET,
};

#endif
