#define CATCOMP_BLOCK
#include "string_data.h"

#include <dopus/common.h>
#include <proto/dopus5.h>
#include <proto/module.h>

void init_locale_data(struct DOpusLocale *locale)
{
	locale->li_BuiltIn = (char *)CatCompBlock;
}
