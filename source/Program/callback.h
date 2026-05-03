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
#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#define MAINCMD_GET_CALLBACKS 0x40100

struct pointer_packet;

DirEntry *ASM HookCreateFileEntry(REG(a0, Lister *lister), REG(a1, struct FileInfoBlock *fib), REG(d0, BPTR lock));

DirEntry *ASM HookAddFileEntry(REG(a0, Lister *lister), REG(a1, DirEntry *entry), REG(d0, BOOL sort));

void ASM HookFileSet(REG(a0, Lister *lister), REG(a1, DirEntry *entry), REG(a2, struct TagItem *tags));

void ASM HookSortFileList(REG(a0, Lister *lister),
						  REG(a1, struct List *list),
						  REG(d0, long file_count),
						  REG(d1, long dir_count));

void ASM HookResortLister(REG(a0, Lister *lister), REG(a1, ListFormat *format));

void ASM HookRefreshLister(REG(a0, Lister *lister), REG(d0, ULONG flags));

void ASM HookLockFileList(REG(a0, Lister *lister), REG(d0, BOOL exclusive));

void ASM HookUnlockFileList(REG(a0, Lister *lister));

DirEntry *ASM HookFindFileEntry(REG(a0, Lister *lister), REG(a1, char *name));

BOOL ASM HookSetFileComment(REG(a0, Lister *lister), REG(a1, char *name), REG(a2, char *comment));

void ASM HookRemoveFileEntry(REG(a0, Lister *lister), REG(a1, DirEntry *entry));

BOOL ASM HookFileQuery(REG(a0, Lister *lister), REG(a1, DirEntry *entry), REG(a2, struct TagItem *tags));

void ASM HookShowHelp(REG(a0, char *file_name), REG(a1, char *node_name));

APTR ASM HookConvertEntry(REG(a0, FunctionEntry *entry));

Lister *ASM HookGetLister(REG(a0, PathNode *path));

PathNode *ASM HookGetSource(REG(a0, FunctionHandle *handle), REG(a1, char *pathbuf));

PathNode *ASM HookNextSource(REG(a0, FunctionHandle *handle), REG(a1, char *pathbuf));

void ASM HookUnlockSource(REG(a0, FunctionHandle *handle));

PathNode *ASM HookGetDest(REG(a0, FunctionHandle *handle), REG(a1, char *pathbuf));

void ASM HookEndSource(REG(a0, FunctionHandle *handle), REG(d0, long complete));

void ASM HookEndDest(REG(a0, FunctionHandle *handle), REG(d0, long complete));

void ASM HookFirstEntry(REG(a0, FunctionHandle *handle));

APTR ASM HookGetEntry(REG(a0, FunctionHandle *handle));

ULONG ASM HookExamineEntry(REG(a0, FunctionEntry *entry), REG(d0, long type));

void ASM HookEndEntry(REG(a0, FunctionHandle *handle), REG(a1, APTR entry), REG(d0, BOOL deselect));

void ASM HookRemoveEntry(REG(a0, FunctionEntry *entry));

long ASM HookEntryCount(REG(a0, FunctionHandle *handle));

void ASM HookReloadEntry(REG(a0, FunctionHandle *handle), REG(a1, FunctionEntry *entry));

void ASM HookAddFile(REG(a0, FunctionHandle *handle),
					 REG(a1, char *path),
					 REG(a2, struct FileInfoBlock *fib),
					 REG(a3, Lister *lister));

void ASM HookDelFile(REG(a0, FunctionHandle *handle),
					 REG(a1, char *path),
					 REG(a2, char *name),
					 REG(a3, Lister *lister));

void ASM HookLoadFile(REG(a0, FunctionHandle *handle),
					  REG(a1, char *path),
					  REG(a2, char *name),
					  REG(d0, long flags),
					  REG(d1, BOOL reload));

void ASM HookDoChanges(REG(a0, FunctionHandle *handle));

BOOL ASM HookCheckAbort(REG(a0, FunctionHandle *handle));

struct Window *ASM HookGetWindow(REG(a0, PathNode *path));

struct MsgPort *ASM HookGetPort(REG(a0, char *portname));

struct Screen *ASM HookGetScreen(REG(a0, char *screenname));

DOpusScreenData *ASM HookGetScreenData(void);

void ASM HookFreeScreenData(REG(a0, APTR data));

void ASM HookOpenProgress(REG(a0, PathNode *path), REG(a1, char *operation), REG(d0, long total));

void ASM HookUpdateProgress(REG(a0, PathNode *path), REG(a1, char *name), REG(d0, long count));

void ASM HookCloseProgress(REG(a0, PathNode *path));

long ASM HookReplaceReq(REG(a0, struct Window *window),
						REG(a1, struct Screen *screen),
						REG(a2, IPCData *ipc),
						REG(a3, struct FileInfoBlock *file1),
						REG(a4, struct FileInfoBlock *file2),
						REG(d0, long default_option));

ULONG ASM HookGetPointer(REG(a0, struct pointer_packet *ptr));

void ASM HookFreePointer(REG(a0, struct pointer_packet *ptr));

ULONG ASM HookSendCommand(REG(a0, FunctionHandle *handle),
						  REG(a1, char *command),
						  REG(a2, char **result),
						  REG(d0, ULONG flags));

void ASM HookCheckDesktop(REG(a0, char *path));

BOOL ASM HookMatchDesktop(REG(a0, char *path));

UWORD ASM HookGetDesktop(REG(a0, char *path));

void ASM HookGetThemes(REG(a0, char *path));

BOOL ASM HookScript(REG(a0, char *name), REG(a1, char *data));

short ASM HookDesktopPopup(REG(a0, ULONG flags));

long ASM HookRexxCommand(REG(a0, char *command),
						 REG(a1, char *result),
						 REG(d0, long length),
						 REG(a2, struct MsgPort *replyport),
						 REG(d1, ULONG flags));

long ASM HookFileRequest(REG(a0, struct Window *parent),
						 REG(a1, char *title),
						 REG(a2, char *initial_path),
						 REG(a3, char *pathname),
						 REG(d0, ULONG flags),
						 REG(d1, char *pattern));

/*
#ifndef __amigaos3__
#pragma pack(2)
#endif

typedef struct
{
	UWORD	dc_Count;

	APTR	dc_CreateFileEntry;
	APTR	dc_FileSet;
	APTR	dc_SortFileList;
	APTR	dc_AddFileEntry;
	APTR	dc_ResortLister;
	APTR	dc_RefreshLister;
	APTR	dc_LockFileList;
	APTR	dc_UnlockFileList;
	APTR	dc_FindFileEntry;
	APTR	dc_SetFileComment;
	APTR	dc_RemoveFileEntry;
	APTR	dc_FileQuery;
	APTR	dc_ShowHelp;

	APTR	dc_ConvertEntry;
	APTR	dc_GetLister;
	APTR	dc_GetSource;
	APTR	dc_NextSource;
	APTR	dc_UnlockSource;
	APTR	dc_GetDest;
	APTR	dc_EndSource;
	APTR	dc_EndDest;
	APTR	dc_GetEntry;
	APTR	dc_ExamineEntry;
	APTR	dc_EndEntry;
	APTR	dc_RemoveEntry;
	APTR	dc_EntryCount;
	APTR	dc_ReloadEntry;
	APTR	dc_AddFile;
	APTR	dc_DelFile;
	APTR	dc_LoadFile;
	APTR	dc_DoChanges;
	APTR	dc_CheckAbort;
	APTR	dc_GetWindow;
	APTR	dc_GetPort;
	APTR	dc_GetScreen;
	APTR	dc_GetScreenData;
	APTR	dc_FreeScreenData;
	APTR	dc_OpenProgress;
	APTR	dc_UpdateProgress;
	APTR	dc_CloseProgress;
	APTR	dc_ReplaceReq;
	APTR	dc_GetPointer;
	APTR	dc_FreePointer;
	APTR	dc_SendCommand;
	APTR	dc_CheckDesktop;
	APTR	dc_GetDesktop;
	APTR	dc_Script;
	APTR	dc_DesktopPopup;
	APTR	dc_FirstEntry;
	APTR	dc_RexxCommand;
	APTR	dc_FileRequest;
	APTR	dc_GetThemes;
} DOpusCallbackInfo;

#ifndef __amigaos3__
#pragma pack()
#endif
*/

#define DOPUS_HOOK_COUNT 52

// Values for HookFileSet

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
#define HFFS_VERSION (TAG_USER + 0xc)	  // ULONG *		- version, revision, days
#define HFFS_MENU (TAG_USER + 0xd)		  // void *		- not implemented

#define HOOKREFRESH_DATE (1 << 0)
#define HOOKREFRESH_FULL (1 << 1)

long HookInitHooks(DOpusCallbackInfo *);
/*
enum
{
	EE_NAME,		// Get name pointer (READ ONLY!)
	EE_TYPE,		// Get type (<0 = file, >0 = dir)
};
*/

#endif
