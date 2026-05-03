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
#include "search.h"

// Search a file (either on disk or in memory)
long LIBFUNC L_SearchFile(REG(a0, APTR file),
						  REG(a1, UBYTE *search_text),
						  REG(d0, ULONG flags),
						  REG(a2, UBYTE *buffer),
						  REG(d1, ULONG buffer_size))
{
	UBYTE match_text[256];
	long match_size;
	long search_len, buf_pos, oldpos;
	search_handle handle;

	// No text?
	if (!search_text || !*search_text)
		return -1;

	// Get length of search string
	search_len = strlen((char *)search_text);

	// Matching hex?
	if (search_text[0] == '$')
	{
		// Convert hex to ASCII string
		for (buf_pos = 1, match_size = 0; buf_pos < search_len; buf_pos += 2, match_size++)
		{
			// Wildcard?
			if (search_text[buf_pos] == '?')
				match_text[match_size] = '?';

			// Else do hex conversion
			else
				match_text[match_size] = (UBYTE)L_Atoh(&search_text[buf_pos], 2);
		}

		// Can't have cast-insensitivity or only words on a hex search
		flags &= ~(SEARCH_NOCASE | SEARCH_ONLYWORDS);
	}

	// Else normal search
	else
	{
		// Go through search text
		for (buf_pos = 0, match_size = 0; buf_pos < search_len; buf_pos++)
		{
			// \ indicates a control sequence
			if (search_text[buf_pos] == '\\')
			{
				// Increment position
				++buf_pos;

				// If followed by another \, match a normal \ character
				if (search_text[buf_pos] == '\\')
					match_text[match_size++] = '\\';

				// Otherwise, indicates a decimal value
				else
				{
					match_text[match_size++] = atoi((char *)search_text + buf_pos);

					// Skip over number
					while (search_text[buf_pos + 1] >= '0' && search_text[buf_pos + 1] <= '9')
						++buf_pos;
				}
			}

			// Otherwise, store character
			else
			{
				// Case insensitive - convert to upper case
				if (flags & SEARCH_NOCASE)
					match_text[match_size++] = toupper(search_text[buf_pos]);

				// Store vertbatim
				else
					match_text[match_size++] = search_text[buf_pos];
			}
		}
	}

	// Zero result settings
	handle.v_search_found_lines = 0;
	handle.v_search_last_line_pos = 0;

	// If we have a buffer, search it and return
	if (buffer)
		return search_buffer(&handle, buffer, buffer_size, match_text, match_size, flags);

	// Allocate buffer to read file into
	if (!(buffer = AllocVec(32004, MEMF_CLEAR)))
		return -1;

	// Remember old file position
	oldpos = L_SeekBuf(file, 0, OFFSET_CURRENT);

	// Loop until aborted, or end of file
	FOREVER
	{
		long size, amount;

		// Get the amount to read
		amount = (buffer_size < 1 || buffer_size > 32000) ? 32000 : buffer_size;

		// Read some data
		if ((size = L_ReadBuf(file, (char *)buffer, amount)) < 1)
			break;

		// Search what we've got
		if (search_buffer(&handle, buffer, size, match_text, match_size, flags) == 1)
		{
			// Free buffer
			FreeVec(buffer);

			// Return match position
			return oldpos + (long)(handle.v_search_found_position - (char *)buffer);
		}

		// Reading limited amount?
		if (buffer_size > 0)
		{
			// Subtract from remaining size
			if ((buffer_size -= amount) < 1)
				break;
		}

		// If end of file, break
		if (size < amount)
			break;

		// Seek back the size of the match (in case we had it spread over the end of the buffer)
		L_SeekBuf(file, -match_size, OFFSET_CURRENT);

		// Adjust oldpos
		oldpos += size - match_size;
	}

	// Free buffer
	FreeVec(buffer);

	// No match
	return -1;
}

// Search a buffer
long search_buffer(search_handle *handle,
				   UBYTE *buffer,
				   ULONG buffer_size,
				   UBYTE *match_buf,
				   ULONG match_size,
				   ULONG flags)
{
	UBYTE match_char, last_char = 0;
	long buf_pos, match_pos = 0, match_start = 0;
	long old_lines, old_pos;

	// Remember old match results
	old_lines = handle->v_search_found_lines;
	old_pos = handle->v_search_last_line_pos;

	// Go through buffer
	for (buf_pos = 0; buf_pos < buffer_size; buf_pos++)
	{
		// Get character
		if (flags & SEARCH_NOCASE)
			match_char = toupper(buffer[buf_pos]);
		else
			match_char = buffer[buf_pos];

		// Is this a newline?
		if (match_char == 10)
		{
			// Increment line count, store position of line
			++handle->v_search_found_lines;
			handle->v_search_last_line_pos = buf_pos + 1;
		}

		// See if this doesn't match
		if (match_buf[match_pos] != match_char && (!(flags & SEARCH_WILDCARD) || match_buf[match_pos] != '?'))
		{
			// Didn't match, go back to where we started from
			if (match_pos > 0)
			{
				// Restore old match information
				buf_pos = match_start;
				handle->v_search_found_lines = old_lines;
				handle->v_search_last_line_pos = old_pos;

				// Reset match information
				match_pos = 0;
				match_start = 0;
			}
		}

		// Matched!
		else
		{
			// Check we satisfy the "only word" requirement (if set)
			if (!(flags & SEARCH_ONLYWORDS) || match_pos || isonlyword(last_char))
			{
				// If this is the start of a match, remember position
				if (match_pos == 0)
				{
					match_start = buf_pos;
					old_lines = handle->v_search_found_lines;
					old_pos = handle->v_search_last_line_pos;
				}

				// Increment match count, see if we've matched the whole thing
				if ((++match_pos) == match_size)
				{
					// Check we satisfy the "only word" requirement (if set)
					if (!(flags & SEARCH_ONLYWORDS) || isonlyword(buffer[buf_pos + 1]))
					{
						// Store match results
						handle->v_search_found_position = (char *)buffer + match_start;
						handle->v_search_found_size = match_pos;

						// Success
						return 1;
					}

					// Failed "only word" test, go back to where we started
					buf_pos = match_start;
					handle->v_search_found_lines = old_lines;
					handle->v_search_last_line_pos = old_pos;

					// Reset match information
					match_pos = 0;
					match_start = 0;
				}
			}
		}

		// Remember last character
		last_char = match_char;
	}

	// Didn't match
	return -1;
}
