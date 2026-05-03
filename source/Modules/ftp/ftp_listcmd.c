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

static const char *ftp_listcmd_skip_padding(const char *text)
{
	while (*text == ' ' || *text == '\t')
		++text;

	return text;
}

static const char *ftp_listcmd_trim_end(const char *start)
{
	const char *end = start + strlen(start);

	while (end > start && (end[-1] == ' ' || end[-1] == '\t'))
		--end;

	return end;
}

static int ftp_listcmd_has_text(const char *text)
{
	const char *start;

	if (!text)
		return 0;

	start = ftp_listcmd_skip_padding(text);
	return ftp_listcmd_trim_end(start) > start;
}

static int ftp_listcmd_equal(const char *a, const char *b)
{
	const char *a_end;
	const char *b_end;

	if (!a || !b)
		return 0;

	a = ftp_listcmd_skip_padding(a);
	b = ftp_listcmd_skip_padding(b);

	a_end = ftp_listcmd_trim_end(a);
	b_end = ftp_listcmd_trim_end(b);

	while (a < a_end && b < b_end)
	{
		if (ftp_listcmd_tolower(*a) != ftp_listcmd_tolower(*b))
			return 0;

		++a;
		++b;
	}

	return a == a_end && b == b_end;
}

static int ftp_listcmd_copy_trimmed(char *dest, size_t dest_size, const char *src)
{
	const char *start;
	const char *end;
	size_t len;

	if (!dest || !src)
		return 0;

	start = ftp_listcmd_skip_padding(src);
	end = ftp_listcmd_trim_end(start);
	len = (size_t)(end - start);

	if (dest_size <= len)
		return 0;

	memcpy(dest, start, len);
	dest[len] = 0;
	return 1;
}

int ftp_listcmd_next_after_failure(const char *current_cmd,
								   const char *default_cmd,
								   char *next_cmd,
								   size_t next_cmd_size)
{
	const char *fallback_default = ftp_listcmd_has_text(default_cmd) ? default_cmd : FTP_LISTCMD_DEFAULT;

	if (ftp_listcmd_equal(current_cmd, FTP_LISTCMD_MLSD))
	{
		if (ftp_listcmd_equal(current_cmd, fallback_default))
			return 0;

		return ftp_listcmd_copy_trimmed(next_cmd, next_cmd_size, fallback_default);
	}

	if (!ftp_listcmd_equal(fallback_default, FTP_LISTCMD_PLAIN) &&
		ftp_listcmd_equal(current_cmd, fallback_default))
		return ftp_listcmd_copy_trimmed(next_cmd, next_cmd_size, FTP_LISTCMD_PLAIN);

	return 0;
}

int ftp_listcmd_result_after_reply(int current_result, int final_reply)
{
	if (current_result)
		return current_result;

	if (final_reply / 100 == 2)
		return 0;

	return -2;
}

int ftp_listcmd_is_mlsd(const char *cmd)
{
	return ftp_listcmd_equal(cmd, FTP_LISTCMD_MLSD);
}
