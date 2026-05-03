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

#include <stdlib.h>

#include "dopuslib.h"

#define VERF_GOT_EXE (1 << 0)
#define VERF_GOT_HEADER (1 << 1)
#define VERF_FAIL (1 << 2)
#define VERF_MATCH (1 << 3)

#define VER_EXECUTABLE 0x3f3
#define VER_HEADER_CODE 0x3e9
#define VER_HEADER_DATA 0x3ea
#define VER_MATCHWORD 0x4afc

short version_skip(char *, short, short);

static char *month_str[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

BOOL LIBFUNC L_GetFileVersion(REG(a0, char *name),
							  REG(d0, short *version),
							  REG(d1, short *revision),
							  REG(a1, struct DateStamp *date),
							  REG(a2, APTR progress))
{
	char *buffer;
	BPTR file;
	short size, pos;
	char match_string[6];
	short match = 0;
	BOOL ok = 0;
	struct TagItem tags[2];
	unsigned short word_read = 0, wordpos = 0;
	unsigned short verflags = 0;
	unsigned long wordcount = 0, headerstart = 0, matchpos = 0, headerlength = 0, limit;

	// Initialise things
	*version = 0;
	*revision = 0;
	if (date)
		date->ds_Days = 0;

	// Allocate buffer
	if (!(buffer = AllocVec(4096, 0)))
		return 0;

	// Open file
	if (!(file = Open(name, MODE_OLDFILE)))
	{
		FreeVec(buffer);
		return 0;
	}

	// Initialise tags
	tags[0].ti_Tag = PW_FileSize;
	tags[1].ti_Tag = TAG_END;

	// Got progress?
	if (progress)
	{
		D_S(struct FileInfoBlock, info)

		// Get file info
		if (ExamineFH(file, info))
		{
			// Set file size
			tags[0].ti_Data = info->fib_Size;
		}
		else
			tags[0].ti_Data = 0;

		// Set bar
		L_SetProgressWindow(progress, tags);

		// No file size?
		if (!tags[0].ti_Data)
			progress = 0;
	}

	// Set tag to update size
	tags[0].ti_Tag = PW_FileDone;
	tags[0].ti_Data = 0;

	// Build match string
	match_string[0] = '$';
	match_string[1] = 'V';
	match_string[2] = 'E';
	match_string[3] = 'R';

	// Wordcount limit
	limit = 256;

	// Read first 4k
	while ((size = Read(file, buffer, 4096)) > -1)
	{
		short extra = 0;

		// End of file?
		if (size == 0)
		{
			// Did we find matchword?
			if (verflags & VERF_MATCH)
			{
				// Seek to match position
				Seek(file, matchpos, OFFSET_BEGINNING);

				// Read into buffer
				size = Read(file, buffer, 512);

				// Signal a match
				if (size > 1)
				{
					match = 5;
					pos = 1;
					verflags &= ~VERF_MATCH;
				}
				else
					break;
			}

			// Failed to find anything
			else
				break;
		}

		// Got progress?
		if (progress)
		{
			// Check for abort
			if (L_CheckProgressAbort(progress))
				break;

			// Increment total, update display
			tags[0].ti_Data += size;
			L_SetProgressWindow(progress, tags);
		}

		// Leave last 32 bytes
		if (size > 4064)
		{
			extra = size - 4064;
			size = 4064;
		}

		// Go through buffer
		for (pos = 0; pos < size && match < 5; pos++)
		{
			// Add to word
			if (wordpos == 0)
			{
				// High byte
				word_read = ((unsigned char)buffer[pos]) << 8;
				wordpos = 1;
			}

			// Low byte
			else if (!(verflags & VERF_FAIL))
			{
				// This completes the word
				word_read |= (unsigned char)buffer[pos];
				wordpos = 0;

				// Increment count
				++wordcount;

				// Executable?
				if (word_read == VER_EXECUTABLE && wordcount == 2)
				{
					verflags |= VERF_GOT_EXE;
				}

				// Correct header?
				else if (verflags & VERF_GOT_EXE && (word_read == VER_HEADER_CODE || word_read == VER_HEADER_DATA) &&
						 !headerstart)
				{
					// Set flag
					verflags |= VERF_GOT_HEADER;
					headerstart = ((wordcount - 1) << 1) + 6;

					// Get length of the header
					CopyMem((char *)(buffer + pos + 1), (char *)&headerlength, sizeof(headerlength));

					// Convert length to words
					++headerlength;
					headerlength <<= 1;
				}

				// Match word?
				else if (verflags & VERF_GOT_EXE && verflags & VERF_GOT_HEADER && word_read == VER_MATCHWORD)
				{
					struct Resident res;

					// Get resident structure
					CopyMem((char *)(buffer + pos - 1), (char *)&res, sizeof(struct Resident));

					// Get position of ID string from hunk offset
					matchpos = ((long)res.rt_IdString) + headerstart - 1;

					// Set match flag
					verflags |= VERF_MATCH | VERF_FAIL;
				}

				// Got a header?
				else if (headerstart)
				{
					// Decrement a word from the headerlength
					if ((--headerlength) <= 0)
					{
						// End of the header
						verflags &= ~VERF_GOT_HEADER;
						headerstart = 0;

						// New word count limit
						limit = wordcount + 256;
					}
				}

				// Have we gone too far?
				if (wordcount > limit && !(verflags & VERF_GOT_HEADER))
					verflags |= VERF_FAIL;
			}

			// Check for match
			if (match < 4 && buffer[pos] == match_string[match])
			{
				// Increment match count
				++match;
			}

			// $VER strings may use either "$VER:" or "$VER "
			else if (match == 4 && (buffer[pos] == ':' || buffer[pos] == ' ' || buffer[pos] == '\t'))
			{
				// Increment match count
				++match;
			}

			// Reset match count
			else
				match = (buffer[pos] == match_string[0]) ? 1 : 0;
		}

		// Got a match?
		if (match == 5)
		{
			short day, month, year;
			char datebuf[20];
			struct DateTime dt;

			// Got progress?
			if (progress)
			{
				// Show full bar
				tags[0].ti_Data = (ULONG)-1;
				L_SetProgressWindow(progress, tags);
			}

			if (pos >= size)
				break;

			// On a space?
			if (buffer[pos] == ' ' || buffer[pos] == '\t')
			{
				// Find first non-space
				while (pos < size && (buffer[pos] == ' ' || buffer[pos] == '\t'))
					++pos;
				if (pos >= size)
					break;
			}

			// Find first space
			while (pos < size && buffer[pos] != ' ' && buffer[pos] != '\t' && buffer[pos] != '\n')
				++pos;
			if (pos >= size || buffer[pos] == '\n')
				break;

			// Find first number
			while (pos < size && (buffer[pos] < '0' || buffer[pos] > '9'))
				++pos;
			if (pos >= size)
				break;

			// Get version number
			*version = atoi(buffer + pos);

			// Find decimal point
			while (pos < size && buffer[pos] != '.' && buffer[pos] != ' ' && buffer[pos] != '\t')
				++pos;
			if (pos >= size - 1)
				break;

			// Check next number
			++pos;
			if (buffer[pos] >= '0' && buffer[pos] <= '9')
				*revision = atoi(buffer + pos);

			// We're ok at this point
			ok = 1;

			// Want date?
			if (!date)
				break;

			// Find parentheses for date
			while (pos < size && buffer[pos] != '(')
				++pos;
			if (pos >= size)
				break;

			// Get day
			++pos;
			day = atoi(buffer + pos);
			if (day < 1 || day > 31)
				break;

			// Skip a segment
			if ((pos = version_skip(buffer, pos, size)) == -1)
				break;

			// Get month
			month = atoi(buffer + pos);
			if (month < 1 || month > 12)
			{
				char buf[4];

				// Maybe this is a character month
				buf[0] = buffer[pos];
				buf[1] = buffer[pos + 1];
				buf[2] = buffer[pos + 2];
				buf[3] = 0;

				// See if it matches
				for (month = 0; month < 12; month++)
					if (stricmp(month_str[month], buf) == 0)
						break;

				// Did it match?
				if (month < 12)
					++month;
				else
					break;
			}

			// Skip a segment
			if ((pos = version_skip(buffer, pos, size)) == -1)
				break;

			// Get year
			year = atoi(buffer + pos);

			// Check year
			if (year > 1900 && year < 2000)
				year -= 1900;
			if (year < 78)
				break;
			while (year > 100)
				year -= 100;

			// Build string
			lsprintf(datebuf, "%ld-%ld-%ld", month, day, year);

			// Convert to datestamp
			dt.dat_Format = FORMAT_USA;
			dt.dat_Flags = 0;
			dt.dat_StrDay = 0;
			dt.dat_StrDate = datebuf;
			dt.dat_StrTime = "00:00:00";
			StrToDate(&dt);

			// Copy datestamp
			*date = dt.dat_Stamp;
			break;
		}

		// Any extra?
		if (extra > 0)
		{
			// Seek back for extra
			Seek(file, -extra, OFFSET_CURRENT);
		}
	}

	// Close file, free buffer
	Close(file);
	FreeVec(buffer);

	return ok;
}

// Skip a segment
short version_skip(char *buffer, short pos, short size)
{
	// Find non-segment
	while (pos < size && ((buffer[pos] >= '0' && buffer[pos] <= '9') || (buffer[pos] >= 'a' && buffer[pos] <= 'z') ||
						  (buffer[pos] >= 'A' && buffer[pos] <= 'Z')))
		++pos;
	if (pos >= size)
		return -1;

	// Find segment
	while (pos < size && !((buffer[pos] >= '0' && buffer[pos] <= '9') || (buffer[pos] >= 'a' && buffer[pos] <= 'z') ||
						   (buffer[pos] >= 'A' && buffer[pos] <= 'Z')))
		++pos;
	if (pos >= size)
		return -1;

	return pos;
}
