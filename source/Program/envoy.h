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

#ifndef _DOPUS_ENVOY
#define _DOPUS_ENVOY

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// Envoy stuff
struct UserInfo
{
	UBYTE ui_UserName[32];
	UWORD ui_UserID;
	UWORD ui_PrimaryGroupID;
	ULONG ui_Flags;
};
struct GroupInfo
{
	UBYTE gi_GroupName[32];
	UWORD gi_GroupID;
};

#ifndef __amigaos3__
	#pragma pack()
#endif

struct UserInfo *AllocUserInfo(void);
struct GroupInfo *AllocGroupInfo(void);
void FreeUserInfo(struct UserInfo *);
void FreeGroupInfo(struct GroupInfo *);

ULONG IDToUser(unsigned long userID, struct UserInfo *user);
ULONG IDToGroup(unsigned long groupID, struct GroupInfo *group);

#define ACTION_Envoy 20000

#define ACTION_NAME_TO_UID (ACTION_Envoy + 0)  // arg1 cstring
#define ACTION_NAME_TO_GID (ACTION_Envoy + 1)  // arg1 cstring
#ifndef ACTION_UID_TO_USERINFO
	#define ACTION_UID_TO_USERINFO (ACTION_Envoy + 2)
#endif
#ifndef ACTION_GID_TO_GROUPINFO
	#define ACTION_GID_TO_GROUPINFO (ACTION_Envoy + 3)
#endif

#define NEW_ACTION_UID_TO_USERINFO 1037
#define NEW_ACTION_GID_TO_GROUPINFO 1038

#endif
