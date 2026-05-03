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

#include "dopuslib.h"
#include <Program/dopus_config.h>

#include <proto/music.h>

// Allocate a MatchHandle for a file
APTR LIBFUNC L_GetMatchHandle(REG(a0, char *name), REG(a6, struct MyLibrary *lib))
{
	MatchHandle *handle;

#if defined(__amigaos4__) || defined(__AROS__)
	lib = dopuslibbase_global;
#endif

	// Invalid name?
	if (!name || !*name)
		return 0;

	// Allocate handle
	if ((handle = AllocVec(sizeof(MatchHandle), MEMF_CLEAR)))
	{
		ULONG longs[3];

		// Store filename pointer
		handle->name = name;

		// Lock file
		if (!(handle->lock = Lock(handle->name, ACCESS_READ)))
		{
			FreeVec(handle);
			return 0;
		}

		// Get some file information
		Examine(handle->lock, &handle->fib);

		// Disk path?
		if (handle->name[strlen(handle->name) - 1] == ':')
		{
			// Set disk flag
			handle->flags |= MATCHF_IS_DISK;

			// Get some disk information
			Info(handle->lock, &handle->info);

			// Get disk type
			switch (handle->info.id_DiskType)
			{
			// Special sort
			case ID_UNREADABLE_DISK:
			case ID_NOT_REALLY_DOS:
			case ID_KICKSTART_DISK:
				handle->disk_type = handle->info.id_DiskType;
				break;

			// Other
			default:

				// Ram disk?
				if (strnicmp(handle->name, "ram:", 4) == 0)
					handle->disk_type = MAKE_ID('R', 'A', 'M', 0);

				// Other
				else
				{
					struct DosList *doslist;

					// Get volume node pointer
					doslist = (struct DosList *)BADDR(handle->info.id_VolumeNode);

					// Get disk type from DOS list if it's set
					if (doslist)
						handle->disk_type = doslist->dol_misc.dol_volume.dol_DiskType;

					// If it's not set, get it from info
					if (!handle->disk_type)
						handle->disk_type = handle->info.id_DiskType;
				}
				break;
			}
		}

		// Get full name
#if 1
		{
			LONG error = 0;
			LONG size;

			for (size = 256; handle->fullname == NULL && error == 0; size += 256)
			{
				handle->fullname = AllocVec(size, MEMF_CLEAR);
				if (handle->fullname)
				{
					if (!NameFromLock(handle->lock, handle->fullname, size))
					{
						error = IoErr();
						if (error == ERROR_LINE_TOO_LONG)
						{
							error = 0;
							FreeVec(handle->fullname);
							handle->fullname = NULL;
						}
					}
				}
				else
					error = IoErr();
			}

			if (!handle->fullname)
			{
				L_FreeMatchHandle(handle);
				return 0;
			}
		}
#else
		NameFromLock(handle->lock, handle->fullname, 256);
#endif

		// Is it a file?
		if (handle->fib.fib_DirEntryType < 0)
		{
			// Make sure it isn't read-protected
			if (handle->fib.fib_Protection & FIBF_READ)
			{
				// Can't do anything with it
				L_FreeMatchHandle(handle);
				return 0;
			}

			// Open file
			if (!(handle->file = L_OpenBuf(handle->name, MODE_OLDFILE, 512)))
			{
				// Failed
				L_FreeMatchHandle(handle);
				return 0;
			}

			// Read first three longwords
			L_ReadBuf(handle->file, (char *)longs, sizeof(ULONG) * 3);
#ifdef __AROS__
			longs[0] = AROS_BE2LONG(longs[0]);
			longs[1] = AROS_BE2LONG(longs[1]);
			longs[2] = AROS_BE2LONG(longs[2]);
#endif
			L_SeekBuf(handle->file, 0, OFFSET_BEGINNING);

			// Store IFF form if appropriate
			if (longs[0] == ID_FORM)
				handle->iff_form = longs[2];

			// Executable?
			else
#ifdef __AROS__
				if (longs[0] == 0x7f454c46)
#else
				if ((longs[0] == 0x000003f3) || (longs[0] == 0x7f454c46))
#endif
				handle->flags |= MATCHF_EXECUTABLE;
		}
	}

	return handle;
}

// Free a MatchHandle
void LIBFUNC L_FreeMatchHandle(REG(a0, MatchHandle *handle))
{
	// Valid handle?
	if (handle)
	{
		FreeVec(handle->fullname);

		// Close file
		L_CloseBuf(handle->file);

		// Unlock file
		UnLock(handle->lock);

		// Free datatype
		if (handle->datatype)
			ReleaseDataType(handle->datatype);

		// Free handle
		FreeVec(handle);
	}
}

// See if a file matches a particular type
BOOL LIBFUNC L_MatchFiletype(REG(a0, MatchHandle *handle), REG(a1, Cfg_Filetype *type), REG(a6, struct MyLibrary *lib))
{
	unsigned long recog_pos;
	short buffer_pos = 0;
	unsigned char operation = FTOP_NOOP;
	BOOL match_okay = 0, test_flag, dir_flag = 0, disk_flag = 0;
	long last_position;
	ULONG res;

#if defined(__amigaos4__) || defined(__AROS__)
	lib = dopuslibbase_global;
#endif

	// See if this is in the cache
	if ((res = FindFiletypeCache(handle, type, (struct LibData *)lib->ml_UserData)) != (ULONG)-1)
		return (BOOL)res;

	// If no valid filetype or recognition string, return
	if (!type || !type->recognition)
		return 0;

	// Invalid recognition string?
	if (!*type->recognition)
		return 0;

	// Seek to beginning of file
	if (handle->file)
		L_SeekBuf(handle->file, 0, OFFSET_BEGINNING);
	last_position = 0;
	handle->last_chunk = 0;

	// Process recognition string
	for (recog_pos = 0;; recog_pos++)
	{
		// If no valid operation, see if this is one
		if (operation == FTOP_NOOP)
		{
			if (type->recognition[recog_pos] > FTOP_NOOP && type->recognition[recog_pos] < FTOP_LAST)
			{
				// Store operation
				operation = type->recognition[recog_pos];

				// Clear first 4 buffer characters
				handle->buffer[0] = 0;
				handle->buffer[1] = 0;
				handle->buffer[2] = 0;
				handle->buffer[3] = 0;
			}
		}

		// Or, see if buffer is full or end of a "string"
		else if (buffer_pos == 1023 || type->recognition[recog_pos] > FTOP_SPECIAL || type->recognition[recog_pos] == 0)
		{
			// Null-terminate buffer
			handle->buffer[buffer_pos] = 0;

			// Clause starts as being successful
			match_okay = 1;

			// Indicates if operation was a "test" or a "move"
			test_flag = 1;

			// Look at operation
			switch (operation)
			{
			// Match characters
			case FTOP_MATCH:

				// See if characters match
				if (!handle->file)
					match_okay = 0;
				else
					match_okay = filetype_match_chars(handle, 0);
				break;

			// Match characters case insensitive
			case FTOP_MATCHNOCASE:

				// See if characters match
				if (!handle->file)
					match_okay = 0;
				else
					match_okay = filetype_match_chars(handle, 1);
				break;

			// Match filename
			case FTOP_MATCHNAME:

				// Parse pattern
				ParsePatternNoCase((STRPTR)handle->buffer, (STRPTR)handle->wild_buf, 258);

				// See if filename matches pattern
				match_okay = MatchPatternNoCase((STRPTR)handle->wild_buf, handle->fib.fib_FileName);
				break;

			// Match protection
			case FTOP_MATCHBITS: {
				ULONG prot[2], match_prot;

				// Get protection values
				parse_prot_string(handle->buffer, prot);

				// Get file's protection value
				match_prot = ((~handle->fib.fib_Protection) & 15) + (handle->fib.fib_Protection & ~15);

				// See if protection matches
				match_okay = ((match_prot & prot[0]) == prot[0] && ((match_prot & ~prot[0]) & prot[1]) == 0);
			}
			break;

			// Match comment
			case FTOP_MATCHCOMMENT:

				// Parse pattern
				ParsePatternNoCase((STRPTR)handle->buffer, (STRPTR)handle->wild_buf, 258);

				// See if comment matches pattern
				match_okay = MatchPatternNoCase((STRPTR)handle->wild_buf, (STRPTR)handle->fib.fib_Comment);
				break;

			// Match size
			case FTOP_MATCHSIZE:

				if (handle->file)
				{
					unsigned long size = 0;
					short match_type = MATCH_INVALID;
					short pos;

					// Go through buffer
					for (pos = 0; handle->buffer[pos]; pos++)
					{
						// No match type set yet
						if (match_type == MATCH_INVALID && !(isspace(handle->buffer[pos])))
						{
							// Get match type
							if (handle->buffer[pos] == '<')
								match_type = MATCH_LESS;
							else if (handle->buffer[pos] == '=')
								match_type = MATCH_EQUAL;
							else if (handle->buffer[pos] == '>')
								match_type = MATCH_GREATER;
							else
							{
								match_okay = 0;
								break;
							}
						}

						// Or, this is the actual size
						else if (match_type != MATCH_INVALID && is_digit(handle->buffer[pos]))
						{
							size = atoi((char *)&handle->buffer[pos]);
							break;
						}
					}

					// Valid match type?
					if (match_type != MATCH_INVALID)
					{
						// Less than
						if (match_type == MATCH_LESS)
							match_okay = (handle->fib.fib_Size < size);

						// Equal to
						else if (match_type == MATCH_EQUAL)
							match_okay = (handle->fib.fib_Size == size);

						// Greater than
						else
							match_okay = (handle->fib.fib_Size > size);
					}
				}
				else
					match_okay = 0;
				break;

			// Match date
			case FTOP_MATCHDATE: {
				struct DateStamp date[2];

				// Get datestamps to match
				parse_date_string((char *)handle->buffer, date);

				// See if datestamp matches
				match_okay = !(CompareDates(&handle->fib.fib_Date, &date[0]) > 0 ||
							   CompareDates(&date[1], &handle->fib.fib_Date) > 0);
			}
			break;

			// Move to
			case FTOP_MOVETO:

				if (handle->file)
				{
					long val;
					short err = -1;

					// Clear "test" flag
					test_flag = 0;

					// Get value.. hex or decimal
					if (handle->buffer[0] == '$')
						val = L_Atoh(handle->buffer + 1, -1);
					else
						val = atoi((char *)handle->buffer);

					// Seek to end?
					if (val == -1)
						err = L_SeekBuf(handle->file, 0, OFFSET_END);

					// Seek to absolute position
					else if (val > -1)
						err = L_SeekBuf(handle->file, val, OFFSET_BEGINNING);

					// Fail?
					match_okay = (err != -1);
				}
				else
					match_okay = 0;
				break;

			// Move (relative)
			case FTOP_MOVE:

				if (handle->file)
				{
					long val;

					// Clear "test" flag
					test_flag = 0;

					// Get value.. hex or decimal
					if (handle->buffer[0] == '$')
						val = L_Atoh(handle->buffer + 1, -1);
					else
						val = atoi((char *)handle->buffer);

					// Attempt seek
					match_okay = ((L_SeekBuf(handle->file, val, OFFSET_CURRENT)) != -1);
				}
				else
					match_okay = 0;
				break;

			// Search for text
			case FTOP_SEARCHFOR:
			case FTOP_SEARCHRANGE:

				if (handle->file)
				{
					long old_pos, val, range = 0;
					char *ptr;

					// Store old position in file
					old_pos = L_SeekBuf(handle->file, 0, OFFSET_CURRENT);

					// Pointer to search text
					ptr = (char *)handle->buffer;

					// Range?
					if (operation == FTOP_SEARCHRANGE)
					{
						// Get the range
						range = atoi((char *)handle->buffer);

						// Bump pointer
						if ((ptr = strchr((char *)handle->buffer, ' ')))
							++ptr;
					}

					// Do the search
					val = L_SearchFile(handle->file, (UBYTE *)ptr, SEARCH_NOCASE | SEARCH_WILDCARD, 0, range);

					// Failed?
					if (val == -1)
					{
						match_okay = 0;
						L_SeekBuf(handle->file, old_pos, OFFSET_BEGINNING);
					}

					// Otherwise, seek to new position in file
					else
						L_SeekBuf(handle->file, val, OFFSET_BEGINNING);
				}
				else
					match_okay = 0;
				break;

			// Match IFF form
			case FTOP_MATCHFORM:

				if (handle->file)
				{
					ULONG match_form;

					// Build form to match
					match_form = (ULONG)(handle->buffer[0] << 24) | (handle->buffer[1] << 16) |
								 (handle->buffer[2] << 8) | handle->buffer[3];

					// Does it match?
					if (!handle->iff_form || handle->iff_form != match_form)
						match_okay = 0;
				}
				else
					match_okay = 0;
				break;

			// Find an IFF chunk
			case FTOP_FINDCHUNK:
			case FTOP_MATCHCHUNK:

				// Is this an IFF file?
				if (!handle->iff_form)
					match_okay = 0;

				// Yes it is
				else if (handle->file)
				{
					ULONG longs[2];
					ULONG match_chunk;
					ULONG old_pos;

					// Build chunk to match
					match_chunk = (ULONG)(handle->buffer[0] << 24) | (handle->buffer[1] << 16) |
								  (handle->buffer[2] << 8) | handle->buffer[3];

					// Store old position
					old_pos = L_SeekBuf(handle->file, 0, OFFSET_CURRENT);

					// Haven't found a chunk yet?
					if (!handle->last_chunk)
					{
						// Seek to start of first chunk
						L_SeekBuf(handle->file, sizeof(ULONG) * 3, OFFSET_BEGINNING);
					}

					// Otherwise, start from chunk after current one
					else
					{
						ULONG size;

						// Seek to last chunk position
						L_SeekBuf(handle->file, handle->last_chunk, OFFSET_BEGINNING);

						// Seek to skip over chunk
						size = sizeof(ULONG) * 2 + handle->chunk_size;
						if (size % 2)
							++size;
						L_SeekBuf(handle->file, size, OFFSET_CURRENT);
					}

					// Loop until failure or end of file
					while (1)
					{
						// Read next chunk
						if ((L_ReadBuf(handle->file, (char *)longs, sizeof(ULONG) * 2)) < sizeof(ULONG) * 2)
						{
							// Seek back to original position
							L_SeekBuf(handle->file, old_pos, OFFSET_BEGINNING);
							match_okay = 0;
							break;
						}

#ifdef __AROS__
						longs[0] = AROS_BE2LONG(longs[0]);
						longs[1] = AROS_BE2LONG(longs[1]);
#endif

						// Is this the right chunk?
						// else
						if (longs[0] == match_chunk)
						{
							// Seek to start of chunk
							L_SeekBuf(handle->file, -sizeof(ULONG) * 2, OFFSET_CURRENT);

							// Remember position of this chunk
							handle->last_chunk = L_SeekBuf(handle->file, 0, OFFSET_CURRENT);
							handle->chunk_size = longs[1];
							break;
						}

						// Match chunk only searches one chunk
						if (operation == FTOP_MATCHCHUNK)
						{
							// Seek back to original position
							L_SeekBuf(handle->file, old_pos, OFFSET_BEGINNING);
							match_okay = 0;
							break;
						}

						// Seek to skip over chunk
						if (longs[1] % 2)
							++longs[1];
						L_SeekBuf(handle->file, longs[1], OFFSET_CURRENT);
					}
				}
				else
					match_okay = 0;
				break;

			// Match DataType group
			case FTOP_MATCHDTGROUP:

				// Need to get datatype?
				if (handle->file && !handle->datatype)
				{
					// DataTypes library valid?
					if (DataTypesBase)
					{
						// Try to get datatype
						handle->datatype = ObtainDataTypeA(DTST_FILE, (APTR)handle->lock, 0);
						handle->flags |= MATCHF_TRIED_DT;
					}
				}

				// Got a datatype?
				if (handle->datatype)
				{
					ULONG match_group;

					// Build group to match
					match_group = (ULONG)(tolower(handle->buffer[0]) << 24) | (tolower(handle->buffer[1]) << 16) |
								  (tolower(handle->buffer[2]) << 8) | tolower(handle->buffer[3]);

					// See if datatype matches
					if (handle->datatype->dtn_Header->dth_GroupID == match_group)
						break;
				}

				// Must have failed
				match_okay = 0;
				break;

			// Match DataType ID
			case FTOP_MATCHDTID:

				// Need to get datatype?
				if (handle->file && !handle->datatype)
				{
					// DataTypes library valid?
					if (DataTypesBase)
					{
						// Try to get datatype
						handle->datatype = ObtainDataTypeA(DTST_FILE, (APTR)handle->lock, 0);
						handle->flags |= MATCHF_TRIED_DT;
					}
				}

				// Got a datatype?
				if (handle->datatype)
				{
					ULONG match_id;

					// Build group to match
					match_id = (ULONG)(tolower(handle->buffer[0]) << 24) | (tolower(handle->buffer[1]) << 16) |
							   (tolower(handle->buffer[2]) << 8) | tolower(handle->buffer[3]);

					// See if datatype matches
					if (handle->datatype->dtn_Header->dth_ID == match_id)
						break;
				}

				// Must have failed
				match_okay = 0;
				break;

			// Directory
			case FTOP_DIRECTORY:
				if (!handle->file && !(handle->flags & MATCHF_IS_DISK))
					dir_flag = 1;
				else
					match_okay = 0;
				break;

			// Disk
			case FTOP_DISK:

				// Fail if a file
				if (handle->file)
				{
					match_okay = 0;
					break;
				}

				// Set disk flag
				disk_flag = 1;

				// Disk?
				if (handle->flags & MATCHF_IS_DISK)
				{
					ULONG match_id, mask = 0xffffffff;
					unsigned char lastc;

					// Over-ride?
					if (strstri((char *)handle->buffer, "override"))
						type->type.flags |= FILETYPEF_OVERRIDE;
					else
						type->type.flags &= ~FILETYPEF_OVERRIDE;

					// Device name?
					if (strchr((char *)handle->buffer, ':'))
					{
						char *ptr1, *ptr2;
						BOOL quote = 0;

						// Match device name
						ptr1 = handle->name;
						ptr2 = (char *)handle->buffer;
						if (*ptr2 == '\"')
						{
							++ptr2;
							quote = 1;
						}
						while (*ptr1 && *ptr2 && *ptr1 != ':' && (*ptr2 == '?' || tolower(*ptr1) == tolower(*ptr2)))
						{
							++ptr1;
							++ptr2;
						}

						// No match?
						if (*ptr1 && *ptr2 && *ptr1 != ':')
							match_okay = 0;
						else
						{
							// Go on to match name too?
							if (*ptr2 == ':' && *(ptr2 + 1) && *(ptr2 + 1) != ' ')
							{
								char test_name[40], parsed_name[80], *p;

								// Match disk name
								stccpy(test_name, ptr2 + 1, 39);
								if ((quote && (p = strchr(test_name, '\"'))) || (p = strchr(test_name, ' ')))
									*p = 0;
								strcat(test_name, ":");
								ParsePatternNoCase(test_name, parsed_name, 80);
								if (!MatchPatternNoCase(parsed_name, handle->fullname))
									match_okay = 0;
							}
						}
						break;
					}

					// Check for wildcard
					if (handle->buffer[0] == '?')
						mask &= 0x00ffffff;
					if (handle->buffer[1] == '?')
						mask &= 0xff00ffff;
					if (handle->buffer[2] == '?')
						mask &= 0xffff00ff;
					if (handle->buffer[3] == '?')
						mask &= 0xffffff00;

					// Last character of ID might be a number or a letter
					lastc = handle->buffer[3];
					if (lastc >= '0' && lastc <= '9')
						lastc -= '0';

					// Build ID to match
					match_id =
						(ULONG)(handle->buffer[0] << 24) | (handle->buffer[1] << 16) | (handle->buffer[2] << 8) | lastc;

					// See if disk type matches
					if (!match_id || (handle->disk_type & mask) == (match_id & mask))
						break;
				}

				// Didn't match
				match_okay = 0;
				break;

			// Check for sound module
			case FTOP_MODULE:

				// Haven't tried yet?
				if (!(handle->flags & MATCHF_TRIED_MODULE))
				{
#if defined(__amigaos3__) || defined(__MORPHOS__)

					struct Library *MUSICBase;

					// Open music library?
					if ((MUSICBase = OpenLibrary("dopus5:libs/inovamusic.library", 0)))
					{
						short ret;

						// Ask library
						ret = IsModule(handle->name);

						// Close library
						CloseLibrary(MUSICBase);

						// Ok?
						if (ret > 0 && ret < 100)
						{
							// It's a module
							handle->flags |= MATCHF_IS_MODULE;
						}
					}
#endif
					// Set flag to say we've tried
					handle->flags |= MATCHF_TRIED_MODULE;
				}

				// Not a module?
				if (!(handle->flags & MATCHF_IS_MODULE))
					match_okay = 0;
				break;

			// Default, clear "test" flag
			default:
				test_flag = 0;
				break;
			}

			// Have we failed?
			if (!match_okay)
			{
				// See if there's any ORs or ANDs
				while (type->recognition[recog_pos])
				{
					if (type->recognition[recog_pos] == FTOP_OR || type->recognition[recog_pos] == FTOP_AND)
						break;
					++recog_pos;
				}

				// If it's an AND, we fail
				if (type->recognition[recog_pos] == FTOP_AND)
					break;

				// If it's an OR, seek to last position
				if (type->recognition[recog_pos] == FTOP_OR && handle->file)
					L_SeekBuf(handle->file, last_position, OFFSET_BEGINNING);
			}

			// Otherwise
			else
			{
				// Save position of last successful match
				if (handle->file)
					last_position = L_SeekBuf(handle->file, 0, OFFSET_CURRENT);

				// If this was a test and the next thing is an OR, we succeed
				if (test_flag && type->recognition[recog_pos] == FTOP_OR)
					break;
			}

			// Reset operation and buffer position
			operation = 0;
			buffer_pos = 0;
		}

		// Store in buffer
		else
			handle->buffer[buffer_pos++] = type->recognition[recog_pos];

		// End of string?
		if (type->recognition[recog_pos] == 0)
			break;
	}

	// If no file, fail unless directory or disk was asked for
	if (!handle->file && !dir_flag && !disk_flag)
		match_okay = 0;

	// Add to cache
	AddFiletypeCache(handle, type, match_okay, (struct LibData *)lib->ml_UserData);
	return match_okay;
}

// Match characters for filetype identification
BOOL filetype_match_chars(MatchHandle *handle, BOOL nocase)
{
	short match_len, check_len;
	short match_pos, check_pos = 0;

	// Valid file?
	if (!handle->file)
		return 0;

	// Get length of match string
	match_len = strlen((char *)handle->buffer);

	// Look at initial character, to find length we need to check for
	switch (handle->buffer[0])
	{
	// Hex
	case '$':
		check_len = (match_len - 1) >> 1;
		break;

	// Binary
	case '%':
		check_len = (match_len - 1) >> 3;
		break;

	// Normal
	default:
		check_len = match_len;
		break;
	}

	// Can't match more than 256 characters
	if (check_len > 256)
		check_len = 256;

	// Read bytes
	if ((L_ReadBuf(handle->file, (char *)handle->wild_buf, check_len)) < check_len)
		return 0;

	// Look at match type
	switch (handle->buffer[0])
	{
	// Hex
	case '$':

		// Go through match buffer
		for (match_pos = 1; match_pos < match_len; match_pos += 2, check_pos++)
		{
			// Not a wildcard
			if (handle->buffer[match_pos] != '?')
			{
				// See if it matches
				if (handle->wild_buf[check_pos] != (unsigned char)L_Atoh(&handle->buffer[match_pos], 2))
					return 0;
			}
		}
		break;

	// Binary
	case '%': {
		short bit_num = 0;

		// Go through match buffer
		for (match_pos = 1; match_pos < match_len; match_pos++)
		{
			// Not a wildcard
			if (handle->buffer[match_pos] != '?')
			{
				// Bit must be on?
				if (handle->buffer[match_pos] == '1' && !(handle->wild_buf[check_pos] & (1 << bit_num)))
					return 0;

				// or must be off
				else if (handle->buffer[match_pos] == '0' && (handle->wild_buf[check_pos] & (1 << bit_num)))
					return 0;
			}

			// Increment bit number
			if ((++bit_num) == 8)
			{
				bit_num = 0;
				++check_pos;
			}
		}
	}
	break;

	// Normal
	default:

		// Go through match buffer
		for (match_pos = 0; match_pos < match_len; match_pos++, check_pos++)
		{
			// If not a wildcard, see if it matches
			if (handle->buffer[match_pos] != '?')
			{
				unsigned char match_ch = 0, wild_ch;

				// Backslash indicates decimal code
				if (handle->buffer[match_pos] == '\\')
				{
					// Get character to match against
					wild_ch = handle->wild_buf[check_pos];

					// Another backslash means literal backslash
					if (handle->buffer[match_pos + 1] == '\\')
					{
						match_ch = '\\';
						++match_pos;
					}

					// Otherwise, should be a decimal number
					else
					{
						short a;

						// Up to three digits
						for (a = 0; a < 3; a++)
						{
							// Is this a number?
							if (handle->buffer[match_pos + 1] >= '0' && handle->buffer[match_pos + 1] <= '9')
							{
								// Add to match character
								match_ch *= 10;
								match_ch += handle->buffer[match_pos + 1] - '0';
								++match_pos;
							}
							else
								break;
						}
					}
				}

				// Otherwise, get characters as normal
				else if (nocase)
				{
					match_ch = tolower(handle->buffer[match_pos]);
					wild_ch = tolower(handle->wild_buf[check_pos]);
				}
				else
				{
					match_ch = handle->buffer[match_pos];
					wild_ch = handle->wild_buf[check_pos];
				}

				// Fail if no match
				if (match_ch != wild_ch)
					return 0;
			}
		}
		break;
	}

	// Successful
	return 1;
}

// Get protection values out of a string (must have and must not have)
void parse_prot_string(unsigned char *string, ULONG *prot)
{
	int a, not_flag = 0;

	// Initialise values
	prot[0] = 0;
	prot[1] = 0;

	// Go through string
	for (a = 0; string[a]; a++)
	{
		char ch;

		// Get character
		ch = tolower(string[a]);

		// - or + turn the NOT flag on or off
		if (ch == '-')
			not_flag = 1;
		else if (ch == '+')
			not_flag = 0;
		else
		{
			int bit;

			// Try to match to a valid protection bit
			for (bit = 0; bit < 8; bit++)
			{
				// Matched?
				if (ch == ((char *)"hsparwed")[bit])
				{
					prot[not_flag] |= (1 << (7 - bit));
					break;
				}
			}
		}
	}
}

// Get two datestamps out of a string
void parse_date_string(char *string, struct DateStamp *date)
{
	char datebuf[85];
	char timebuf[85];
	char *ptr;
	long range;

	// Get first date and time strings
	ptr = L_ParseDateStrings(string, datebuf, timebuf, &range);

	// Convert to a datestamp
	L_DateFromStrings(datebuf, timebuf, &date[0]);

	// If only one date supplied, use midnight on the same day
	if (!range)
		strcpy(timebuf, "23:59:59");

	// Else if a different date and time supplied, parse the strings out
	else
		L_ParseDateStrings(ptr, datebuf, timebuf, &range);

	// Convert second date to a datestamp
	L_DateFromStrings(datebuf, timebuf, &date[1]);

	// Fix seconds to beginning of the minute in first date, and end in the second
	date[0].ds_Tick = ((date[0].ds_Tick / 50) * 50);
	date[1].ds_Tick = ((date[1].ds_Tick / 50) * 50) + 49;
}

// Add entry to the filetype cache
void AddFiletypeCache(MatchHandle *handle, Cfg_Filetype *type, ULONG result, struct LibData *data)
{
	FileTypeCache *cache;

	// Cache disabled?
	if (!(data->flags & LIBDF_FT_CACHE))
		return;

	// Lock cache list
	L_GetSemaphore(&data->filetype_cache.lock, TRUE, 0);

	// See if it's already in the cache
	if (!(cache = (FileTypeCache *)L_FindNameI(&data->filetype_cache.list, handle->fullname)))
	{
		// Allocate a new entry
		if ((cache = L_AllocMemH(data->memory, sizeof(FileTypeCache) + strlen(handle->fullname))))
		{
			// Initialise entry
			cache->ftc_Node.ln_Name = cache->ftc_Name;
			strcpy(cache->ftc_Name, handle->fullname);
			NewList((struct List *)&cache->ftc_List);

			// Add to list
			AddTail(&data->filetype_cache.list, &cache->ftc_Node);

			// Max cache number reached?
			if (data->ft_cache_count == data->ft_cache_max)
			{
				// Remove the first one
				FreeFiletypeCache(data, (FileTypeCache *)data->filetype_cache.list.lh_Head);
			}

			// Increment cache count
			++data->ft_cache_count;
		}
	}

	// Got an entry?
	if (cache)
	{
		FileTypeEntry *entry;

		// Scan filetype match list
		for (entry = (FileTypeEntry *)cache->ftc_List.mlh_Head; entry->fte_Node.mln_Succ;
			 entry = (FileTypeEntry *)entry->fte_Node.mln_Succ)
		{
			// Match filetype pointer?
			if (entry->fte_Pointer == (APTR)type)
				break;
		}

		// Not found?
		if (!entry->fte_Node.mln_Succ)
		{
			// Allocate new entry
			if ((entry = L_AllocMemH(data->memory, sizeof(FileTypeEntry))))
			{
				// Initialise entry and add to list
				entry->fte_Pointer = type;
				AddTail((struct List *)&cache->ftc_List, (struct Node *)entry);
			}
		}

		// Got an entry?
		if (entry)
		{
			// Store result
			entry->fte_Result = result;
		}
	}

	// Unlock list
	L_FreeSemaphore(&data->filetype_cache.lock);
}

// Find entry in the filetype cache
ULONG FindFiletypeCache(MatchHandle *handle, Cfg_Filetype *type, struct LibData *data)
{
	FileTypeCache *cache;
	ULONG result = (ULONG)-1;

	// Cache disabled?
	if (!(data->flags & LIBDF_FT_CACHE))
		return result;

	// Lock cache list
	L_GetSemaphore(&data->filetype_cache.lock, FALSE, 0);

	// See if it's in the cache
	if ((cache = (FileTypeCache *)L_FindNameI(&data->filetype_cache.list, handle->fullname)))
	{
		FileTypeEntry *entry;

		// Scan filetype match list
		for (entry = (FileTypeEntry *)cache->ftc_List.mlh_Head; entry->fte_Node.mln_Succ;
			 entry = (FileTypeEntry *)entry->fte_Node.mln_Succ)
		{
			// Match filetype pointer?
			if (entry->fte_Pointer == (APTR)type)
			{
				// Store result and exit
				result = entry->fte_Result;
				break;
			}
		}
	}

	// Unlock list
	L_FreeSemaphore(&data->filetype_cache.lock);

	return result;
}

// Free a cached entry or all (list must be locked)
void FreeFiletypeCache(struct LibData *data, FileTypeCache *one)
{
	FileTypeCache *cache, *next;

	// Cache disabled?
	if (!(data->flags & LIBDF_FT_CACHE))
		return;

	// Go through list, if given an entry start with that
	for (cache = (one) ? one : (FileTypeCache *)data->filetype_cache.list.lh_Head; cache->ftc_Node.ln_Succ;
		 cache = next)
	{
		FileTypeEntry *entry, *ne;

		// Cache the next entry
		next = (FileTypeCache *)cache->ftc_Node.ln_Succ;

		// Free entries
		for (entry = (FileTypeEntry *)cache->ftc_List.mlh_Head; entry->fte_Node.mln_Succ; entry = ne)
		{
			// Cache next entry
			ne = (FileTypeEntry *)entry->fte_Node.mln_Succ;

			// Free this one
			L_FreeMemH(entry);
		}

		// If we're only freeing one, remove it and decrement count
		if (one)
		{
			Remove((struct Node *)one);
			--data->ft_cache_count;
		}

		// Free cache entry
		L_FreeMemH(cache);

		// If we were only freeing one, break
		if (one)
			break;
	}

	// If we freed the whole lot, reinitialise the list
	if (!one)
	{
		NewList(&data->filetype_cache.list);
		data->ft_cache_count = 0;
	}
}

// Free all cached entries
void LIBFUNC L_ClearFiletypeCache(REG(a6, struct MyLibrary *lib))
{
	struct LibData *data;

#if defined(__amigaos4__) || defined(__AROS__)
	lib = dopuslibbase_global;
#endif

	// Get data pointer
	data = (struct LibData *)lib->ml_UserData;

	// Cache disabled?
	if (!(data->flags & LIBDF_FT_CACHE))
		return;

	// Lock cache list
	L_GetSemaphore(&data->filetype_cache.lock, TRUE, 0);

	// Clear the cache
	FreeFiletypeCache((struct LibData *)lib->ml_UserData, 0);

	// Unlock list
	L_FreeSemaphore(&data->filetype_cache.lock);
}
