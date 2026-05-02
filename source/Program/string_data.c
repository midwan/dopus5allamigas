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

#define CATCOMP_BLOCK
#include "string_data.h"
#include "dopus.h"

#define LocaleBase locale->li_LocaleBase

void init_locale_data(struct DOpusLocale *locale)
{
	// Initialise, get pointer to built-in strings
	locale->li_Catalog = 0;
#ifdef __AROS__
	if ((locale->li_BuiltIn = AllocVec(sizeof(CatCompBlock), MEMF_PUBLIC)))
		CopyMem((APTR)CatCompBlock, locale->li_BuiltIn, sizeof(CatCompBlock));
	else
		locale->li_BuiltIn = (char *)CatCompBlock;
#else
	locale->li_BuiltIn = (char *)CatCompBlock;
#endif

	// Try to open locale library
	if ((locale->li_LocaleBase = OpenLibrary("locale.library", 38)))
	{
#ifdef __amigaos4__
		if (!(ILocale = (struct LocaleIFace *)GetInterface(LocaleBase, "main", 1, NULL)))
		{
			CloseLibrary(LocaleBase);
			return;
		}
#endif
		// Try for a catalog
		locale->li_Catalog = OpenCatalogA(0, "dopus.catalog", 0);

		// Get default locale
		locale->li_Locale = OpenLocale(0);
	}
}

void free_locale_data(struct DOpusLocale *locale)
{
	if (locale)
	{
#ifdef __AROS__
		if (locale->li_BuiltIn && locale->li_BuiltIn != CatCompBlock)
		{
			FreeVec(locale->li_BuiltIn);
			locale->li_BuiltIn = 0;
		}
#endif
		if (locale->li_LocaleBase)
		{
			CloseLocale(locale->li_Locale);
			CloseCatalog(locale->li_Catalog);
#ifdef __amigaos4__
			DropInterface((struct Interface *)ILocale);
#endif
			CloseLibrary(locale->li_LocaleBase);
		}
	}
}
