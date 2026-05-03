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

#ifdef __amigaos3__
	#define ACTION_GET_FILE_SIZE64 8004
#endif

void LIBFUNC L_DivideU64(REG(a0, UQUAD *num), REG(d0, ULONG div), REG(a1, UQUAD *rem), REG(a2, UQUAD *quo))
{
	UQUAD quotient = *num / div;
	*rem = *num - (div * quotient);
	*quo = quotient;
}

// Convert unsigned integer to a string
void LIBFUNC L_ItoaU64(REG(a0, UQUAD *num), REG(a1, char *str), REG(d0, int str_size), REG(d1, char sep))
{
	char *result;
	char *s;
	int pos = 1;
	UQUAD number = *num;
	UQUAD remainder;

	s = &str[str_size - 1];
	(*s--) = '\0';

	do
	{
		result = s;
		L_DivideU64(&number, 10, &remainder, &number);
		(*s--) = '0' + (remainder % 10);
		if (sep && (pos % 3) == 0)
			(*s--) = sep;
		pos++;
	} while (number > 0);

	if (str < result)
	{
		for (pos = 0; pos < str_size; pos++)
			str[pos] = result[pos];
	}
}

// Do division (fake float) into a string
void LIBFUNC L_DivideToString64(REG(a0, char *string),
								REG(d0, int str_size),
								REG(a1, UQUAD *bytes),
								REG(d1, ULONG div),
								REG(d2, int places),
								REG(d3, char sep))
{
	UQUAD whole;
	// ULONG remainder;
	UQUAD remainder;

	// Zero?
	if (div == 0)
	{
		string[0] = '0';
		string[1] = 0;
		return;
	}

	// Do division
	L_DivideU64(bytes, div, &remainder, &whole);

	// Get whole number string
	L_ItoaU64(&whole, string, str_size, sep);

	// Want remainder?
	if (places > 0)
	{
		char rem_buf[20], form_buf[10];
		unsigned long rem100;

		// Convert to fraction
		rem100 = remainder * 100 / div;

		// Round up
		if (places == 1)
			rem100 += 5;

		// Rounded up to next whole number?
		if (rem100 > 99)
		{
			// Move to next whole number
			rem100 -= 100;
			whole++;

			// Get whole number string again
			L_ItoaU64(&whole, string, str_size, sep);
		}

		// Build formatting string
		lsprintf(form_buf, "%%0%ldld", places + 1);

		// Convert remainder to a string, chop to desired decimal places
		lsprintf(rem_buf, form_buf, rem100);
		rem_buf[places] = 0;

		// Not zero?
		if (atoi(rem_buf) != 0)
		{
			char *ptr = string + strlen(string);
			lsprintf(ptr, "%lc%s", decimal_point, (IPTR)&rem_buf);
		}
	}
}

// Return a disk size as a string
void LIBFUNC L_BytesToString64(REG(a0, UQUAD *bytes),
							   REG(a1, char *string),
							   REG(d0, int str_size),
							   REG(d1, int places),
							   REG(d2, char sep))
{
	UQUAD numbytes = *bytes;
	ULONG div = 0;
	char *size_str = "K";

	// Less than a kilobyte?
	if (numbytes < 1024)
	{
		// Nothing?
		if (numbytes < 1024)
			strncpy(string, "0K", str_size);
		else
		{
			L_ItoaU64(&numbytes, string, str_size, sep);
			strncat(string, "b", str_size);
		}
		return;
	}

	// Convert to kilobytes
	numbytes >>= 10;

	// Petabyte-plus? (genuinely absurd, bail out)
	if (numbytes > (UQUAD)1024 * 1024 * 1024 * 1024)
		strncpy(string, "HUGE", str_size);

	// Terabyte range?
	else if (numbytes > 1073741824)
	{
		div = 1073741824;
		size_str = "T";
	}

	// Gigabyte range?
	else if (numbytes > 1048576)
	{
		div = 1048576;
		size_str = "G";
	}

	// Megabyte range?
	else if (numbytes > 4096)
	{
		div = 1024;
		size_str = "M";
	}

	// Kilobytes
	else
	{
		L_ItoaU64(&numbytes, string, str_size, sep);
		strncat(string, "K", str_size);
	}

	// Need to do a division?
	if (div)
	{
		// Do division to string
		L_DivideToString64(string, str_size, &numbytes, div, places, sep);

		// Tack on character
		strncat(string, size_str, str_size);
	}
}

// just like Examine, but tries to get the 64-bit size too
BOOL LIBFUNC L_ExamineLock64(REG(d0, BPTR lock), REG(a0, FileInfoBlock64 *fib))
{
	BOOL success;

#ifdef __MORPHOS__
	success = Examine64(lock, fib, TAG_DONE);
#else
	success = Examine(lock, (struct FileInfoBlock *)fib);
	fib->fib_Size64 = (UQUAD)((struct FileInfoBlock *)fib)->fib_Size;
#endif

#ifdef __amigaos3__
	/*if (success)
	{
		UQUAD *size_ptr;
		size_ptr = (UQUAD *)DoPkt(((struct FileLock *)BADDR(lock))->fl_Task, ACTION_GET_FILE_SIZE64, (ULONG)lock, 0, 0,
	0, 0);

		if (size_ptr && IoErr() != ERROR_ACTION_NOT_KNOWN)
			fib->fib_Size64 = *size_ptr;
	}*/
#elif defined(__amigaos4__)
	if (success && fib->fib_DirEntryType < 0)
	{
		struct ExamineData *exdata;

		if ((exdata = ExamineObjectTags(EX_FileLockInput, lock, TAG_END)))
		{
			fib->fib_Size64 = exdata->FileSize;
			// D(bug("Got 64-bit size for %s: %lld\n", fib->fib_FileName, fib->fib_Size64));
			FreeDosObject(DOS_EXAMINEDATA, exdata);
		}
	}
#endif

	return success;
}

BOOL LIBFUNC L_ExamineNext64(REG(d0, BPTR lock), REG(a0, FileInfoBlock64 *fib))
{
	BOOL success;

#ifdef __MORPHOS__
	success = ExNext64(lock, fib, TAG_DONE);
#else
	success = ExNext(lock, (struct FileInfoBlock *)fib);
	fib->fib_Size64 = (UQUAD)((struct FileInfoBlock *)fib)->fib_Size;
#endif

#ifdef __amigaos3__
	/*if (success)
	{
		UQUAD *size_ptr;
		size_ptr = (UQUAD *)DoPkt(((struct FileLock *)BADDR(lock))->fl_Task, ACTION_GET_FILE_SIZE64, (ULONG)lock, 0, 0,
	0, 0);

		if (size_ptr && IoErr() != ERROR_ACTION_NOT_KNOWN)
			fib->fib_Size64 = *size_ptr;
	}*/
#elif defined(__amigaos4__)
	if (success && fib->fib_DirEntryType < 0)
	{
		BPTR flock;
		struct ExamineData *exdata;
		char buf[512];

		NameFromLock(lock, buf, sizeof(buf));
		AddPart(buf, fib->fib_FileName, sizeof(buf));

		if ((flock = Lock(buf, ACCESS_READ)))
		{
			if ((exdata = ExamineObjectTags(EX_FileLockInput, flock, TAG_END)))
			{
				fib->fib_Size64 = exdata->FileSize;
				// D(bug("Got 64-bit size for %s: %lld\n", fib->fib_FileName, fib->fib_Size64));
				FreeDosObject(DOS_EXAMINEDATA, exdata);
			}
			UnLock(flock);
		}
	}
#endif

	return success;
}

BOOL LIBFUNC L_ExamineHandle64(REG(d0, BPTR fh), REG(a0, FileInfoBlock64 *fib))
{
	BOOL success;

#ifdef __MORPHOS__
	success = ExamineFH64(fh, fib, TAG_DONE);
#else
	success = ExamineFH(fh, (struct FileInfoBlock *)fib);
	fib->fib_Size64 = (UQUAD)((struct FileInfoBlock *)fib)->fib_Size;
#endif

#ifdef __amigaos3__
	/*if (success)
	{
		UQUAD *size_ptr;

		size_ptr = (UQUAD *)DoPkt(((struct FileHandle *)BADDR(fh))->fh_Type, ACTION_GET_FILE_SIZE64, fh, 0, 0, 0, 0);

		if (size_ptr && IoErr() != ERROR_ACTION_NOT_KNOWN)
			fib->fib_Size64 = *size_ptr;
	}*/
#elif defined(__amigaos4__)
	if (success)
	{
		struct ExamineData *exdata;

		if ((exdata = ExamineObjectTags(EX_FileHandleInput, fh, TAG_END)))
		{
			fib->fib_Size64 = exdata->FileSize;
			// D(bug("Got 64-bit size for %s: %lld\n", fib->fib_FileName, fib->fib_Size64));
			FreeDosObject(DOS_EXAMINEDATA, exdata);
		}
	}
#endif

	return success;
}

LONG LIBFUNC L_MatchFirst64(REG(a0, STRPTR pat), REG(a1, struct AnchorPath *panchor))
{
	// MatchFirst() returns 0 for success, errorcode for error
	LONG error = 0;

	error = MatchFirst(pat, panchor);
#ifndef __MORPHOS__
	((FileInfoBlock64 *)&panchor->ap_Info)->fib_Size64 = (UQUAD)panchor->ap_Info.fib_Size;
#endif

#ifdef __amigaos4__
	if (!error && panchor->ap_Info.fib_DirEntryType < 0)
	{
		BPTR flock;
		struct ExamineData *exdata;
		char buf[512];

		NameFromLock(panchor->ap_Current->an_Lock, buf, sizeof(buf));
		AddPart(buf, panchor->ap_Info.fib_FileName, sizeof(buf));

		if ((flock = Lock(buf, ACCESS_READ)))
		{
			if ((exdata = ExamineObjectTags(EX_FileLockInput, flock, TAG_END)))
			{
				((FileInfoBlock64 *)&panchor->ap_Info)->fib_Size64 = (UQUAD)exdata->FileSize;
				// D(bug("Got 64-bit size for %s: %lld\n", panchor->ap_Info.fib_FileName, ((FileInfoBlock64
				// *)&panchor->ap_Info)->fib_Size64));
				FreeDosObject(DOS_EXAMINEDATA, exdata);
			}
			UnLock(flock);
		}
	}
#endif

	return error;
}

LONG LIBFUNC L_MatchNext64(REG(a0, struct AnchorPath *panchor))
{
	// MatchNext() returns 0 for success, errorcode for error
	LONG error = 0;

	error = MatchNext(panchor);
#ifndef __MORPHOS__
	((FileInfoBlock64 *)&panchor->ap_Info)->fib_Size64 = (UQUAD)panchor->ap_Info.fib_Size;
#endif

#ifdef __amigaos4__
	if (!error && panchor->ap_Info.fib_DirEntryType < 0)
	{
		BPTR flock;
		struct ExamineData *exdata;
		char buf[512];

		NameFromLock(panchor->ap_Current->an_Lock, buf, sizeof(buf));
		AddPart(buf, panchor->ap_Info.fib_FileName, sizeof(buf));

		if ((flock = Lock(buf, ACCESS_READ)))
		{
			if ((exdata = ExamineObjectTags(EX_FileLockInput, flock, TAG_END)))
			{
				((FileInfoBlock64 *)&panchor->ap_Info)->fib_Size64 = (UQUAD)exdata->FileSize;
				// D(bug("Got 64-bit size for %s: %lld\n", panchor->ap_Info.fib_FileName, ((FileInfoBlock64
				// *)&panchor->ap_Info)->fib_Size64));
				FreeDosObject(DOS_EXAMINEDATA, exdata);
			}
			UnLock(flock);
		}
	}
#endif

	return error;
}

// Unfinished, probably not worth the trouble
#if 0
void LIBFUNC L_SeekHandle64(
	REG(d0, BPTR file),
	REG(a0, UQUAD *pos),
	REG(d1, LONG mode),
	REG(a1, UQUAD *oldpos))
{
	UQUAD retval=-1;
	#ifdef __amigaos3__

	retval=(UQUAD)Seek(file, (ULONG)*pos, mode);
	#elif defined(__amigaos4__)
	UQUAD oldpos;
	UQUAD size;
	UQUAD startpos=0;

	oldpos=GetFilePosition(file);
	size=GetFileSize(file);

	switch (mode)
	{
		case OFFSET_BEGINNING:
			startpos=0;
			break;
		case OFFSET_CURRENT:
			startpos=oldpos;
			break
		case OFFSET_END:
			startpos=size;
			break;
	}

	retval=startpos;
	if (*pos!=0 && ChangeFilePosition(file, startpos+*pos, 0)==DOSTRUE)
		retval=GetFilePosition(file);
	#elif defined(__MORPHOS__)
	retval=Seek64(file, *pos, mode);
	#else
	retval=(UQUAD)Seek(file, (ULONG)*pos, mode);
	#endif
	if (oldpos)
		*oldpos=retval;
}
#endif
