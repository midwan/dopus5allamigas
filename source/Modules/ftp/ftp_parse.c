/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

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

static int ftp_parse_isspace(char c)
{
	return c == ' ' || c == '\t';
}

static char ftp_parse_toupper(char c)
{
	if (c >= 'a' && c <= 'z')
		return (char)(c - ('a' - 'A'));

	return c;
}

static const char *ftp_parse_skip_spaces(const char *text)
{
	while (ftp_parse_isspace(*text))
		++text;

	return text;
}

static int ftp_parse_feature_token_end(char c)
{
	return c == 0 || c == '\r' || c == '\n' || ftp_parse_isspace(c) || c == ';';
}

static int ftp_parse_feature_word(const char *text, const char *word)
{
	while (*word)
	{
		if (ftp_parse_toupper(*text) != *word)
			return 0;

		++text;
		++word;
	}

	return ftp_parse_feature_token_end(*text);
}

static int ftp_parse_feature_prefix(const char **text, const char *word)
{
	const char *p = *text;

	while (*word)
	{
		if (ftp_parse_toupper(*p) != *word)
			return 0;

		++p;
		++word;
	}

	*text = p;
	return 1;
}

static int ftp_parse_feature_rest_stream(const char *text)
{
	if (!ftp_parse_feature_prefix(&text, "REST") || !ftp_parse_isspace(*text))
		return 0;

	text = ftp_parse_skip_spaces(text);

	return ftp_parse_feature_word(text, "STREAM");
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

unsigned int ftp_parse_feat_line(const char *line)
{
	const char *feat;
	unsigned int caps = 0;

	if (!line)
		return 0;

	feat = ftp_parse_skip_spaces(line);

	if (ftp_parse_isdigit(feat[0]) && ftp_parse_isdigit(feat[1]) && ftp_parse_isdigit(feat[2]) &&
		(feat[3] == '-' || feat[3] == ' '))
		feat = ftp_parse_skip_spaces(feat + 4);

	if (ftp_parse_feature_word(feat, "MLST") || ftp_parse_feature_word(feat, "MLSD"))
		caps |= FTP_PARSE_FEAT_MLST;

	if (ftp_parse_feature_word(feat, "EPSV"))
		caps |= FTP_PARSE_FEAT_EPSV;

	if (ftp_parse_feature_word(feat, "SIZE"))
		caps |= FTP_PARSE_FEAT_SIZE;

	if (ftp_parse_feature_word(feat, "MDTM"))
		caps |= FTP_PARSE_FEAT_MDTM;

	if (ftp_parse_feature_rest_stream(feat))
		caps |= FTP_PARSE_FEAT_REST_STREAM;

	if (ftp_parse_feature_word(feat, "UTF8"))
		caps |= FTP_PARSE_FEAT_UTF8;

	return caps;
}

unsigned int ftp_parse_feat_reply(const char *reply)
{
	unsigned int caps = 0;

	if (!reply)
		return 0;

	while (*reply)
	{
		caps |= ftp_parse_feat_line(reply);

		while (*reply && *reply != '\r' && *reply != '\n')
			++reply;

		while (*reply == '\r' || *reply == '\n')
			++reply;
	}

	return caps;
}

int ftp_parse_ipv4_is_non_global(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
	if (a > 255 || b > 255 || c > 255 || d > 255)
		return 1;

	if (a == 0 || a == 10 || a == 127)
		return 1;

	if (a == 100 && b >= 64 && b <= 127)
		return 1;

	if (a == 169 && b == 254)
		return 1;

	if (a == 172 && b >= 16 && b <= 31)
		return 1;

	if (a == 192 && b == 168)
		return 1;

	if (a == 192 && b == 0 && c == 0)
		return 1;

	if (a == 192 && b == 0 && c == 2)
		return 1;

	if (a == 198 && (b == 18 || b == 19))
		return 1;

	if (a == 198 && b == 51 && c == 100)
		return 1;

	if (a == 203 && b == 0 && c == 113)
		return 1;

	if (a >= 224)
		return 1;

	return 0;
}

static int ftp_parse_ipv4_is_same_address(const unsigned int left[4], const unsigned int right[4])
{
	return left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3];
}

static int ftp_parse_ipv4_is_link_local(const unsigned int octets[4])
{
	return octets[0] == 169 && octets[1] == 254;
}

int ftp_parse_pasv_should_use_control_peer(const unsigned int pasv[4], const unsigned int peer[4])
{
	if (!pasv || !peer)
		return 0;

	if (ftp_parse_ipv4_is_same_address(pasv, peer))
		return 0;

	if (pasv[0] == 0 || pasv[0] >= 224)
		return 1;

	if (pasv[0] == 127 && peer[0] != 127)
		return 1;

	if (ftp_parse_ipv4_is_link_local(pasv) && !ftp_parse_ipv4_is_link_local(peer))
		return 1;

	return ftp_parse_ipv4_is_non_global(pasv[0], pasv[1], pasv[2], pasv[3]) &&
		   !ftp_parse_ipv4_is_non_global(peer[0], peer[1], peer[2], peer[3]);
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
