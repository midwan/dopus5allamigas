#define CATCOMP_BLOCK
#include "filetype.strings"

#include <dopus/common.h>
#include <proto/dopus5.h>
#include <proto/module.h>

//#include "dopuslib:dopusbase.h"

void init_locale_data(struct DOpusLocale *locale)
{
	locale->li_BuiltIn = (char *)CatCompBlock;
}
