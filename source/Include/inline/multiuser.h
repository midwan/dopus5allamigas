#ifndef _INLINE_MULTIUSER_H
#define _INLINE_MULTIUSER_H

#ifndef CLIB_MULTIUSER_PROTOS_H
	#define CLIB_MULTIUSER_PROTOS_H
#endif

#ifndef __INLINE_MACROS_H
	#include <inline/macros.h>
#endif

#ifndef LIBRARIES_MULTIUSER_H
	#include <libraries/multiuser.h>
#endif

#ifndef MULTIUSER_BASE_NAME
	#define MULTIUSER_BASE_NAME muBase
#endif

#define muLogoutA(taglist) \
	LP1(0x1e, ULONG, muLogoutA, struct TagItem *, taglist, a0, , MULTIUSER_BASE_NAME)

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___muLogout(struct Library *muBase, ULONG taglist, ...)
{
	return muLogoutA((struct TagItem *)&taglist);
}

	#define muLogout(tags...) ___muLogout(MULTIUSER_BASE_NAME, tags)
#endif

#define muLoginA(taglist) \
	LP1(0x24, ULONG, muLoginA, struct TagItem *, taglist, a0, , MULTIUSER_BASE_NAME)

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___muLogin(struct Library *muBase, ULONG taglist, ...)
{
	return muLoginA((struct TagItem *)&taglist);
}

	#define muLogin(tags...) ___muLogin(MULTIUSER_BASE_NAME, tags)
#endif

#define muGetTaskOwner(task) \
	LP1(0x2a, ULONG, muGetTaskOwner, struct Task *, task, d0, , MULTIUSER_BASE_NAME)

#define muPasswd(oldpwd, newpwd) \
	LP2(0x30, BOOL, muPasswd, STRPTR, oldpwd, a0, STRPTR, newpwd, a1, , MULTIUSER_BASE_NAME)

#define muAllocUserInfo() \
	LP0(0x36, struct muUserInfo *, muAllocUserInfo, , MULTIUSER_BASE_NAME)

#define muFreeUserInfo(info) \
	LP1NR(0x3c, muFreeUserInfo, struct muUserInfo *, info, a0, , MULTIUSER_BASE_NAME)

#define muGetUserInfo(info, keytype) \
	LP2(0x42, struct muUserInfo *, muGetUserInfo, struct muUserInfo *, info, a0, ULONG, keytype, d0, , MULTIUSER_BASE_NAME)

#define muSetDefProtectionA(taglist) \
	LP1(0x4e, BOOL, muSetDefProtectionA, struct TagItem *, taglist, a0, , MULTIUSER_BASE_NAME)

#ifndef NO_INLINE_STDARG
static __inline__ BOOL ___muSetDefProtection(struct Library *muBase, ULONG taglist, ...)
{
	return muSetDefProtectionA((struct TagItem *)&taglist);
}

	#define muSetDefProtection(tags...) ___muSetDefProtection(MULTIUSER_BASE_NAME, tags)
#endif

#define muGetDefProtection(task) \
	LP1(0x54, ULONG, muGetDefProtection, struct Task *, task, d0, , MULTIUSER_BASE_NAME)

#define muSetProtection(name, mask) \
	LP2(0x5a, BOOL, muSetProtection, STRPTR, name, d1, LONG, mask, d2, , MULTIUSER_BASE_NAME)

#define muLimitDOSSetProtection(flag) \
	LP1(0x60, BOOL, muLimitDOSSetProtection, BOOL, flag, d0, , MULTIUSER_BASE_NAME)

#define muCheckPasswd(taglist) \
	LP1(0x66, BOOL, muCheckPasswd, struct TagItem *, taglist, a0, , MULTIUSER_BASE_NAME)

#ifndef NO_INLINE_STDARG
static __inline__ BOOL ___muCheckPasswdTags(struct Library *muBase, ULONG taglist, ...)
{
	return muCheckPasswd((struct TagItem *)&taglist);
}

	#define muCheckPasswdTags(tags...) ___muCheckPasswdTags(MULTIUSER_BASE_NAME, tags)
#endif

#define muGetPasswdDirLock() \
	LP0(0x72, BPTR, muGetPasswdDirLock, , MULTIUSER_BASE_NAME)

#define muGetConfigDirLock() \
	LP0(0x78, BPTR, muGetConfigDirLock, , MULTIUSER_BASE_NAME)

#define muGetTaskExtOwner(task) \
	LP1(0x7e, struct muExtOwner *, muGetTaskExtOwner, struct Task *, task, d0, , MULTIUSER_BASE_NAME)

#define muFreeExtOwner(info) \
	LP1NR(0x84, muFreeExtOwner, struct muExtOwner *, info, a0, , MULTIUSER_BASE_NAME)

#define muGetRelationshipA(user, owner, taglist) \
	LP3(0x8a, ULONG, muGetRelationshipA, struct muExtOwner *, user, d0, ULONG, owner, d1, struct TagItem *, taglist, a0, , MULTIUSER_BASE_NAME)

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___muGetRelationship(struct Library *muBase,
											 struct muExtOwner *user,
											 ULONG owner,
											 ULONG taglist,
											 ...)
{
	return muGetRelationshipA(user, owner, (struct TagItem *)&taglist);
}

	#define muGetRelationship(user, owner...) ___muGetRelationship(MULTIUSER_BASE_NAME, user, owner)
#endif

#define muUserInfo2ExtOwner(info) \
	LP1(0x90, struct muExtOwner *, muUserInfo2ExtOwner, struct muUserInfo *, info, a0, , MULTIUSER_BASE_NAME)

#define muAllocGroupInfo() \
	LP0(0x96, struct muGroupInfo *, muAllocGroupInfo, , MULTIUSER_BASE_NAME)

#define muFreeGroupInfo(info) \
	LP1NR(0x9c, muFreeGroupInfo, struct muGroupInfo *, info, a0, , MULTIUSER_BASE_NAME)

#define muGetGroupInfo(info, keytype) \
	LP2(0xa2, struct muGroupInfo *, muGetGroupInfo, struct muGroupInfo *, info, a0, ULONG, keytype, d0, , MULTIUSER_BASE_NAME)

#define muAddMonitor(monitor) \
	LP1(0xa8, BOOL, muAddMonitor, struct muMonitor *, monitor, a0, , MULTIUSER_BASE_NAME)

#define muRemMonitor(monitor) \
	LP1NR(0xae, muRemMonitor, struct muMonitor *, monitor, a0, , MULTIUSER_BASE_NAME)

#define muKill(task) \
	LP1(0xb4, BOOL, muKill, struct Task *, task, d0, , MULTIUSER_BASE_NAME)

#define muFreeze(task) \
	LP1(0xba, BOOL, muFreeze, struct Task *, task, d0, , MULTIUSER_BASE_NAME)

#define muUnfreeze(task) \
	LP1(0xc0, BOOL, muUnfreeze, struct Task *, task, d0, , MULTIUSER_BASE_NAME)

#endif /*  _INLINE_MULTIUSER_H  */
