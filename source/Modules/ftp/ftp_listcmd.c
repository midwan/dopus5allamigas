/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#include "ftp_listcmd.h"

#include <string.h>

static char ftp_listcmd_tolower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return (char)(c + ('a' - 'A'));

	return c;
}

static int ftp_listcmd_equal(const char *a, const char *b)
{
	if (!a || !b)
		return 0;

	while (*a && *b)
	{
		if (ftp_listcmd_tolower(*a) != ftp_listcmd_tolower(*b))
			return 0;

		++a;
		++b;
	}

	return *a == 0 && *b == 0;
}

static int ftp_listcmd_copy(char *dest, size_t dest_size, const char *src)
{
	size_t len;

	if (!dest || !src)
		return 0;

	len = strlen(src);
	if (dest_size <= len)
		return 0;

	memcpy(dest, src, len + 1);
	return 1;
}

int ftp_listcmd_next_after_failure(const char *current_cmd,
								   const char *default_cmd,
								   char *next_cmd,
								   size_t next_cmd_size)
{
	const char *fallback_default = default_cmd && *default_cmd ? default_cmd : FTP_LISTCMD_DEFAULT;

	if (ftp_listcmd_equal(current_cmd, FTP_LISTCMD_MLSD))
	{
		if (ftp_listcmd_equal(current_cmd, fallback_default))
			return 0;

		return ftp_listcmd_copy(next_cmd, next_cmd_size, fallback_default);
	}

	if (!ftp_listcmd_equal(fallback_default, FTP_LISTCMD_PLAIN) &&
		ftp_listcmd_equal(current_cmd, fallback_default))
		return ftp_listcmd_copy(next_cmd, next_cmd_size, FTP_LISTCMD_PLAIN);

	return 0;
}
