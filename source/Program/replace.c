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

#include "dopus.h"
#include "replace.h"

#define VERSION_BUFFER_SIZE 4096
#define VERSION_BUFFER_OVERLAP 128

#define VERSIONF_GOT_EXE (1 << 0)
#define VERSIONF_GOT_HEADER (1 << 1)
#define VERSIONF_FAIL (1 << 2)
#define VERSIONF_MATCH (1 << 3)

#define VERSION_EXECUTABLE 0x3f3
#define VERSION_HEADER_CODE 0x3e9
#define VERSION_HEADER_DATA 0x3ea
#define VERSION_MATCHWORD 0x4afc

enum {
	TEST_SAME,
	TEST_OLDER,
	TEST_NEWER,
	TEST_BIGGER,
	TEST_SMALLER,
};

static BOOL copy_get_file_version(char *, short *, short *, short *, APTR);
static BOOL copy_scan_file_version(char *, short *, short *, short *);
static BOOL copy_scan_resident_version(char *, short *, short *, short *);
static BOOL copy_parse_version(char *, short, short *, short *, short *);
static BOOL copy_parse_version_text(char *, short, short, short *, short *, short *);
static BOOL copy_parse_version_number(char *, short, short *, short *);
static BOOL copy_is_digit(char);

long SmartAskReplace(struct Window *parent,
					 struct Screen *sparent,
					 IPCData *ipc,
					 BPTR dir_old,
					 struct FileInfoBlock *file_old,
					 BPTR dir_new,
					 struct FileInfoBlock *file_new,
					 long flags)
{
	char *datebuf[2];
	char *buf, *buf2;
	short ret = 0;
	short version[2] = {0, 0}, revision[2] = {0, 0}, patch[2] = {0, 0}, noversion = 0;
	short got_version = 0, can_ver;
	short date_test = TEST_SAME, size_test = TEST_SAME, version_test = TEST_SAME;
	short
	try
		, option = 0;
	char old_name[35], new_name[35];

	// No version?
	if (flags & REPREQF_NOVERSION)
		noversion = 1;

	// Get truncated filenames
	get_trunc_filename(file_old->fib_FileName, old_name);
	get_trunc_filename(file_new->fib_FileName, new_name);

	// Allocate memory
	if (!(buf = AllocVec(768, MEMF_CLEAR)))
		return 0;
	datebuf[0] = buf + 447;
	datebuf[1] = buf + 479;
	buf2 = buf + 512;

	// Build date strings
	date_build_string(&file_new->fib_Date, datebuf[0], 0);
	date_build_string(&file_old->fib_Date, datebuf[1], 0);

	// Allowed to test version?
	can_ver = (environment->env->settings.replace_flags & REPLACEF_CHECK_VERSION && !noversion) ? 1 : 0;

	// Can loop twice
	for (try = 0; try < 2; try ++)
	{
		// Allowed to test versions?
		if (can_ver)
		{
			// Got directory locks?
			if (dir_new && dir_old)
			{
				BPTR dir;
				BOOL ok;
				APTR progress = 0;

				// Change to new directory
				dir = CurrentDir(dir_new);

				// If files are bigger than 50k, open progress window
				if (file_new->fib_Size > 50000 || file_old->fib_Size > 50000)
				{
					// Open progrss window
					progress = OpenProgressWindowTags(PW_Window,
													  parent,
													  PW_Screen,
													  sparent,
													  PW_Title,
													  GetString(&locale, MSG_REPLACE_VERSION_TITLE),
													  PW_Flags,
													  PWF_FILENAME | PWF_FILESIZE | PWF_GRAPH | PWF_ABORT,
													  PW_FileCount,
													  2,
													  TAG_END);

					// Set initial filename
					SetProgressWindowTags(progress, PW_FileName, new_name, PW_FileNum, 1, TAG_END);
				}

				// Get version of new file
				if ((ok = copy_get_file_version(file_new->fib_FileName, &version[0], &revision[0], &patch[0], progress)))
				{
					// Change to old directory
					CurrentDir(dir_old);
					got_version |= 1;

					// Set second filename
					if (progress)
						SetProgressWindowTags(progress, PW_FileName, old_name, PW_FileNum, 2, TAG_END);

					// Get version of old file
					if (copy_get_file_version(file_old->fib_FileName, &version[1], &revision[1], &patch[1], progress))
					{
						// Got versions for both
						got_version |= 2;
					}
				}

				// Restore CD
				CurrentDir(dir);

				// Close progress requester
				CloseProgressWindow(progress);
			}
		}

		// Do date test
		ret = CompareDates(&file_new->fib_Date, &file_old->fib_Date);

		// Get test value
		if (ret < 0)
			date_test = TEST_NEWER;
		else if (ret > 0)
			date_test = TEST_OLDER;

		// Do size test
		if (file_new->fib_Size > file_old->fib_Size)
			size_test = TEST_BIGGER;
		else if (file_new->fib_Size < file_old->fib_Size)
			size_test = TEST_SMALLER;

		// Got versions?
		if (got_version == 3)
		{
			// Do version test
			if (version[0] > version[1])
				version_test = TEST_NEWER;
			else if (version[0] < version[1])
				version_test = TEST_OLDER;

			// Same version, check revision and patch
			else
			{
				if (revision[0] > revision[1])
					version_test = TEST_NEWER;
				else if (revision[0] < revision[1])
					version_test = TEST_OLDER;
				else if (patch[0] > patch[1])
					version_test = TEST_NEWER;
				else if (patch[0] < patch[1])
					version_test = TEST_OLDER;
			}
		}

		// Brief requester? Only if something is different though...
		if (!(environment->env->settings.replace_flags & REPLACEF_VERBOSE_REPLACE) &&
			!(date_test == TEST_SAME && size_test == TEST_SAME && version_test == TEST_SAME))
		{
			char verbuf[2][32];

			// Build version strings
			if (got_version & 1)
				lsprintf(verbuf[0], GetString(&locale, MSG_REPLACE_VER), version[0], revision[0], patch[0]);
			else if (got_version & 2)
				strcpy(verbuf[0], GetString(&locale, MSG_REPLACE_VER_UNKNOWN));
			else
				verbuf[0][0] = 0;

			if (got_version & 2)
				lsprintf(verbuf[1], GetString(&locale, MSG_REPLACE_VER), version[1], revision[1], patch[1]);
			else if (got_version & 1)
				strcpy(verbuf[1], GetString(&locale, MSG_REPLACE_VER_UNKNOWN));
			else
				verbuf[1][0] = 0;

			// Build requester text
			lsprintf(buf,
					 GetString(&locale, MSG_BRIEF_FILE_EXISTS),
					 old_name,
					 file_new->fib_Size,
					 datebuf[0],
					 verbuf[0],
					 file_old->fib_Size,
					 datebuf[1],
					 verbuf[1]);
		}

		// Verbose
		else
		{
			// Build requester text
			lsprintf(buf, GetString(&locale, MSG_FILE_ALREADY_EXISTS), old_name);

			// Everything the same?
			if (date_test == TEST_SAME && size_test == TEST_SAME && version_test == TEST_SAME)
				strcat(buf, GetString(&locale, MSG_REPLACE_SAME));

			// Otherwise
			else
			{
				short len;

				// Same size?
				if (size_test == TEST_SAME)
					lsprintf(buf2, GetString(&locale, MSG_REPLACE_SIZE_SAME), file_old->fib_Size);

				// Bigger?
				else if (size_test == TEST_BIGGER)
					lsprintf(
						buf2, GetString(&locale, MSG_REPLACE_SIZE_BIGGER), file_new->fib_Size - file_old->fib_Size);

				// Must be smaller
				else
					lsprintf(
						buf2, GetString(&locale, MSG_REPLACE_SIZE_SMALLER), file_old->fib_Size - file_new->fib_Size);

				// Add to string
				strcat(buf, buf2);

				// Same date?
				if (date_test == TEST_SAME)
					lsprintf(buf2, GetString(&locale, MSG_REPLACE_DATE_SAME), datebuf[1]);

				// Newer?
				else if (date_test == TEST_NEWER)
					lsprintf(buf2, GetString(&locale, MSG_REPLACE_DATE_NEWER), datebuf[0]);

				// Must be older
				else
					lsprintf(buf2, GetString(&locale, MSG_REPLACE_DATE_OLDER), datebuf[0]);

				// Add to string
				strcat(buf, buf2);

				// Got version info?
				if (got_version == 3)
				{
					// Same version?
					if (version_test == TEST_SAME)
						lsprintf(buf2,
								 GetString(&locale, MSG_REPLACE_VERSION_SAME),
								 version[0],
								 revision[0],
								 patch[0]);

					// Newer version?
					else if (version_test == TEST_NEWER)
						lsprintf(buf2,
								 GetString(&locale, MSG_REPLACE_VERSION_NEWER),
								 version[0],
								 revision[0],
								 patch[0]);

					// Must be older
					else
						lsprintf(buf2,
								 GetString(&locale, MSG_REPLACE_VERSION_OLDER),
								 version[0],
								 revision[0],
								 patch[0]);

					// Add to string
					strcat(buf, buf2);
				}

				// Take off last newline
				if (buf[(len = strlen(buf) - 1)] == '\n')
					buf[len] = 0;
			}
		}

		// Display requester
		ret = AsyncRequestTags(ipc,
							   REQTYPE_SIMPLE,
							   parent,
							   0,
							   0,
							   AR_Window,
							   parent,
							   AR_Screen,
							   sparent,
							   AR_Message,
							   buf,
							   AR_Button,
							   GetString(&locale, MSG_REPLACE),
							   AR_Button,
							   GetString(&locale, MSG_REPLACE_ALL),
							   AR_Button,
							   GetString(&locale, MSG_SKIP),
							   AR_Button,
							   GetString(&locale, MSG_SKIP_ALL),
							   (!can_ver && !noversion) ? AR_Button : TAG_IGNORE,
							   GetString(&locale, MSG_GET_VERSION),
							   AR_Button,
							   GetString(&locale, MSG_ABORT),
							   TAG_END);

		// Re-test for version?
		if (ret == REPLACE_GAD_VERSION)
			can_ver = 1;
		else
			break;
	}

	// Free buffer
	FreeVec(buf);

	// Get return value
	if (ret == REPLACE_GAD_ABORT)
		option = REPLACE_ABORT;
	else if (ret == REPLACE_GAD_SKIP)
		option = REPLACE_LEAVE;
	else if (ret == REPLACE_GAD_SKIP_ALL)
		option = REPLACE_LEAVE | REPLACE_ALL;
	else if (ret == REPLACE_GAD_REPLACE)
		option = REPLACE_REPLACE;
	else if (ret == REPLACE_GAD_REPLACE_ALL)
		option = REPLACE_REPLACE | REPLACE_ALL;

	return option;
}

static BOOL copy_get_file_version(char *name, short *version, short *revision, short *patch, APTR progress)
{
	short scan_version = 0, scan_revision = 0, scan_patch = 0;
	BOOL got_version;

	*version = 0;
	*revision = 0;
	*patch = 0;

	got_version = GetFileVersion(name, version, revision, 0, progress);

	if (copy_scan_file_version(name, &scan_version, &scan_revision, &scan_patch) &&
		(!got_version || (*version == 0 && *revision == 0) ||
		 (scan_version == *version && scan_revision == *revision)))
	{
		*version = scan_version;
		*revision = scan_revision;
		*patch = scan_patch;
		return 1;
	}

	if (copy_scan_resident_version(name, &scan_version, &scan_revision, &scan_patch) &&
		(!got_version || (*version == 0 && *revision == 0) ||
		 (scan_version == *version && scan_revision == *revision)))
	{
		*version = scan_version;
		*revision = scan_revision;
		*patch = scan_patch;
		return 1;
	}

	return got_version;
}

static BOOL copy_scan_file_version(char *name, short *version, short *revision, short *patch)
{
	char *buffer;
	BPTR file;
	short size, scan_size, carry, pos;
	BOOL ok = 0;

	*version = 0;
	*revision = 0;
	*patch = 0;

	if (!(buffer = AllocVec(VERSION_BUFFER_SIZE + VERSION_BUFFER_OVERLAP, 0)))
		return 0;

	if (!(file = Open(name, MODE_OLDFILE)))
	{
		FreeVec(buffer);
		return 0;
	}

	carry = 0;
	while ((size = Read(file, buffer + carry, VERSION_BUFFER_SIZE)) > -1)
	{
		if (size == 0)
			break;

		scan_size = carry + size;

		if (copy_parse_version(buffer, scan_size, version, revision, patch))
		{
			ok = 1;
			break;
		}

		carry = (scan_size > VERSION_BUFFER_OVERLAP) ? VERSION_BUFFER_OVERLAP : scan_size;
		for (pos = 0; pos < carry; pos++)
			buffer[pos] = buffer[scan_size - carry + pos];
	}

	Close(file);
	FreeVec(buffer);

	return ok;
}

static BOOL copy_scan_resident_version(char *name, short *version, short *revision, short *patch)
{
	char *buffer;
	BPTR file;
	short size, pos;
	BOOL ok = 0;
	unsigned short word_read = 0, wordpos = 0;
	unsigned short verflags = 0;
	unsigned long wordcount = 0, headerstart = 0, matchpos = 0, headerlength = 0, limit;

	*version = 0;
	*revision = 0;
	*patch = 0;

	if (!(buffer = AllocVec(VERSION_BUFFER_SIZE, 0)))
		return 0;

	if (!(file = Open(name, MODE_OLDFILE)))
	{
		FreeVec(buffer);
		return 0;
	}

	limit = 256;

	while ((size = Read(file, buffer, VERSION_BUFFER_SIZE)) > -1)
	{
		short extra = 0;

		if (size == 0)
		{
			if (verflags & VERSIONF_MATCH)
			{
				Seek(file, matchpos, OFFSET_BEGINNING);
				size = Read(file, buffer, 512);
				if (size > 1)
					ok = copy_parse_version_text(buffer, size, 1, version, revision, patch);
			}
			break;
		}

		if (size > VERSION_BUFFER_SIZE - 32)
		{
			extra = size - (VERSION_BUFFER_SIZE - 32);
			size = VERSION_BUFFER_SIZE - 32;
		}

		for (pos = 0; pos < size && !(verflags & VERSIONF_MATCH); pos++)
		{
			if (wordpos == 0)
			{
				word_read = ((unsigned char)buffer[pos]) << 8;
				wordpos = 1;
			}
			else if (!(verflags & VERSIONF_FAIL))
			{
				word_read |= (unsigned char)buffer[pos];
				wordpos = 0;
				++wordcount;

				if (word_read == VERSION_EXECUTABLE && wordcount == 2)
					verflags |= VERSIONF_GOT_EXE;
				else if (verflags & VERSIONF_GOT_EXE &&
						 (word_read == VERSION_HEADER_CODE || word_read == VERSION_HEADER_DATA) && !headerstart)
				{
					verflags |= VERSIONF_GOT_HEADER;
					headerstart = ((wordcount - 1) << 1) + 6;
					CopyMem((char *)(buffer + pos + 1), (char *)&headerlength, sizeof(headerlength));
					++headerlength;
					headerlength <<= 1;
				}
				else if (verflags & VERSIONF_GOT_EXE && verflags & VERSIONF_GOT_HEADER &&
						 word_read == VERSION_MATCHWORD)
				{
					struct Resident res;

					CopyMem((char *)(buffer + pos - 1), (char *)&res, sizeof(struct Resident));
					matchpos = ((long)res.rt_IdString) + headerstart - 1;
					verflags |= VERSIONF_MATCH | VERSIONF_FAIL;
				}
				else if (headerstart)
				{
					if ((--headerlength) <= 0)
					{
						verflags &= ~VERSIONF_GOT_HEADER;
						headerstart = 0;
						limit = wordcount + 256;
					}
				}

				if (wordcount > limit && !(verflags & VERSIONF_GOT_HEADER))
					verflags |= VERSIONF_FAIL;
			}
		}

		if (verflags & VERSIONF_MATCH)
		{
			Seek(file, matchpos, OFFSET_BEGINNING);
			size = Read(file, buffer, 512);
			if (size > 1)
				ok = copy_parse_version_text(buffer, size, 1, version, revision, patch);
			break;
		}

		if (extra > 0)
			Seek(file, -extra, OFFSET_CURRENT);
	}

	Close(file);
	FreeVec(buffer);

	return ok;
}

static BOOL copy_parse_version(char *buffer, short size, short *version, short *revision, short *patch)
{
	short pos;

	for (pos = 0; pos <= size - 5; pos++)
	{
		short scan;

		if (buffer[pos] != '$' || buffer[pos + 1] != 'V' || buffer[pos + 2] != 'E' || buffer[pos + 3] != 'R')
			continue;

		if (buffer[pos + 4] == ':')
			scan = pos + 5;
		else if (buffer[pos + 4] == ' ' || buffer[pos + 4] == '\t')
			scan = pos + 4;
		else
			continue;

		if (copy_parse_version_text(buffer, size, scan, version, revision, patch))
			return 1;
	}

	return 0;
}

static BOOL copy_parse_version_text(char *buffer, short size, short scan, short *version, short *revision, short *patch)
{
	short ver = 0, rev = 0, pat = 0;

	while (scan < size && (buffer[scan] == ' ' || buffer[scan] == '\t'))
		++scan;
	if (scan >= size)
		return 0;

	while (scan < size && buffer[scan] != ' ' && buffer[scan] != '\t' && buffer[scan] != '\n')
		++scan;
	if (scan >= size || buffer[scan] == '\n')
		return 0;

	while (scan < size && !copy_is_digit(buffer[scan]))
		++scan;
	if (scan >= size || !copy_parse_version_number(buffer, size, &scan, &ver))
		return 0;

	while (scan < size && buffer[scan] != '.' && buffer[scan] != ' ' && buffer[scan] != '\n')
		++scan;
	if (scan >= size)
		return 0;

	if (buffer[scan] == '.')
	{
		++scan;
		if (scan >= size)
			return 0;

		if (copy_is_digit(buffer[scan]))
			copy_parse_version_number(buffer, size, &scan, &rev);

		if (scan < size && buffer[scan] == '.')
		{
			++scan;
			if (scan >= size)
				return 0;

			if (copy_is_digit(buffer[scan]))
				copy_parse_version_number(buffer, size, &scan, &pat);
		}
	}
	else if (buffer[scan] == ' ')
	{
		++scan;
		if (scan >= size)
			return 0;

		if (copy_is_digit(buffer[scan]))
			copy_parse_version_number(buffer, size, &scan, &rev);
	}

	*version = ver;
	*revision = rev;
	*patch = pat;
	return 1;
}

static BOOL copy_parse_version_number(char *buffer, short size, short *pos, short *value)
{
	long number = 0;
	BOOL got = 0;

	while (*pos < size && copy_is_digit(buffer[*pos]))
	{
		got = 1;
		number = (number * 10) + buffer[*pos] - '0';
		++(*pos);
	}

	*value = number;
	return got;
}

static BOOL copy_is_digit(char c)
{
	return (c >= '0' && c <= '9');
}
