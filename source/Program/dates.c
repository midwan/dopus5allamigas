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

#include "dopus.h"

// Turn a DateStamp into a string
void date_build_string(struct DateStamp *date, char *buffer, int pad)
{
	char time_buf[LEN_DATSTRING], date_buf[LEN_DATSTRING], buf[40];

	// Get date and time strings
	date_to_strings(date, date_buf, time_buf, 1);

	// Build date and time string
	lsprintf(buf, "%-9s %s", date_buf, time_buf);
	buf[22] = 0;

	// If padding selected, pad buffer with spaces at the start
	buffer[0] = 0;
	if (pad)
	{
		short len = GUI->date_length + 9;
		pad = ((environment->env->settings.date_flags & DATE_12HOUR) ? len + 1 : len) - strlen(buf);
		if (pad > 0)
			stccpy(buffer, str_space_string, pad);
	}

	// Add date string to end of buffer
	strcat(buffer, buf);
}

// Convert a DateStamp to two strings
void date_to_strings(struct DateStamp *date, char *date_buf, char *time_buf, int flags)
{
	struct DateTime datetime;

	// Initialise DateTime structure
	datetime.dat_Stamp = *date;
	datetime.dat_Format = environment->env->settings.date_format;
	datetime.dat_Flags = 0;
	datetime.dat_StrDay = 0;
	datetime.dat_StrDate = date_buf;
	datetime.dat_StrTime = 0;

	// Sub-strings ok?
	if (flags == -1 || (flags == 1 && environment->env->settings.date_flags & DATE_SUBST))
		datetime.dat_Flags |= DTF_SUBST;

	// Convert date to a string
	DateToStr(&datetime);

	// No time buffer?
	if (!time_buf)
		return;

	// Build time string. 12 hour clock?
	if (flags > 0 && environment->env->settings.date_flags & DATE_12HOUR)
	{
		int hours;
		char ampm = 'a';

		// Get hours, convert to 12 hour clock
		hours = datetime.dat_Stamp.ds_Minute / 60;
		if (hours > 11)
		{
			ampm = 'p';
			hours -= 12;
		}
		if (hours == 0)
			hours = 12;

		// Build time string
		lsprintf(time_buf,
				 "%2ld:%02ld:%02ld%lc",
				 hours,
				 datetime.dat_Stamp.ds_Minute % 60,
				 datetime.dat_Stamp.ds_Tick / TICKS_PER_SECOND,
				 ampm);
	}

	// 24 hour clock
	else
		lsprintf(time_buf,
				 "%02ld:%02ld:%02ld",
				 datetime.dat_Stamp.ds_Minute / 60,
				 datetime.dat_Stamp.ds_Minute % 60,
				 datetime.dat_Stamp.ds_Tick / TICKS_PER_SECOND);
}

// Get date as a string
void date_string(long days, char *string, short format, BOOL paren)
{
	struct DateTime dt;
	char buf[20];

	// Fill out datetime
	dt.dat_Stamp.ds_Days = days;
	dt.dat_Stamp.ds_Minute = 0;
	dt.dat_Stamp.ds_Tick = 0;
	dt.dat_Format = (format == -1) ? environment->env->settings.date_format : format;
	dt.dat_Flags = 0;
	dt.dat_StrDay = 0;
	dt.dat_StrDate = (paren) ? buf : string;
	dt.dat_StrTime = 0;

	// Convert string
	DateToStr(&dt);

	// Want parentheses?
	if (paren)
	{
		char *ptr, *ptr1;

		// Strip leading spaces
		for (ptr = buf; *ptr == ' '; ptr++)
			;

		// Strip trailing spaces
		for (ptr1 = ptr + strlen(ptr) - 1; ptr1 > ptr && *ptr1 == ' '; --ptr1)
			;
		if (*(ptr1 + 1) == ' ')
			*(ptr1 + 1) = 0;

		// Build string
		lsprintf(string, "(%s)", ptr);
	}
}

// Get time as a string
void time_string(long minutes, long tick, char *string)
{
	// Build string
	lsprintf(string, "%02ld:%02ld:%02ld", minutes / 60, minutes % 60, tick / TICKS_PER_SECOND);
}
