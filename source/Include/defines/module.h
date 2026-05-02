/* Automatically generated header! Do not edit! */

#ifndef _INLINE_MODULE_H
#define _INLINE_MODULE_H

#ifndef AROS_LIBCALL_H
	#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef MODULE_BASE_NAME
	#define MODULE_BASE_NAME ModuleBase
#endif /* !MODULE_BASE_NAME */

#define Module_Entry(___files, ___screen, ___ipc, ___main_ipc, ___id, ___data) \
	AROS_LC6(int,                                                              \
			 Module_Entry,                                                     \
			 AROS_LCA(struct List *, (___files), A0),                          \
			 AROS_LCA(struct Screen *, (___screen), A1),                       \
			 AROS_LCA(IPCData *, (___ipc), A2),                                \
			 AROS_LCA(IPCData *, (___main_ipc), A3),                           \
			 AROS_LCA(IPTR, (___id), D0),                                      \
			 AROS_LCA(IPTR, (___data), D1),                                    \
			 struct Library *,                                                 \
			 MODULE_BASE_NAME,                                                 \
			 5,                                                                \
			 /* s */)

#define Module_Identify(___num)                                                                                      \
	AROS_LC1(ModuleInfo *, Module_Identify, AROS_LCA(int, (___num), D0), struct Library *, MODULE_BASE_NAME, 6, /* s \
																												 */)

#endif /* !_INLINE_MODULE_H */
