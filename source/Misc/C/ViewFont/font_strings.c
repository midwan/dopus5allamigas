#define CATCOMP_BLOCK
#include "font.strings"
#include <libraries/dopus5.h>

void init_locale_data(struct DOpusLocale *locale)
{
	locale->li_BuiltIn = (char *)CatCompBlock;
}
