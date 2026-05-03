#ifndef _MODULE_DEPS_H
#define _MODULE_DEPS_H

#include <dopus/common.h>
#include <proto/dopus5.h>
#include <proto/module.h>

extern struct DOpusLocale *locale;
typedef struct
{
	ULONG ver;					 // Module version
	char *name;					 // Module name
	char *locale_name;			 // Catalog name
	ULONG flags;				 // Module flags
	ULONG function_count;		 // Number of functions in module
	ModuleFunction function[4];	 // All the module functions
} ModuleInfo_4;
extern ModuleInfo_4 module_info;

#define VALID_QUALIFIERS                                                                                           \
	(IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND | IEQUALIFIER_CONTROL | IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | \
	 IEQUALIFIER_LALT | IEQUALIFIER_RALT)

void init_locale_data(struct DOpusLocale *);

#define userlibname "themes.module"
#define textsegmentname "\0$VER: themes.module " LIB_STRING

#endif
