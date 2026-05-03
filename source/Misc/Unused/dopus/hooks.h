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

#ifndef _DOPUS_HOOK
#define _DOPUS_HOOK

/*****************************************************************************

 Directory Opus Direct Hooks

 *****************************************************************************/

// Values for dc_FileSet/dc_FileQuery

#define HFFS_NAME (TAG_USER + 0x1)		  // char * 		- Entry name
#define HFFS_SIZE (TAG_USER + 0x2)		  // ULONG 		- Entry size
#define HFFS_PROTECTION (TAG_USER + 0x3)  // ULONG 		- Protection flags
#define HFFS_DATE (TAG_USER + 0x4)		  // struct Datestamp *	- Entry date
#define HFFS_COMMENT (TAG_USER + 0x5)	  // char *		- Comment
#define HFFS_SELECTED (TAG_USER + 0x6)	  // BOOL			- Selected state
#define HFFS_LINK (TAG_USER + 0x7)		  // BOOL			- Set if a link
#define HFFS_COLOUR (TAG_USER + 0x8)	  // ULONG		- 1 = device, 2 = assign
#define HFFS_USERDATA (TAG_USER + 0x9)	  // ULONG		- Userdata
#define HFFS_FILETYPE (TAG_USER + 0xa)	  // char *		- Filetype description
#define HFFS_DISPLAY (TAG_USER + 0xb)	  // char *		- Custom display string
#define HFFS_VERSION (TAG_USER + 0xc)	  // VersionInfo *	- Version information
#define HFFS_MENU (TAG_USER + 0xd)		  // struct List *	- Custom menus for entry
#define HFFS_ICON (TAG_USER + 0xe)		  // struct DiskObject *	- not implemented

// Additional file requester flags
#define DCRFF_PATTERN (1 << 29)
#define DCRFF_SCREENPARENT (1 << 30)
#define DCRFF_REJECTICONS (1 << 31)

#if 0
// Holds version information (used by HFFS_VERSION)
typedef struct _VersionInfo {
	UWORD		vi_Version;		// Major version #
	UWORD		vi_Revision;		// Minor revision #
	long		vi_Days;		// Days of date
	ULONG		vi_Flags;		// Flags
	char		vi_Char;		// Version character
	char		vi_String[1];		// Version string
} VersionInfo;
#endif

// Flags for dc_RefreshLister()
#define HOOKREFRESH_DATE (1 << 0)
#define HOOKREFRESH_FULL (1 << 1)

// Type value for dc_ExamineEntry()
enum {
	EE_NAME,  // Get pointer to name (READ ONLY!)
	EE_TYPE,  // Get type (<0 = file, >0 = dir)
};

// Flags for dc_ReplaceReq()
#define REPREQF_NOVERSION (1 << 16)	 // No 'version' button

// Define this if you want to use dc_ResortLister
#ifdef DOPUSHOOK_INCLUDE_FORMAT

// Sort format
struct SortFormat
{
	BYTE sf_Sort;		 // Sort method
	BYTE sf_SortFlags;	 // Sort flags
	BYTE sf_Separation;	 // File separation
};

	#define SORT_REVERSE (1 << 0)  // Sort in reverse order

// List format
struct ListFormat
{
	// Colour fields, not used at present
	UBYTE lf_FilesUnsel[2];	 // Unselected files
	UBYTE lf_FilesSel[2];	 // Selected files
	UBYTE lf_DirsUnsel[2];	 // Unselected directories
	UBYTE lf_DirsSel[2];	 // Selected directories

	// Sort information
	struct SortFormat lf_Sort;	// Sort method
	BYTE lf_DisplayPos[16];		// Item display position
	BYTE lf_DisplayLen[15];		// Display length (not used)

	UBYTE lf_Flags;	 // See LFORMATF_xxx below

	// Not used
	BYTE lf_ShowFree;  // Show free space type

	// You must call ParsePattern() yourself
	char lf_ShowPattern[40];   // Show pattern
	char lf_HidePattern[40];   // Hide pattern
	char lf_ShowPatternP[40];  // Show pattern parsed
	char lf_HidePatternP[40];  // Hide pattern parsed
};

	#define LFORMATF_REJECT_ICONS (1 << 0)	// Reject icons
	#define LFORMATF_HIDDEN_BIT (1 << 1)	// Respect the H bit
	#define LFORMATF_ICON_VIEW (1 << 2)		// Default to icon view
	#define LFORMATF_SHOW_ALL (1 << 3)		// Show all

// Used for the sf_Sort and lf_DisplayPos fields
enum {
	DISPLAY_NAME,
	DISPLAY_SIZE,
	DISPLAY_PROTECT,
	DISPLAY_DATE,
	DISPLAY_COMMENT,
	DISPLAY_FILETYPE,
	DISPLAY_OWNER,
	DISPLAY_GROUP,
	DISPLAY_NETPROT,
	DISPLAY_VERSION,

	DISPLAY_LAST
};

// Used for the sf_Separation field
enum {
	SEPARATE_MIX,
	SEPARATE_DIRSFIRST,
	SEPARATE_FILESFIRST,
};

#endif

// Used for the dc_GetPointer/dc_FreePointer hooks
struct GetPointerPkt
{
	ULONG gpp_Type;
	APTR gpp_Ptr;
	ULONG gpp_Flags;
};

#define GETPTR_HANDLE 2	   // Get a function handle
#define GETPTR_COMMANDS 4  // Get internal command list

#ifndef DEF_DOPUSCOMMANDLIST
	#define DEF_DOPUSCOMMANDLIST

struct DOpusCommandList
{
	struct Node dcl_Node;	// Command name in ln_Name
	ULONG dcl_Flags;		// Function flags
	char *dcl_Description;	// Description string
	char *dcl_Template;		// Command template
	char *dcl_Module;		// Module command resides in
	char *dcl_Help;			// Help file for command
};

#endif

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// Use this command to get the address of the hooks from the Opus process.
// Send it to the main Opus IPC, and supply the address of a DOpusCallbackInfo
// structure in the data field of the message.

#define HOOKCMD_GET_CALLBACKS 0x40100

#define DOPUS_HOOK_COUNT 52

// This structure will grow in the future, correct use of the dc_Count field
// ensures innocent memory won't get overwritten.

typedef struct
{
	// Set to the number of hooks you want
	UWORD dc_Count;

	// Create a file entry
	APTR ASM (*dc_CreateFileEntry)(REG(a0, ULONG lister), REG(a1, struct FileInfoBlock *fib), REG(d0, BPTR lock));

	// Change parameters of a file entry
	VOID ASM (*dc_FileSet)(REG(a0, ULONG lister), REG(a1, APTR entry), REG(a2, struct TagItem *tags));

	// Sort list of entries into a lister
	VOID ASM (*dc_SortFileList)(REG(a0, ULONG lister),
								REG(a1, struct List *list),
								REG(d0, long file_count),
								REG(d1, long dir_count));

	// Add single entry to a lister
	APTR ASM (*dc_AddFileEntry)(REG(a0, ULONG lister), REG(a1, APTR entry), REG(d0, BOOL sort));

	// Resort a lister
	VOID ASM (*dc_ResortLister)(REG(a0, ULONG lister), REG(a1, struct ListFormat *format));

	// Refresh a lister
	VOID ASM (*dc_RefreshLister)(REG(a0, ULONG lister), REG(d0, ULONG flags));

	// Lock lister file list
	VOID ASM (*dc_LockFileList)(REG(a0, ULONG lister), REG(d0, BOOL exclusive));

	// Unlock file list
	VOID ASM (*dc_UnlockFileList)(REG(a0, ULONG lister));

	// Find entry in a lister by name
	APTR ASM (*dc_FindFileEntry)(REG(a0, ULONG lister), REG(a1, char *name));

	// Change comment of an entry
	BOOL ASM (*dc_SetFileComment)(REG(a0, ULONG lister), REG(a1, char *name), REG(a2, char *comment));

	// Remove file entry from a lister
	VOID ASM (*dc_RemoveFileEntry)(REG(a0, ULONG lister), REG(a1, APTR entry));

	// Query file entry
	BOOL ASM (*dc_FileQuery)(REG(a0, ULONG lister), REG(a1, APTR entry), REG(a2, struct TagItem *tags));

	// Show help
	void ASM (*dc_ShowHelp)(REG(a0, char *file_name), REG(a1, char *node_name));

	// Convert entry pointer from one type to another
	APTR ASM (*dc_ConvertEntry)(REG(a0, APTR entry));

	// Get lister pointer from a path handle
	ULONG ASM (*dc_GetLister)(REG(a0, APTR path));

	// Get first source lister
	APTR ASM (*dc_GetSource)(REG(a0, APTR handle), REG(a1, char *path));

	// Get next source lister
	APTR ASM (*dc_NextSource)(REG(a0, APTR handle), REG(a1, char *path));

	// Unlock source listers
	void ASM (*dc_UnlockSource)(REG(a0, APTR handle));

	// Get next destination lister
	APTR ASM (*dc_GetDest)(REG(a0, APTR handle), REG(a1, char *path));

	// End use of source lister
	void ASM (*dc_EndSource)(REG(a0, APTR handle), REG(d0, long complete));

	// End use of destination lister
	void ASM (*dc_EndDest)(REG(a0, APTR handle), REG(d0, long complete));

	// Get next selected entry
	APTR ASM (*dc_GetEntry)(REG(a0, APTR handle));

	// Examine an entry
	ULONG ASM (*dc_ExamineEntry)(REG(a0, APTR entry), REG(d0, long type));

	// End use of an entry
	void ASM (*dc_EndEntry)(REG(a0, APTR handle), REG(a1, APTR entry), REG(d0, BOOL deselect));

	// Mark an entry for removal
	void ASM (*dc_RemoveEntry)(REG(a0, APTR entry));

	// Return count of selected entries
	long ASM (*dc_EntryCount)(REG(a0, APTR handle));

	// Mark an entry to be reloaded
	void ASM (*dc_ReloadEntry)(REG(a0, APTR handle), REG(a1, APTR entry));

	// Add a file to a lister
	void ASM (*dc_AddFile)(REG(a0, APTR handle),
						   REG(a1, char *path),
						   REG(a2, struct FileInfoBlock *fib),
						   REG(a3, ULONG lister));

	// Delete a file from a lister
	void ASM (*dc_DelFile)(REG(a0, APTR handle), REG(a1, char *path), REG(a2, char *name), REG(a3, ULONG lister));

	// Load/reload a file in a lister
	void ASM (*dc_LoadFile)(REG(a0, APTR handle),
							REG(a1, char *path),
							REG(a2, char *name),
							REG(d0, long flags),
							REG(d1, BOOL reload));

	// Perform changes on a lister
	void ASM (*dc_DoChanges)(REG(a0, APTR handle));

	// Check for user abort
	BOOL ASM (*dc_CheckAbort)(REG(a0, APTR handle));

	// Get window pointer from a path handle
	struct Window *ASM (*dc_GetWindow)(REG(a0, APTR path));

	// Get Opus ARexx port name
	struct MsgPort *ASM (*dc_GetPort)(REG(a0, char *name));

	// Get Opus public screen name
	struct Screen *ASM (*dc_GetScreen)(REG(a0, char *name));

	// Get information about the Opus screen
	struct DOpusScreenData *ASM (*dc_GetScreenData)(void);

	// Free screen data structure
	void ASM (*dc_FreeScreenData)(void);

	// Open progress indicator in a lister
	void ASM (*dc_OpenProgress)(REG(a0, APTR path), REG(a1, char *operation), REG(d0, long total));

	// Update progress indicator
	void ASM (*dc_UpdateProgress)(REG(a0, APTR path), REG(a1, char *name), REG(d0, long count));

	// Close progress indicator
	void ASM (*dc_CloseProgress)(REG(a0, APTR path));

	// Show 'File exists - Replace?' requester
	long ASM (*dc_ReplaceReq)(REG(a0, struct Window *window),
							  REG(a1, struct Screen *screen),
							  REG(a2, IPCData *ipc),
							  REG(a3, struct FileInfoBlock *file1),
							  REG(a4, struct FileInfoBlock *file2),
							  REG(d0, long flags));

	// Get pointer to Opus internal things
	APTR ASM (*dc_GetPointer)(REG(a0, struct GetPointerPkt *));
	void ASM (*dc_FreePointer)(REG(a0, struct GetPointerPkt *));

	// Send an ARexx command direct to Opus
	ULONG ASM (*dc_SendCommand)(REG(a0, APTR handle),
								REG(a1, char *command),
								REG(a2, char **result),
								REG(d0, ULONG flags));

	// Make DOpus check if the desktop needs updating
	void ASM (*dc_CheckDesktop)(REG(a0, char *path));

	// Get desktop path
	short ASM (*dc_GetDesktop)(REG(a0, char *buffer));

	// Run script
	short ASM (*dc_Script)(REG(a0, char *name), REG(a1, char *data));

	// Popup desktop popup
	short ASM (*dc_DesktopPopup)(REG(a0, ULONG flags));

	// Reset file entries
	void ASM (*dc_FirstEntry)(REG(a0, APTR handle));

	// Send rexx command direct to DOpus
	long ASM (*dc_RexxCommand)(REG(a0, char *command),
							   REG(a1, char *result),
							   REG(d0, long length),
							   REG(a2, struct MsgPort *replyport),
							   REG(d1, ULONG flags));

	// Show file requester
	long ASM (*dc_FileRequest)(REG(a0, struct Window *parent),
							   REG(a1, char *title),
							   REG(a2, char *initial_path),
							   REG(a3, char *pathname),
							   REG(d0, ULONG flags),
							   REG(d1, char *pattern));

	// Get themes path
	void ASM (*dc_GetThemes)(REG(a0, char *buffer));

} DOpusCallbackInfo;

#ifndef __amigaos3__
	#pragma pack()
#endif

#if defined(__amigaos4__)

	#define DC_REGA0 ET_RegisterA0
	#define DC_REGA1 ET_RegisterA1
	#define DC_REGA2 ET_RegisterA2
	#define DC_REGA3 ET_RegisterA3
	#define DC_REGA4 ET_RegisterA4
	#define DC_REGD0 ET_RegisterD0
	#define DC_REGD1 ET_RegisterD1
	#define DC_REGD2 ET_RegisterD2

	#define DC_CALL1(dc, func, t1, v1) EmulateTags(dc->func, t1, v1, TAG_DONE)

	#define DC_CALL2(dc, func, t1, v1, t2, v2) EmulateTags(dc->func, t1, v1, t2, v2, TAG_DONE)

	#define DC_CALL3(dc, func, t1, v1, t2, v2, t3, v3) EmulateTags(dc->func, t1, v1, t2, v2, t3, v3, TAG_DONE)

	#define DC_CALL4(dc, func, t1, v1, t2, v2, t3, v3, t4, v4) \
		EmulateTags(dc->func, t1, v1, t2, v2, t3, v3, t4, v4, TAG_DONE)

	#define DC_CALL5(dc, func, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5) \
		EmulateTags(dc->func, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, TAG_DONE)

	#define DC_CALL6(dc, func, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6) \
		EmulateTags(dc->func, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, TAG_DONE)

#elif defined(__MORPHOS__)

	#define DC_REGA0 REG_A0
	#define DC_REGA1 REG_A0
	#define DC_REGA2 REG_A0
	#define DC_REGA3 REG_A0
	#define DC_REGA4 REG_A0
	#define DC_REGD0 REG_A0
	#define DC_REGD1 REG_D1
	#define DC_REGD2 REG_D2

	#define DC_CALL1(dc, func, t1, v1)                    \
		({                                                \
			t1 = (ULONG)v1;                               \
			(*MyEmulHandle->EmulCallDirect68k)(dc->func); \
		})

	#define DC_CALL2(dc, func, t1, v1, t2, v2)            \
		({                                                \
			t1 = (ULONG)v1;                               \
			t2 = (ULONG)v2;                               \
			(*MyEmulHandle->EmulCallDirect68k)(dc->func); \
		})

	#define DC_CALL3(dc, func, t1, v1, t2, v2, t3, v3)    \
		({                                                \
			t1 = (ULONG)v1;                               \
			t2 = (ULONG)v2;                               \
			t3 = (ULONG)v3;                               \
			(*MyEmulHandle->EmulCallDirect68k)(dc->func); \
		})

	#define DC_CALL4(dc, func, t1, v1, t2, v2, t3, v3, t4, v4) \
		({                                                     \
			t1 = (ULONG)v1;                                    \
			t2 = (ULONG)v2;                                    \
			t3 = (ULONG)v3;                                    \
			t4 = (ULONG)v4;                                    \
			(*MyEmulHandle->EmulCallDirect68k)(dc->func);      \
		})

	#define DC_CALL5(dc, func, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5) \
		({                                                             \
			t1 = (ULONG)v1;                                            \
			t2 = (ULONG)v2;                                            \
			t3 = (ULONG)v3;                                            \
			t4 = (ULONG)v4;                                            \
			t5 = (ULONG)v5;                                            \
			(*MyEmulHandle->EmulCallDirect68k)(dc->func);              \
		})

	#define DC_CALL6(dc, func, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6) \
		({                                                                     \
			t1 = (ULONG)v1;                                                    \
			t2 = (ULONG)v2;                                                    \
			t3 = (ULONG)v3;                                                    \
			t4 = (ULONG)v4;                                                    \
			t5 = (ULONG)v5;                                                    \
			t6 = (ULONG)v6;                                                    \
			(*MyEmulHandle->EmulCallDirect68k)(dc->func);                      \
		})

#else

	#define DC_REGA0
	#define DC_REGA1
	#define DC_REGA2
	#define DC_REGA3
	#define DC_REGA4
	#define DC_REGD0
	#define DC_REGD1
	#define DC_REGD2

	#define DC_CALL1(dc, func, t1, v1) dc->func(v1)

	#define DC_CALL2(dc, func, t1, v1, t2, v2) dc->func(v1, v2)

	#define DC_CALL3(dc, func, t1, v1, t2, v2, t3, v3) dc->func(v1, v2, v3)

	#define DC_CALL4(dc, func, t1, v1, t2, v2, t3, v3, t4, v4) dc->func(v1, v2, v3, v4)

	#define DC_CALL5(dc, func, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5) dc->func(v1, v2, v3, v4, v5)

	#define DC_CALL6(dc, func, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6) dc->func(v1, v2, v3, v4, v5, v6)

#endif

#endif
