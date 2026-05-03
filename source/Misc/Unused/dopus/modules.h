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

#ifndef _DOPUS_MODULES
#define _DOPUS_MODULES

/****************************************************************************

 Support file for DOpus modules

 ****************************************************************************/

#ifndef _DOPUSBASE
	#include <dopus/dopusbase.h>
#endif

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// Defines a function in a module
typedef struct
{
	ULONG id;		 // Function ID code
	char *name;		 // Function name
	ULONG desc;		 // Locale string ID for function description
	ULONG flags;	 // Function flags
	char *template;	 // Command template
} ModuleFunction;

// Function flags
#define FUNCF_NEED_SOURCE (1 << 0)	// Needs a source directory
#define FUNCF_NEED_DEST (1 << 1)	// Needs a destination directory
#define FUNCF_NEED_FILES (1 << 2)	// Needs some files to work with
#define FUNCF_NEED_DIRS (1 << 3)	// Needs some files to work with
#define FUNCF_NEED_ENTRIES (FUNCF_NEED_FILES | FUNCF_NEED_DIRS)
#define FUNCF_CAN_DO_ICONS (1 << 6)	  // Function can do icons
#define FUNCF_SINGLE_SOURCE (1 << 8)  // Only a single source needed
#define FUNCF_SINGLE_DEST (1 << 9)	  // Only a single destination needed
#define FUNCF_WANT_DEST (1 << 11)	  // Want destinations, don't need them
#define FUNCF_WANT_SOURCE (1 << 12)	  // Want source, don't need it
#define FUNCF_WANT_ENTRIES (1 << 19)  // Want entries
#define FUNCF_PRIVATE (1 << 26)		  // Function is private

// Defines all the functions in a module
typedef struct
{
	ULONG ver;					 // Module version
	char *name;					 // Module name
	char *locale_name;			 // Catalog name
	ULONG flags;				 // Module flags
	ULONG function_count;		 // Number of functions in module
	ModuleFunction function[1];	 // Definition of first function
} ModuleInfo;

/*** If the module has more than one function, the additional ModuleFunction
	 structures MUST follow the ModuleInfo structure in memory. Eg,

	 ModuleInfo module_info={....};
	 ModuleFunction more_funcs[2]={{...},{...}};                           ***/

// Flags for ModuleInfo
#define MODULEF_CALL_STARTUP (1 << 0)	  // Call ModuleEntry() on startup
#define MODULEF_STARTUP_SYNC (1 << 1)	  // Run Synchronously on startup
#define MODULEF_CATALOG_VERSION (1 << 2)  // Use version for catalog
#define MODULEF_HELP_AVAILABLE (1 << 3)	  // Command help is available

// ID passed to Module_Entry() if module is run on startup
#define FUNCID_STARTUP 0xffffffff

// Callback commands
#define EXTCMD_GET_SOURCE 0		   // Get current source path
#define EXTCMD_NEXT_SOURCE 1	   // Get next source path
#define EXTCMD_UNLOCK_SOURCE 2	   // Unlock source paths
#define EXTCMD_GET_ENTRY 3		   // Get entry
#define EXTCMD_END_ENTRY 4		   // End entry
#define EXTCMD_RELOAD_ENTRY 5	   // Reload entry
#define EXTCMD_CHECK_ABORT 9	   // Check abort status
#define EXTCMD_ENTRY_COUNT 10	   // Get entry count
#define EXTCMD_GET_WINDOW 11	   // Get window handle
#define EXTCMD_GET_DEST 12		   // Get next destination
#define EXTCMD_END_SOURCE 13	   // Cleanup current source path
#define EXTCMD_END_DEST 14		   // Cleanup current destination path
#define EXTCMD_ADD_FILE 16		   // Add a file to a lister
#define EXTCMD_GET_HELP 17		   // Get help on a topic
#define EXTCMD_GET_PORT 18		   // Get ARexx port name
#define EXTCMD_GET_SCREEN 19	   // Get public screen name
#define EXTCMD_REPLACE_REQ 20	   // Show exists/replace? requester
#define EXTCMD_REMOVE_ENTRY 21	   // Mark an entry for removal
#define EXTCMD_GET_SCREENDATA 22   // Get DOpus screen data
#define EXTCMD_FREE_SCREENDATA 23  // Free screen data
#define EXTCMD_SEND_COMMAND 30	   // Send a command to DOpus
#define EXTCMD_DEL_FILE 31		   // Delete a file from a lister
#define EXTCMD_DO_CHANGES 32	   // Perform changes
#define EXTCMD_LOAD_FILE 33		   // Load files to listers

// Structures used with callback commands
struct function_entry
{
	ULONG pad[2];
	char *name;	  // File name
	APTR entry;	  // Entry pointer (don't touch!)
	short type;	  // Type of file
	short flags;  // File flags
};

struct path_node
{
	ULONG pad[2];
	char buffer[512];  // Contains path string
	char *path;		   // Points to path string
	APTR lister;	   // Lister pointer
	ULONG flags;	   // Flags
};

/****************************************************************************

 Packets used to send commands

 ****************************************************************************/

// EXTCMD_END_ENTRY
struct endentry_packet
{
	struct function_entry *entry;  // Entry pointer
	BOOL deselect;				   // TRUE to deselect entry
};

// EXTCMD_ADD_FILE
struct addfile_packet
{
	char *path;					  // Path to add file to
	struct FileInfoBlock *fib;	  // FileInfoBlock to add
	struct ListerWindow *lister;  // Lister pointer
};

// EXTCMD_DEL_FILE
struct delfile_packet
{
	char *path;					  // Path to delete file from
	char *name;					  // Name of file to delete
	struct ListerWindow *lister;  // Lister pointer
};

// EXTCMD_LOAD_FILE
struct loadfile_packet
{
	char *path;	   // Path of file
	char *name;	   // File name
	short flags;   // Flags field
	short reload;  // TRUE to reload existing file
};

#define LFF_ICON (1 << 0)

// EXTCMD_REPLACE_REQ
struct replacereq_packet
{
	struct Window *window;		  // Window to open over
	struct Screen *screen;		  // Screen to open on
	IPCData *ipc;				  // Your process IPC pointer
	struct FileInfoBlock *file1;  // First file
	struct FileInfoBlock *file2;  // Second file
	short flags;				  // Flags
};

// Result code from EXTCMD_REPLACE_REQ
#define REPLACE_ABORT -1
#define REPLACE_LEAVE 0
#define REPLACE_REPLACE 1
#define REPLACEF_ALL (1 << 1)

// EXTCMD_GET_SCREENDATA
struct DOpusScreenData
{
	struct Screen *screen;		   // Screen pointer
	struct DrawInfo *draw_info;	   // DrawInfo pointer
	unsigned short depth;		   // Screen depth
	unsigned short pen_alloc;	   // Mask of allocated pens
	unsigned short pen_array[16];  // Pen array
	short pen_count;			   // Number of pens
};

// EXTCMD_SEND_COMMAND
struct command_packet
{
	char *command;	// Command to send
	ULONG flags;	// Command flags
	char *result;	// Will point to result string
	ULONG rc;		// Return code
};

#define COMMANDF_RESULT (1 << 0)

// Prototype for the callback hook
ULONG __asm __saveds function_external_hook(register __d0 ULONG command,
											register __a0 APTR handle,
											register __a1 APTR packet);

#define EXT_FUNC(name) unsigned long __asm (*name)(register __d0 ULONG, register __a0 APTR, register __a1 APTR)
#define TYPE_EXT(var) (unsigned long (*)()) var

#define IDCMP_FUNC(name) unsigned long __asm (*name)(register __d0 ULONG, register __a0 struct IntuiMessage *)

/**** Define this if you are using the standard modinit.o code ****/
#ifdef _DOPUS_MODULE_DEF

// These library bases are defined and made available automatically
extern struct Library *DOSBase;
extern struct Library *DOpusBase;
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;
extern struct Library *IconBase;
extern struct Library *UtilityBase;
extern struct Library *LayersBase;
extern struct Library *GadToolsBase;
extern struct Library *AslBase;
extern struct Library *DiskfontBase;
extern struct Library *RexxSysBase;
extern struct Library *TimerBase;
extern struct Library *LocaleBase;
extern struct Library *WorkbenchBase;

// Global pointer to a module's locale information
extern struct DOpusLocale *locale;
void init_locale_data(struct DOpusLocale *);

// You MUST define the ModuleInfo structure yourself
extern ModuleInfo module_info;

#endif

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
