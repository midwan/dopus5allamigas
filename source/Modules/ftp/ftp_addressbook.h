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

#ifndef _addressbook_h
#define _addressbook_h

/*
 *	09-04-96	Changed CONFIGFILE from "DOpus5:FTP/ftp.config" to "DOpus5:System/ftp.config"
 */

#include "ftp_opusftp.h"
#include "ftp_addressbook_protos.h"

extern char *CONFIGFILE, *FTP_OPTIONS_NAME, *FTP_ADDR_NAME;

/* OLD
struct phentry
{
struct Node node;
BOOL   anon;
char   prefix[2];
char   name[HOSTNAMELEN+1];
char   host[HOSTNAMELEN+1];
char   user[USERNAMELEN+1];
char   pass[PASSWORDLEN+1];
char   path[PATHLEN+1];
LONG   port;
};
*/

#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct short_site_entry
{
	BOOL se_anon;
	char se_name[HOSTNAMELEN + 1];
	char se_host[HOSTNAMELEN + 1];
	char se_user[USERNAMELEN + 1];
	char se_pass[PASSWORDLEN + 1];
	char se_path[PATHLEN + 1];
	LONG se_port;
	BOOL se_has_custom_env;
};

// MUST BE kept as a partial strure compatible with site_entry

struct long_site_entry
{
	BOOL se_anon;
	char se_name[HOSTNAMELEN + 1];
	char se_host[HOSTNAMELEN + 1];
	char se_user[USERNAMELEN + 1];
	char se_pass[PASSWORDLEN + 1];
	char se_path[PATHLEN + 1];
	LONG se_port;
	BOOL se_has_custom_env;
	struct ftp_environment se_local_env;
};
#ifndef __amigaos3__
	#pragma pack()
#endif

#define TMPBUFLEN 512

#define CHANGED_OPT (1 << 0)
#define CHANGED_ADR (1 << 1)

// types of windows

#define WT_ADR (1 << 0)
#define WT_EDIT (1 << 1)
#define WT_NEW (1 << 2)
#define WT_OPT (1 << 3)
#define WT_DEFOPT (1 << 4)
#define WT_CON (1 << 5)
#define WT_ADD (1 << 6)
#define WT_ADRLIST (1 << 7)

// wt_type defines as MSB parenttype LSB type

#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct display_globals
{
	struct opusftp_globals *dg_og;
	IPCData *dg_ipc;

	WORD dg_selected;  // current highlighted item in ftp list
	WORD dg_config_changed;

	struct ftp_config dg_oc;  // local copy of config

	struct ftp_config dg_default_oc;  // copy of absolute system defaults
									  // for checking

	BOOL dg_log_open;  // current log state from opus globals

	char dg_anonpass[PASSWORDLEN + 1];

	char dg_opusport[PORTNAMELEN + 1];	// Port name of this copy of Opus (DOPUS.1, DOPUS.2, ...)

	Att_List *dg_wp_list;  // lists of all wps

	struct MsgPort *idcmp_port;	 // global idcmp port for windows

	ULONG win_sig;	// signal bit for idcmp
	struct window_params *dg_defoptions;

	char dg_options_name[256];
	char dg_options_last_saved_name[256];
	char dg_addressbook_name[256];
	char dg_addressbook_last_saved_name[256];

	WORD dg_options_change;
	WORD dg_site_change;
	WORD dg_edit_count;	 // number of sites being edited

	TimerHandle *dg_timer;

	ULONG dg_old_winflags;	// Saved flags
	ULONG dg_old_idcmp;		// Saved IDCMP
	int dg_tick_count;		// For lock-up prevention
	int dg_last_tick;		// For lock-up prevention

	DragInfo *dg_draginfo;	// DragInfo
	int dg_drag_x;			// X offset (for dragging etc)
	int dg_drag_y;			// Y offset
	int dg_drag_item;		// List item dragged

	Att_List *dg_wp_hidden;	 // lists of all wps

	// ptrs to addressbook window stuff
	struct window_params *dg_addrwp;
};

struct window_params
{
	Att_Node *wp_node;					   // ptr to node in dg_wp_list
	struct window_params *wp_parentwp;	   // parent window_params
	struct display_globals *wp_dg;		   // back pointer to global
	Att_List *wp_children;				   // list this windows children
	struct Window *wp_win;				   // window for this call
	ObjectList *wp_objlist;				   // objects attached to this window
	ObjectList *wp_sub_objlist;			   // rhs objects
	Att_List *wp_options_list;			   // list attached to sub_objlist
	SubOptionHandle *wp_SubOptionsHandle;  // ptr to array added as sub options

	// the following is used a reference to entry being edited
	struct site_entry *wp_se_real;	// ptr to orig entry IF ANY

	// the following is allocated copy of entry being edited
	struct site_entry *wp_se_copy;	// our local copy for comparison

	WORD wp_option;			// current selected option
	WORD wp_itemnumber;		// sub option seleted
	Att_Node *wp_sitenode;	// node iof site being edited

	WORD wp_type;  // type of window

	IPCMessage *wp_imsg;  // ptr to Calling IPC message
						  // so can reply to ADR/CON/OPT msgs after closing win

	IPCData *wp_extask_ipc;	 // ipc of any spawned format requesters
							 // so we can kill it if we need to

	struct SignalSemaphore wp_extask_semaphore;	 // set for external tasks attached to any config window
												 // to allow orderly aborts of main process
};
#ifndef __amigaos3__
	#pragma pack()
#endif

#define ID_FTPSITE_SHORT MAKE_ID('F', 'A', 'D', '1')
#define ID_FTPSITE_LONG MAKE_ID('F', 'A', 'D', '2')
#define ID_FTPSITE_PROTOCOL MAKE_ID('F', 'A', 'D', 'P')
#define ID_OPTIONS MAKE_ID('F', 'O', 'P', 'T')
#define ID_ENV MAKE_ID('F', 'E', 'N', 'V')

#define FINDWP(win) ((struct window_params *)(DATA(win)->userdata))

/**********************************************************/
//
//	Stuff copied from main Opus config include files.
//	We can't include them ourselved becuase of loads
//	of conflicts and stuff.
/**********************************************************/
#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct Window *window;

	DragInfo *drag;
	Att_Node *drag_node;
	short drag_x;
	short drag_y;

	long tick_count;
	long last_tick;
	TimerHandle *timer;

	unsigned long old_flags;
	unsigned long old_idcmp;

	short lock_count;
	short pad;
} CfgDragInfo;

typedef struct _FunctionEntry
{
	Att_Node *node;	   // Points back to node
	int type;		   // Entry type
	char buffer[256];  // Entry buffer
} FunctionEntry;
#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
