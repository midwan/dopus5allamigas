#ifndef _MODULE_DEPS_H
#define _MODULE_DEPS_H

#include <dopus/common.h>
#include <proto/dopus5.h>
#include <proto/module.h>

extern struct DOpusLocale *locale;
extern ModuleInfo module_info;

#define VALID_QUALIFIERS                                                                                           \
	(IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND | IEQUALIFIER_CONTROL | IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | \
	 IEQUALIFIER_LALT | IEQUALIFIER_RALT)

void init_locale_data(struct DOpusLocale *);

#define userlibname "diskcopy.module"
#define textsegmentname "\0$VER: diskcopy.module " LIB_STRING

#endif
