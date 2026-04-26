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

/*
 *	27-08-95	Increased password length from 32 to 64 characters
 *	22-09-95	Increased filename length to 256 for Unix
 *	22-09-95	Increased password length to be equal to the address length
 *	20-03-96	Added semaphore to update_info so it can be used asynchronously
 *	07-06-96	Changed FTPQuit template from NOW/S to FORCE/S
 *
 *	08-07-97	added template_old for backward compatibility for systems
 *			which did not specify the PORT in template
 *
 */

#ifndef _OPUSFTP_H
#define _OPUSFTP_H

#include <proto/dopus5.h>
#include <proto/module.h>

// Not actually defined in dos.h
#define FIBF_HIDDEN (1 << 7)

#define PORTNAME "_OPUSFTP_"

#define USERNAMELEN 32
#define FILENAMELEN 256								// Unix allows greater than Amiga's 30
#define AMIFILENAMELEN 29							// 30 including null terminator
#ifdef __amigaos4__
#define FIBFILENAMELEN	255							// 256 including null terminator
#else
#define FIBFILENAMELEN	107							// 108 including null terminator
#endif
#define COMMENTLEN 79								// 80 including null terminator
#define HANDLELEN 12								// lister handle
#define HOSTNAMELEN 96								// eg, 'planet.earth'
#define LOGNAMELEN 256								// for logfile CON:...
#define ADDRESSLEN (USERNAMELEN + 1 + HOSTNAMELEN)	// full internet address eg. 'foo@planet.earth'

#define LSCMDLEN 20				// LIST or NLST command + arguments
#define PASSWORDLEN ADDRESSLEN	// anonymous password is your address
#define PATHLEN 256
#define PORTNAMELEN 10						  // Opus Arexx port name eg. 'DOPUS.1'
#define SCREENNAMELEN (MAXPUBSCREENNAME + 1)  // Opus public screen name eg. 'Workbench'
#define TRAPTEMPCMDLEN 16					  // Trappable Opus command eg 'smartread'

#define CONFIG_TEMPLATE \
	"LIBRARY=LIB/K,LOG/K,LOGOFF/S,\
DEBUG/K/N,TIMEOUT/K/N,LISTUPDATE/K/N,ANONPASS/K,\
SHOWINDEX/S,INDEXSIZE/K/N,AUTOINDEX/S,NOOP/S,\
CONNECTRETRY/S,CONNECTTRIES/K/N,CONNECTRETRYTIMEOUT/K/N,\
NODIRPROGRESS/S"

enum  // offsets for parse template
{ C_OPT_LIB,
  C_OPT_LOG,
  C_OPT_LOGOFF,
  C_OPT_DBUG,
  C_OPT_TIME,
  C_OPT_UPDT,
  C_OPT_ANONPASS,
  C_OPT_SHOWINDEX,
  C_OPT_INDEXSIZE,
  C_OPT_AUTOINDEX,
  C_OPT_NOOP,
  C_OPT_CONNECT_RETRY,
  C_OPT_CONNECT_TRIES,
  C_OPT_CONNECT_RETRY_TIMEOUT,
  C_OPT_DIRSCAN_PROGRESS,
};

#define ADDR_TEMPLATE \
	"ANONYMOUS=ANON/S,USERACCOUNT=ACCT/S,ALIAS=NAME/K,HOST,ADDRESS=ADDR/K,PORT/N,PATH=DIR,USER/K,PASSWORD=PASS/K"

enum  // offsets for parse template
{ A_OPT_ANON,
  A_OPT_ACCT,
  A_OPT_ALIS,
  A_OPT_HOST,
  A_OPT_ADDR,
  A_OPT_PORT,
  A_OPT_PATH,
  A_OPT_USER,
  A_OPT_PASS,
};

#define CONNECT_TEMPLATE "HOST,PORT/N/K,USER,PASSWORD=PASS,DIR/K,LISTER/N/K,SITE/K,GUI/S,NOSCAN/S,RECON/S"
#define ADDRBOOK_TEMPLATE 0
#if 0
	#define CD_TEMPLATE "DIR/F"
	#define COPY_TEMPLATE 0
	#define RENAME_TEMPLATE 0
	#define DELETE_TEMPLATE "FILE/F"
#endif
#define COMMAND_TEMPLATE "QUIET/S,LONG/S,COMMAND/F"
#define SETVAR_TEMPLATE "VAR,VALUE/F"
#define QUIT_TEMPLATE "FORCE/S"
#define ADD_TEMPLATE 0
#define OPTIONS_TEMPLATE "DEFAULT/S"

enum {
	D_OPT_HOST,
	D_OPT_PORT,
	D_OPT_USER,
	D_OPT_PASS,
	D_OPT_PATH,
	D_OPT_HANDLE,
	D_OPT_SITE,
	D_OPT_GUI,
	D_OPT_NOSCAN,
	D_OPT_RECON,
};

// Module function IDs
enum {
	MID_ADDRESSBOOK,  // <no template>
	MID_CONNECT,	  // HOST,PORT,USER,PASS,DIR/K,LISTER/K/N
	MID_COMMAND,	  // COMMAND/F	FTPCommand SYST HELP
	MID_SETVAR,		  // VAR,VALUE/F	FTPSetVar LIST NLST -al
	MID_OPTIONS,
	MID_ADD,   // <no template>
	MID_QUIT,  // FORCE/S
};

// msgs flags sent from addressbook to main for changes to logfile

#define ID_LOGNAME_F (1 << 0)
#define ID_LOGON_F (1 << 1)

// cycle gadget values for copy_type
enum {
	COPY_TYPE_DEFAULT,
	COPY_TYPE_UPDATE,
	COPY_TYPE_NEWER,
};

// cycle gadget values for replace
enum {
	COPY_REPLACE_ALWAYS,
	COPY_REPLACE_NEVER,
	COPY_REPLACE_ASK,
};

#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct ftp_environment
{
	int e_retry_count;	// Number of times to retry a connection
	int e_retry_delay;	// Number of seconds to wait before retrying
	int e_list_update;	// How often is lister updated during LIST
	int e_timeout;
	int e_script_time;
	int e_indexsize;

	char e_toolbar[256];

#ifdef __AROS__
	union
	{
		struct
		{
#endif
			unsigned int e_custom_options : 1;

			unsigned int e_retry : 1;  // Retry failed attempt to connect?
			unsigned int e_retry_lost : 1;

			unsigned int e_noops : 1;  // Send noops

			unsigned int e_passive : 1;

			unsigned int e_keep_last_dir : 1;
			unsigned int e_safe_links : 1;
			unsigned int e_unk_links_file : 1;

			unsigned int e_show_startup : 1;
			unsigned int e_show_dir : 1;
			unsigned int e_progress_window : 1;	 // Display progress bars while scanning dirs

			unsigned int e_tls_mode : 2;  // FTP_TLS_MODE_*

			unsigned int e_script_connect_ok : 1;
			unsigned int e_script_connect_fail : 1;
			unsigned int e_script_copy_ok : 1;
			unsigned int e_script_copy_fail : 1;
			unsigned int e_script_error : 1;
			unsigned int e_script_close : 1;

			unsigned int e_index_enable : 1;
			unsigned int e_index_auto : 1;	// If exceeds the limit then skip

			unsigned int e_custom_format : 1;

			unsigned int e_copy_type : 2;  // default/update/newer
			unsigned int e_copy_set_archive : 1;
			unsigned int e_copy_replace : 2;

			unsigned int e_copy_opus_default : 1;
			unsigned int e_copy_datestamp : 1;
			unsigned int e_copy_protection : 1;
			unsigned int e_copy_comment : 1;

			unsigned int e_url_comment : 1;

			unsigned int e_transfer_details : 1;  //(-ve flag 0=TRUE)
			unsigned int e_rescan : 1;
			unsigned int e_recursive_special : 1;  //(recursive copy names with space)
			unsigned int e_special_dir : 1;		   // old fred hack env var
			unsigned int e_tls_verify_peer : 1;	   // Verify FTPS certificates
			unsigned int pad8 : 1;
#ifdef __AROS__
		};
		struct
		{
			unsigned int bitfield1;
			unsigned int bitfield2;
		};
	};
#endif

	// IMPORTANT for memcmp comparisions of different structures
	// the ListFormat MUST BE LAST IN STRUCTURE

	ListFormat e_listformat;
};

struct ftp_config
{
	char oc_logname[LOGNAMELEN + 1]; /* Buffer for name of log file */
	char oc_anonpass[PASSWORDLEN + 1];

#ifdef __AROS__
	union
	{
		struct
		{
#endif
			unsigned int pad1 : 1;				//
			unsigned int oc_enable_log : 1;		// user wants log open on start
			unsigned int oc_log_debug : 1;		// User level debugging (print FTP commands)
			unsigned int oc_user_password : 1;	// user has defined user password

			unsigned int oc_addr_auto : 1;	// auto close addressbook on connect
			unsigned int oc_addr_dc : 2;	// what to do on double click
			unsigned int pad8 : 1;
#ifdef __AROS__
		};
		unsigned int bitfield1;
	};
#endif

	// IMPORTANT for memcmp comparisions of different structures
	// the environment MUST BE LAST IN STRUCTURE
	struct ftp_environment oc_env;	// default global environment
};
#ifndef __amigaos3__
	#pragma pack()
#endif

/****************************************************************************
 *
 *	NOTE: COPY OF SITE ENTRY STRUCTURES
 *
 *	you cannot memcpy site_entry structures because of internal ptr
 *
 *	ALWAYS Use fn copy_site_entry(og,new,old)
 *
 *****************************************************************************/
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct site_entry
{
	BOOL se_anon;
	char se_name[HOSTNAMELEN + 1];
	char se_host[HOSTNAMELEN + 1];
	char se_user[USERNAMELEN + 1];
	char se_pass[PASSWORDLEN + 1];
	char se_path[PATHLEN + 1];
	LONG se_port;
	BOOL se_has_custom_env;					// Only Greg may touch
	struct ftp_environment se_env_private;	// Only Greg may touch
	struct ftp_environment *se_env;			// back -> local or -> global env

	/***************************************************************/
	/********* ^^ do not change size or order above ^^ 	********/
	/********* 	see ftp_addressbook.h			********/
	/***************************************************************/

	UWORD se_active_gadget;	 // contains 0 or GADGET ID for
							 // gadget to be activated on gui
							 // from ftp_opuftp.h GAD_CONNECT...

	UBYTE se_preserve_format;  // do not call get_site_format when copying
							   // site_entry so preserve se_listformat when
							   // pasing structure

	UBYTE se_pad;

	ListFormat se_listformat;  // listerformat to be used as tmp
							   // by a lister during connection
};

/**
 **	Global variables
 **/

// 28.1.97 GP og_opus_ipc NOW IS the REAL IPC for the main Opus process

struct opusftp_globals
{
	// Global for all Opuses
	IPCData *og_opus_ipc;  // IPC of Opus itself
	IPCData *og_main_ipc;  // IPC of main FTP process
	int og_ftp_launched;   // flag to stop duplicate running of module
	int og_noreq;		   // TRUE to suppress ALL listers

	struct ListLock og_listerlist;	// List of connected FTP listers
	int og_listercount;				// Number of connected FTP listers

	IPCData *og_addrproc;
	BOOL og_addrbook_open;	// So we don't have to queue address book messages

	// Unsure - Global or One per Opus?
	EXT_FUNC(og_func_callback);	 // Function callback

	// Should be seperate for each lister, addressbook (multiple Opuses)
	char og_opusname[PORTNAMELEN + 1];
	struct Screen *og_screen;

	DOpusCallbackInfo og_hooks;	 // gp

	struct ftp_config og_oc;

	Att_List *og_SiteList;	// globally accesable addrress book sitelist
	struct SignalSemaphore og_SiteList_semaphore;
	int og_log_open;  // log file is open

	ListFormat og_opus_format;	// holder for current opus default lister format

	struct site_entry og_last_site;	 // last site for connect msg
	BOOL og_valid_site;
};

/*
 *	This structure contains any per-process global variables
 *	ie. variables which much not be accessed by two multitasking processes
 *	at the same time
 */

struct globals
{
	struct Library *g_socketbase; /* Socket library must be opened by each calling process */
#ifdef __amigaos4__
	struct SocketIFace *g_isocket;
#endif
	int g_errno;		   /* Socket library puts its errors here */
	IPCData *g_master_ipc; /* Master process's IPC */
};
#ifndef __amigaos3__
	#pragma pack()
#endif

/*
 * the socket libraries need to be opened by each process that uses them
 * SocketBase is stored in each process's userdata field
 */

#define GETGLOBAL(t, v) ((struct globals *)((IPCData *)(t->tc_UserData))->userdata)->v

#define SocketBase GETGLOBAL(FindTask(NULL), g_socketbase)
#define ISocket GETGLOBAL(FindTask(NULL), g_isocket)
#undef errno
#define errno GETGLOBAL(FindTask(NULL), g_errno)

/******** stuff for gui adressbook */

extern ConfigWindow ftp_main_window, ftp_edit_window, ftp_options_window;
extern ObjectDef ftp_main_objects[], ftp_edit_objects[], ftp_options_objects[];

enum {
	GAD_BASE,

	GAD_FTP_MAIN_LAYOUT,
	GAD_FTP_SITES,

	GAD_FTP_NEW,
	GAD_FTP_EDIT,
	GAD_FTP_DELETE,
	GAD_FTP_SORT,
	GAD_FTP_SITE_LAYOUT,
	GAD_FTP_SITE_NAME,
	GAD_FTP_HOST_NAME,
	GAD_FTP_PORT,
	GAD_FTP_ANON,
	GAD_FTP_CUSTOM_LAYOUT,
	GAD_FTP_CUSTOM_OPTIONS,
	GAD_FTP_SET_CUSTOM_OPTIONS,
	GAD_FTP_SAVE,
	GAD_FTP_CONNECT,
	GAD_FTP_SET_DEFAULT_OPTIONS,
	GAD_FTP_CANCEL,

	GAD_ENV_ITEM_LISTER,
	GAD_ENV_EDIT_AREA,
	GAD_ENV_SAVE,
	GAD_ENV_RESET_DEFAULT,
	GAD_ENV_CANCEL,

	GAD_ENV_START_UPDATE,  // start marker for gadgets reqd to updating the gui dissplay
						   // gadgets between here ane end automatically call update
						   // fn to check status of default/reset etc

	GAD_ENV_ENABLE_RETRY,
	GAD_ENV_RETRY_COUNT,
	GAD_ENV_RETRY_DELAY,
	GAD_ENV_ENABLE_RETRY_LOST,
	GAD_ENV_NOOPS,
	GAD_ENV_TLS_MODE,
	GAD_ENV_TLS_VERIFY,
	GAD_ENV_PASSIVE,

	GAD_ENV_TIMEOUT,
	GAD_ENV_LIST_UPDATE,

	GAD_ENV_KEEP_LAST_DIR,
	GAD_ENV_SPECIAL_DIR,
	GAD_ENV_SAFE_LINKS,
	GAD_ENV_UNK_LINKS,

	GAD_ENV_SHOW_STARTUP_MSG,
	GAD_ENV_SHOW_DIR_MSG,
	GAD_ENV_PROGRESS_WINDOW,
	GAD_ENV_TRANSFER_DETAILS,

	GAD_ENV_SCRIPT_CONNECT_SUCCESS,
	GAD_ENV_SCRIPT_CONNECT_FAIL,
	GAD_ENV_SCRIPT_COPY_SUCCESS,
	GAD_ENV_SCRIPT_COPY_FAIL,
	GAD_ENV_SCRIPT_ERROR,
	GAD_ENV_SCRIPT_CLOSE,
	GAD_ENV_SCRIPT_TIME,

	GAD_ENV_INDEX_ENABLE,

	GAD_ENV_AUTO_DOWNLOAD,
	GAD_ENV_AUTO_DOWNLOAD_SIZE,
	GAD_ENV_LOG_ENABLE,
	GAD_ENV_DEBUG,
	GAD_ENV_LOG_FILE,
	GAD_ENV_ANON_PASSWORD,

	GAD_ENV_TOOLBAR_GLASS,
	GAD_ENV_TOOLBAR,

	GAD_ENV_CUST_FORMAT,
	GAD_ENV_SET_FORMAT,

	GAD_ENV_COPY_TYPE,

	GAD_ENV_COPY_SET_ARCHIVE,
	GAD_ENV_COPY_URL_COMMENT,
	GAD_ENV_COPY_RESCAN,
	GAD_ENV_COPY_RECURSIVE_SPECIAL,

	GAD_ENV_REPLACE,
	GAD_ENV_COPY_OPUS_DEFAULT,
	GAD_ENV_COPY_DATESTAMP,
	GAD_ENV_COPY_PROTECTION,
	GAD_ENV_COPY_COMMENT,
	GAD_ENV_ADDR_DC,	// addressbook double click
	GAD_ENV_ADDR_AUTO,	// close after DC or connect

	GAD_ENV_END_UPDATE,	 // end marker for gadgets reqd to updating the gui dissplay

	GAD_CONNECT_LAYOUT,
	GAD_CONNECT_GLASS,
	GAD_CONNECT_NAME,
	GAD_CONNECT_HOST,
	GAD_CONNECT_ANON,
	GAD_CONNECT_PORT,
	GAD_CONNECT_USER,
	GAD_CONNECT_PASSWORD,
	GAD_CONNECT_DIR,
	GAD_CONNECT_CONNECT,
	GAD_CONNECT_LAST,
	GAD_CONNECT_CANCEL,

	GAD_EDIT_NAME,
	GAD_EDIT_HOST,
	GAD_EDIT_ANON,
	GAD_EDIT_PORT,
	GAD_EDIT_USER,
	GAD_EDIT_PASSWORD,
	GAD_EDIT_DIR,
	GAD_EDIT_OK,
	GAD_EDIT_OPTIONS,
	GAD_EDIT_CANCEL,
	GAD_EDIT_CUSTOM_LAYOUT,
	GAD_EDIT_CUSTOM_OPTIONS,
	GAD_EDIT_SET_CUSTOM_OPTIONS,

	MENU_ENV_OPEN,
	MENU_ENV_SAVE,
	MENU_ENV_SAVEAS,
	MENU_ENV_CANCEL,
	MENU_ENV_RESET_DEFAULTS,
	MENU_ENV_LAST_SAVED,
	MENU_ENV_RESTORE,

	MENU_FTP_OPEN,
	MENU_FTP_IMPORT,
	MENU_FTP_IMPORT_AMFTP,
	MENU_FTP_SAVE,
	MENU_FTP_SAVEAS,
	MENU_FTP_EXPORT,
	MENU_FTP_CANCEL,

	GAD_LIST_SITES,
	GAD_LIST_OK,
	GAD_LIST_CANCEL,

};

enum {
	// for main lister titles
	ENV_SUB_COPYFLAGS,
	ENV_SUB_COPYATTR,
	ENV_SUB_LISTER,
	ENV_SUB_DISPLAY,
	ENV_SUB_MISC,
	ENV_SUB_SCRIPTS,
	ENV_SUB_INDEX,
	ENV_SUB_CONNECTION,
	ENV_SUB_GLOBAL,
};

/********************************/

//
//	Some structures copied from the Opus includes...
//

// Defines information about a GUI element
#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	BOOL visible;  // Indicates visibility

	struct IBox box;		// Dimensions in an IBox
	struct Rectangle rect;	// Dimensions in a Rectangle

	UWORD fg_pen;			// Foreground pen to use
	UWORD bg_pen;			// Background pen to use
	struct TextFont *font;	// Font to use

	struct RastPort rast;  // RastPort copy
} GUI_Element;
#ifndef __amigaos3__
	#pragma pack()
#endif

#include <Program/reselect.h>
#include <Program/pattern.h>
#include <Program/toolbar.h>
#include <Program/lister.h>
#include <Program/dirlist.h>

/********************************/

// Config sub-option handles
#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct _SubOptionHandle
{
	int num;			 // Option number
	ULONG name;			 // Option name ID
	ObjectDef *objects;	 // Object list
} SubOptionHandle;
#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
