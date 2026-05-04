/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

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

#include "ftp.h"
#include "ftp_utf8.h"
#include "ftp_lister.h"

// Use the system codesets.library headers - works on all platforms
// (OS3 m68k, OS4 PPC, MorphOS, AROS)

// Enable variadic inline macros on MorphOS (CodesetsUTF8ToStr, CodesetsUTF8Create, etc.)
#if defined(__MORPHOS__) && !defined(USE_INLINE_STDARG)
#define USE_INLINE_STDARG
#endif

#include <proto/codesets.h>
#include <libraries/codesets.h>

// External declarations - opened/closed in libinit.c
extern struct Library *CodesetsBase;
#ifdef __amigaos4__
extern struct CodesetsIFace *ICodesets;
#endif

// Check if codesets.library is available (was opened successfully)
BOOL ftp_codesets_available(void)
{
#ifdef __amigaos4__
	return (CodesetsBase != NULL && ICodesets != NULL);
#else
	return (CodesetsBase != NULL);
#endif
}

// Check if the FTP server supports UTF-8
BOOL ftp_is_utf8_server(struct ftp_info *info)
{
	return (info && (info->fi_flags & FTP_FEAT_UTF8));
}

// Convert UTF-8 string to local charset (using codesets.library system default).
// Returns allocated string that must be freed with ftp_codesets_free(), or NULL on failure.
// The system default codeset is configured via codesets.library Prefs.
char *ftp_utf8_to_local(const char *utf8_name)
{
	STRPTR result;

	if (!utf8_name || !ftp_codesets_available())
		return NULL;

	// CSA_DestCodeset = NULL: use codesets.library system default.
	// CSA_MapForeignChars = TRUE: map unmappable characters to similar-looking ones.
	result = CodesetsUTF8ToStr(
		CSA_Source, (IPTR)utf8_name,
		CSA_DestCodeset, (IPTR)NULL,
		CSA_MapForeignChars, (IPTR)TRUE,
		TAG_DONE);

	return (char *)result;
}

// Convert local charset string to UTF-8 (using codesets.library system default).
// Returns allocated string that must be freed with ftp_codesets_free(), or NULL on failure.
// The system default codeset is configured via codesets.library Prefs.
char *ftp_local_to_utf8(const char *local_name)
{
	UTF8 *result;

	if (!local_name || !ftp_codesets_available())
		return NULL;

	// CSA_SourceCodeset = NULL: use codesets.library system default.
	result = CodesetsUTF8Create(
		CSA_Source, (IPTR)local_name,
		CSA_SourceCodeset, (IPTR)NULL,
		TAG_DONE);

	return (char *)result;
}

// Convert filename from UTF-8 (server) to local charset (display/filesystem)
// This modifies the entry_info structure in place
void ftp_convert_filename_from_utf8(struct entry_info *entry, struct ftp_info *info)
{
	char *converted_name;

	if (!entry || !info)
		return;

	// Only convert if server supports UTF-8 and codesets is available
	if (!ftp_is_utf8_server(info) || !ftp_codesets_available())
		return;

	converted_name = ftp_utf8_to_local(entry->ei_name);

	if (converted_name)
	{
		// Copy converted name back to entry, ensuring we don't overflow
		stccpy(entry->ei_name, converted_name, sizeof(entry->ei_name));
		ftp_codesets_free(converted_name);
	}
	else
	{
		// Conversion failed while codesets is available (invalid UTF-8 from
		// server, or OOM). Stop treating this session as UTF-8 so raw bytes
		// pass through unchanged in both directions. ei_name keeps the raw
		// UTF-8 bytes it already holds, which the server will accept back
		// verbatim now that conversion is disabled.
		info->fi_flags &= ~FTP_FEAT_UTF8;
	}
}

// Convert a UTF-8 path in-place to the local charset.
// - If the server does not advertise UTF-8 or codesets is unavailable, does
//   nothing (the buffer is left untouched).
// - On conversion failure while codesets is available, clears FTP_FEAT_UTF8
//   for the session so the raw UTF-8 bytes round-trip cleanly in subsequent
//   operations.
void ftp_convert_inplace_utf8_to_local(char *buf, int buf_size, struct ftp_info *info)
{
	char *local_path;

	if (!buf || buf_size <= 0 || !info)
		return;

	if (!ftp_is_utf8_server(info) || !ftp_codesets_available())
		return;

	local_path = ftp_utf8_to_local(buf);
	if (local_path)
	{
		stccpy(buf, local_path, buf_size);
		ftp_codesets_free(local_path);
	}
	else
	{
		// Conversion failed: leave the UTF-8 bytes in place and disable
		// UTF-8 for the session so ftp_convert_path_to_utf8 also becomes a
		// no-op. This keeps bytes consistent in both directions.
		info->fi_flags &= ~FTP_FEAT_UTF8;
	}
}

// Store a server-returned UTF-8 path into a local-charset buffer.
// Wrapper around ftp_convert_inplace_utf8_to_local for the common case of
// copying from a separate UTF-8 source buffer.
void ftp_store_utf8_path_as_local(char *dest, int dest_size, const char *utf8_path, struct ftp_info *info)
{
	if (!dest || dest_size <= 0 || !utf8_path || !info)
		return;

	stccpy(dest, utf8_path, dest_size);
	ftp_convert_inplace_utf8_to_local(dest, dest_size, info);
}

// Convert local path to UTF-8 for sending to server
// Returns allocated string that must be freed with ftp_codesets_free(), or NULL on failure
char *ftp_convert_path_to_utf8(const char *local_path, struct ftp_info *info)
{
	if (!local_path || !info || !ftp_is_utf8_server(info) || !ftp_codesets_available())
		return NULL;

	return ftp_local_to_utf8(local_path);
}

// Free a string allocated by codesets.library functions
void ftp_codesets_free(void *str)
{
	if (!str)
		return;

	if (ftp_codesets_available())
		CodesetsFreeA(str, NULL);
}
