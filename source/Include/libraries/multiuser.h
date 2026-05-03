#ifndef LIBRARIES_MULTIUSER_H
#define LIBRARIES_MULTIUSER_H

/*
	multiuser.library include

	Copyright © 2002-2003 The MorphOS Development Team, All Rights Reserved.
*/

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef EXEC_LISTS_H
	#include <exec/lists.h>
#endif /* EXEC_LISTS_H */

#ifndef EXEC_LIBRARIES_H
	#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

#ifndef EXEC_EXECBASE_H
	#include <exec/execbase.h>
#endif /* EXEC_EXECBASE_H */

#ifndef EXEC_PORTS_H
	#include <exec/ports.h>
#endif /* EXEC_PORTS_H */

#ifndef LIBRARIES_DOS_H
	#include <libraries/dos.h>
#endif /* LIBRARIES_DOS_H */

#ifndef UTILITY_TAGITEM_H
	#include <utility/tagitem.h>
#endif /* UTILITY_TAGITEM_H */

#ifndef LIBRARIES_LOCALE_H
	#include <libraries/locale.h>
#endif /* LIBRARIES_LOCALE_H */

#ifndef __amigaos3__
	#pragma pack(2)
#endif

#define MULTIUSERNAME "multiuser.library"
#define MULTIUSERVERSION 39

#define MULTIUSERCATALOGNAME "multiuser.catalog"
#define MULTIUSERCATALOGVERSION 1

#define muOWNER_NOBODY 0x00000000
#define muOWNER_SYSTEM 0xffffffff

#define muMASK_UID 0xffff0000
#define muMASK_GID 0x0000ffff

#define muROOT_UID 0xffff
#define muROOT_GID 0xffff

#define muNOBODY_UID 0x0000

#define muUSERIDSIZE 32
#define muGROUPIDSIZE 32
#define muPASSWORDSIZE 32
#define muUSERNAMESIZE 220
#define muGROUPNAMESIZE 220
#define muHOMEDIRSIZE 256
#define muSHELLSIZE 256

#define muPasswd_FileName "passwd"
#define muGroup_FileName "MultiUser.group"
#define muConfig_FileName "MultiUser.config"
#define muLog_FileName "MultiUser.log"
#define muLastLogin_FileName ".lastlogin"
#define muProfile_FileName ".profile"
#define muPlan_FileName ".plan"
#define muKey_FileName ":.MultiUser.keyfile"
#define muT_Input (TAG_USER + 1)
#define muT_Output (TAG_USER + 2)
#define muT_Graphical (TAG_USER + 3)
#define muT_PubScrName (TAG_USER + 4)
#define muT_Task (TAG_USER + 5)
#define muT_Own (TAG_USER + 6)
#define muT_Global (TAG_USER + 7)
#define muT_Quiet (TAG_USER + 8)
#define muT_UserID (TAG_USER + 9)
#define muT_Password (TAG_USER + 10)
#define muT_DefProtection (TAG_USER + 11)
#define muT_All (TAG_USER + 12)
#define muT_NoLog (TAG_USER + 13)

struct muUserInfo
{
	char UserID[muUSERIDSIZE];
	UWORD uid;
	UWORD gid;
	char UserName[muUSERNAMESIZE];
	char HomeDir[muHOMEDIRSIZE];
	UWORD NumSecGroups;
	UWORD *SecGroups;
	char Shell[muSHELLSIZE];
};

struct muGroupInfo
{
	char GroupID[muGROUPIDSIZE];
	UWORD gid;
	UWORD MgrUid;
	char GroupName[muGROUPNAMESIZE];
};

#define muKeyType_First 0
#define muKeyType_Next 1
#define muKeyType_gid 4

#define muKeyType_UserID 2
#define muKeyType_uid 3
#define muKeyType_gidNext 5
#define muKeyType_UserName 6
#define muKeyType_WUserID 7
#define muKeyType_WUserName 8
#define muKeyType_WUserIDNext 9
#define muKeyType_WUserNameNext 10

#define muKeyType_GroupID 11
#define muKeyType_WGroupID 12
#define muKeyType_WGroupIDNext 13
#define muKeyType_GroupName 14
#define muKeyType_WGroupName 15
#define muKeyType_WGroupNameNext 16
#define muKeyType_MgrUid 17
#define muKeyType_MgrUidNext 18

struct muExtOwner
{
	UWORD uid;
	UWORD gid;
	UWORD NumSecGroups;
};

#define muSecGroups(x) ((UWORD *)((UBYTE *)(x) + sizeof(struct muExtOwner)))
#define muExtOwner2ULONG(x) ((ULONG)((x) ? (x)->uid << 16 | (x)->gid : muOWNER_NOBODY))

#define muFIBB_SET_UID 31
#define muFIBF_SET_UID (1 << muFIBB_SET_UID)

#define DEFPROTECTION (FIBF_OTR_READ | FIBF_GRP_READ)

#define muRelB_ROOT_UID 0
#define muRelB_ROOT_GID 1
#define muRelB_NOBODY 2
#define muRelB_UID_MATCH 3
#define muRelB_GID_MATCH 4
#define muRelB_PRIM_GID 5
#define muRelB_NO_OWNER 6

#define muRelF_ROOT_UID (1 << muRelB_ROOT_UID)
#define muRelF_ROOT_GID (1 << muRelB_ROOT_GID)
#define muRelF_NOBODY (1 << muRelB_NOBODY)
#define muRelF_UID_MATCH (1 << muRelB_UID_MATCH)
#define muRelF_GID_MATCH (1 << muRelB_GID_MATCH)
#define muRelF_PRIM_GID (1 << muRelB_PRIM_GID)
#define muRelF_NO_OWNER (1 << muRelB_NO_OWNER)

struct muMonitor
{
	struct MinNode Node;
	ULONG Mode;
	ULONG Triggers;
	union
	{
		struct
		{
			struct Task *Task;
			ULONG SignalNum;
		} Signal;

		struct
		{
			struct MsgPort *Port;
		} Message;

	} muMonitor_sendunion;
};

#define muMon_IGNORE 0
#define muMon_SEND_SIGNAL 1
#define muMon_SEND_MESSAGE 2

struct muMonMsg
{
	struct Message ExecMsg;
	struct muMonitor *Monitor;
	ULONG Trigger;
	UWORD From;
	UWORD To;
	char UserID[muUSERIDSIZE];
};

#define muTrgB_OwnerChange 0
#define muTrgB_Login 1
#define muTrgB_LoginFail 2
#define muTrgB_Passwd 3
#define muTrgB_PasswdFail 4
#define muTrgB_CheckPasswd 5
#define muTrgB_CheckPasswdFail 6

#define muTrgF_OwnerChange (1 << muTrgB_OwnerChange)
#define muTrgF_Login (1 << muTrgB_Login)
#define muTrgF_LoginFail (1 << muTrgB_LoginFail)
#define muTrgF_Passwd (1 << muTrgB_Passwd)
#define muTrgF_PasswdFail (1 << muTrgB_PasswdFail)
#define muTrgF_CheckPasswd (1 << muTrgB_CheckPasswd)
#define muTrgF_CheckPasswdFail (1 << muTrgB_CheckPasswdFail)

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif /* LIBRARIES_MULTIUSER_H */
