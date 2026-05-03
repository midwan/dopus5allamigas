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

#ifndef _DOPUS_APPMSG
#define _DOPUS_APPMSG

DOpusAppMessage *alloc_appmsg_files(DirEntry *, DirBuffer *, BOOL);
struct ArgArray *AppArgArray(DOpusAppMessage *, short);
void FreeArgArray(struct ArgArray *);
void set_appmsg_data(DOpusAppMessage *, ULONG, ULONG, ULONG);
BOOL get_appmsg_data(DOpusAppMessage *, ULONG *, ULONG *, ULONG *);
short FindWBArg(struct WBArg *args, short count, char *name);
void UnlockWBArg(struct WBArg *args, short count);
struct ArgArray *WBArgArray(struct WBArg *, short, short);

struct ArgArray
{
	struct MinList aa_List;
	APTR aa_Memory;
	ULONG aa_Flags;
	ULONG aa_Count;
};

#ifndef __amigaos3__
	#pragma pack(2)
#endif

struct ArgArrayEntry
{
	struct MinNode ae_Node;
	UWORD ae_Flags;
	char ae_String[1];
};

#ifndef __amigaos3__
	#pragma pack()
#endif

#define AAF_ALLOW_DIRS (1 << 0)

#define AEF_DIR (1 << 0)
#define AEF_LINK (1 << 1)
#define AEF_FAKE_ICON (1 << 2)

#define BuildArgArray(...)                                   \
	({                                                       \
		IPTR __args[] = {__VA_ARGS__};                       \
		(struct ArgArray *)BuildArgArrayA((char **)&__args); \
	})
struct ArgArray *BuildArgArrayA(char **);
struct ArgArray *NewArgArray(void);
struct ArgArrayEntry *NewArgArrayEntry(struct ArgArray *, char *);

#endif
