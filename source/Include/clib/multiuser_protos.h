#ifndef CLIB_MULTIUSER_PROTOS_H
#define CLIB_MULTIUSER_PROTOS_H

/*
	multiuser.library C prototypes

	Copyright © 2002-2003 The MorphOS Development Team, All Rights Reserved.
*/

#ifndef LIBRARIES_MULTIUSER_H
	#include <libraries/multiuser.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

ULONG muLogoutA(struct TagItem *taglist);
#if !defined(USE_INLINE_STDARG)
ULONG muLogout(ULONG tag1type, ...);
#endif
ULONG muLoginA(struct TagItem *taglist);
#if !defined(USE_INLINE_STDARG)
ULONG muLogin(ULONG tag1type, ...);
#endif
ULONG muGetTaskOwner(struct Task *task);
BOOL muPasswd(STRPTR oldpwd, STRPTR newpwd);
struct muUserInfo *muAllocUserInfo(void);
void muFreeUserInfo(struct muUserInfo *info);
struct muUserInfo *muGetUserInfo(struct muUserInfo *info, ULONG keytype);
BOOL muSetDefProtectionA(struct TagItem *taglist);
#if !defined(USE_INLINE_STDARG)
BOOL muSetDefProtection(ULONG tag1type, ...);
#endif
ULONG muGetDefProtection(struct Task *task);
BOOL muSetProtection(STRPTR name, LONG mask);
BOOL muLimitDOSSetProtection(BOOL flag);
BOOL muCheckPasswd(struct TagItem *taglist);
BOOL muCheckPasswdTagList(struct TagItem *taglist);
#if !defined(USE_INLINE_STDARG)
BOOL muCheckPasswdTags(ULONG tag1type, ...);
#endif
BPTR muGetPasswdDirLock(void);
BPTR muGetConfigDirLock(void);
struct muExtOwner *muGetTaskExtOwner(struct Task *task);
void muFreeExtOwner(struct muExtOwner *owner);
ULONG muGetRelationshipA(struct muExtOwner *user, ULONG owner, struct TagItem *taglist);
#if !defined(USE_INLINE_STDARG)
ULONG muGetRelationship(struct muExtOwner *user, ULONG owner, ULONG tag1type, ...);
#endif
struct muExtOwner *muUserInfo2ExtOwner(struct muUserInfo *info);
struct muGroupInfo *muAllocGroupInfo(void);
void muFreeGroupInfo(struct muGroupInfo *info);
struct muGroupInfo *muGetGroupInfo(struct muGroupInfo *info, ULONG keytype);
BOOL muAddMonitor(struct muMonitor *monitor);
void muRemMonitor(struct muMonitor *monitor);
BOOL muKill(struct Task *task);
BOOL muFreeze(struct Task *task);
BOOL muUnfreeze(struct Task *task);
BOOL muFSRendezVous(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIB_MULTIUSER_PROTOS_H */
