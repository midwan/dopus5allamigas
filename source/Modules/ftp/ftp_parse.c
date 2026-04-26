/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#include "ftp_parse.h"

#include <ctype.h>
#include <string.h>

static int ftp_parse_isdigit(char c)
{
	return isdigit((unsigned char)c);
}

static void ftp_parse_copy_tuple(unsigned int dest[6], const unsigned int src[6])
{
	int i;

	for (i = 0; i < 6; i++)
		dest[i] = src[i];
}

int ftp_parse_has_eol(const char *text)
{
	if (!text)
		return 0;

	while (*text)
	{
		if (*text == '\r' || *text == '\n')
			return 1;
		++text;
	}

	return 0;
}

int ftp_parse_epsv_port(const char *buf, unsigned int *port)
{
	const char *p;
	char delim;
	unsigned int value = 0;
	int i;

	if (!port || !buf)
		return 0;

	if (!(p = strchr(buf, '(')))
		return 0;

	++p;
	if (!(delim = *p))
		return 0;

	for (i = 0; i < 3; i++)
	{
		if (*p != delim)
			return 0;
		++p;
	}

	if (!ftp_parse_isdigit(*p))
		return 0;

	while (ftp_parse_isdigit(*p))
	{
		value = (value * 10) + (*p - '0');
		if (value > 65535)
			return 0;
		++p;
	}

	if (*p != delim || value == 0)
		return 0;
	++p;

	while (*p == ' ' || *p == '\t')
		++p;

	if (*p != ')')
		return 0;

	*port = value;
	return 1;
}

static int ftp_parse_pasv_tuple_at(const char *in, unsigned int values[6], int require_close)
{
	int i;

	for (i = 0; i < 6; i++)
	{
		unsigned int value = 0;

		while (*in == ' ' || *in == '\t')
			++in;

		if (!ftp_parse_isdigit(*in))
			return 0;

		while (ftp_parse_isdigit(*in))
		{
			value = (value * 10) + (*in - '0');
			if (value > 255)
				return 0;
			++in;
		}

		values[i] = value;

		while (*in == ' ' || *in == '\t')
			++in;

		if (i < 5)
		{
			if (*in != ',')
				return 0;
			++in;
		}
	}

	if (((values[4] << 8) + values[5]) == 0)
		return 0;

	if (require_close)
	{
		while (*in == ' ' || *in == '\t')
			++in;

		if (*in != ')')
			return 0;
	}

	return 1;
}

int ftp_parse_pasv_tuple(const char *buf, unsigned int values[6])
{
	const char *in;
	unsigned int tmp[6];

	if (!buf || !values)
		return 0;

	if ((in = strchr(buf, '(')))
	{
		if (ftp_parse_pasv_tuple_at(in + 1, tmp, 1))
		{
			ftp_parse_copy_tuple(values, tmp);
			return 1;
		}

		return 0;
	}

	if (!strncmp(buf, "227", 3))
		in = buf + 3;
	else
		in = buf;

	while (*in)
	{
		while (*in && !ftp_parse_isdigit(*in))
			++in;

		if (!*in)
			break;

		if (ftp_parse_pasv_tuple_at(in, tmp, 0))
		{
			ftp_parse_copy_tuple(values, tmp);
			return 1;
		}

		++in;
	}

	return 0;
}
