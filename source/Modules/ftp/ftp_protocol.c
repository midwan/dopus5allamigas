/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#include "ftp_protocol.h"

#include <ctype.h>
#include <string.h>

static int protocol_text_equals(const char *a, const char *b)
{
	unsigned char ca;
	unsigned char cb;

	if (!a || !b)
		return 0;

	while (*a && isspace((unsigned char)*a))
		++a;

	while (*b && isspace((unsigned char)*b))
		++b;

	while (*a && *b)
	{
		ca = (unsigned char)*a;
		cb = (unsigned char)*b;

		if (isspace(ca))
		{
			while (*a && isspace((unsigned char)*a))
				++a;
			return *a == 0 && *b == 0;
		}

		if (tolower(ca) != tolower(cb))
			return 0;

		++a;
		++b;
	}

	while (*a && isspace((unsigned char)*a))
		++a;
	while (*b && isspace((unsigned char)*b))
		++b;

	return *a == 0 && *b == 0;
}

static int protocol_text_starts_with(const char *text, const char *prefix)
{
	if (!text || !prefix)
		return 0;

	while (*prefix)
	{
		if (tolower((unsigned char)*text) != tolower((unsigned char)*prefix))
			return 0;
		++text;
		++prefix;
	}

	return 1;
}

const char *ftp_protocol_name(int protocol)
{
	switch (protocol)
	{
	case FTP_PROTOCOL_FTP:
		return "ftp";
	case FTP_PROTOCOL_SFTP:
		return "sftp";
	}

	return "unknown";
}

int ftp_protocol_default_port(int protocol)
{
	switch (protocol)
	{
	case FTP_PROTOCOL_FTP:
		return 21;
	case FTP_PROTOCOL_SFTP:
		return 22;
	}

	return 0;
}

int ftp_protocol_from_text(const char *text, int *protocol)
{
	if (!text || !*text || protocol_text_equals(text, "ftp") || protocol_text_equals(text, "ftps") ||
		protocol_text_equals(text, "ftpes"))
	{
		if (protocol)
			*protocol = FTP_PROTOCOL_FTP;
		return 1;
	}

	if (protocol_text_equals(text, "sftp") || protocol_text_equals(text, "ssh") ||
		protocol_text_equals(text, "ssh2"))
	{
		if (protocol)
			*protocol = FTP_PROTOCOL_SFTP;
		return 1;
	}

	return 0;
}

int ftp_protocol_from_url_scheme(const char *url, const char **without_scheme, int *protocol)
{
	if (protocol_text_starts_with(url, "ftp://"))
	{
		if (without_scheme)
			*without_scheme = url + 6;
		if (protocol)
			*protocol = FTP_PROTOCOL_FTP;
		return 1;
	}

	if (protocol_text_starts_with(url, "ftps://"))
	{
		if (without_scheme)
			*without_scheme = url + 7;
		if (protocol)
			*protocol = FTP_PROTOCOL_FTP;
		return 1;
	}

	if (protocol_text_starts_with(url, "sftp://"))
	{
		if (without_scheme)
			*without_scheme = url + 7;
		if (protocol)
			*protocol = FTP_PROTOCOL_SFTP;
		return 1;
	}

	return 0;
}

int ftp_protocol_infer_from_connection(int protocol, int port, int anonymous)
{
	(void)port;
	(void)anonymous;

	if (protocol == FTP_PROTOCOL_SFTP)
		return FTP_PROTOCOL_SFTP;

	return FTP_PROTOCOL_FTP;
}
