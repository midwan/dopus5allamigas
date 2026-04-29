/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#include "../ftp_protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void check_true(const char *name, int value)
{
	if (!value)
	{
		printf("FAIL: %s\n", name);
		++failures;
	}
}

static void check_false(const char *name, int value)
{
	if (value)
	{
		printf("FAIL: %s\n", name);
		++failures;
	}
}

static void check_int(const char *name, int actual, int expected)
{
	if (actual != expected)
	{
		printf("FAIL: %s: got %d expected %d\n", name, actual, expected);
		++failures;
	}
}

static void check_string(const char *name, const char *actual, const char *expected)
{
	if (!actual || strcmp(actual, expected))
	{
		printf("FAIL: %s: got %s expected %s\n", name, actual ? actual : "(null)", expected);
		++failures;
	}
}

static void test_protocol_text(void)
{
	int protocol = -1;

	check_true("empty defaults to ftp", ftp_protocol_from_text("", &protocol));
	check_int("empty protocol", protocol, FTP_PROTOCOL_FTP);

	protocol = -1;
	check_true("ftp text", ftp_protocol_from_text(" FTP ", &protocol));
	check_int("ftp protocol", protocol, FTP_PROTOCOL_FTP);

	protocol = -1;
	check_true("ftps text is ftp transport", ftp_protocol_from_text("ftps", &protocol));
	check_int("ftps protocol", protocol, FTP_PROTOCOL_FTP);

	protocol = -1;
	check_true("sftp text", ftp_protocol_from_text("sftp", &protocol));
	check_int("sftp protocol", protocol, FTP_PROTOCOL_SFTP);

	protocol = -1;
	check_true("ssh alias", ftp_protocol_from_text("ssh", &protocol));
	check_int("ssh protocol", protocol, FTP_PROTOCOL_SFTP);

	check_false("reject http", ftp_protocol_from_text("http", &protocol));
}

static void test_protocol_properties(void)
{
	check_string("ftp name", ftp_protocol_name(FTP_PROTOCOL_FTP), "ftp");
	check_string("sftp name", ftp_protocol_name(FTP_PROTOCOL_SFTP), "sftp");
	check_string("unknown name", ftp_protocol_name(99), "unknown");

	check_int("ftp port", ftp_protocol_default_port(FTP_PROTOCOL_FTP), 21);
	check_int("sftp port", ftp_protocol_default_port(FTP_PROTOCOL_SFTP), 22);
	check_int("unknown port", ftp_protocol_default_port(99), 0);

	check_int("infer ftp on standard port",
			  ftp_protocol_infer_from_connection(FTP_PROTOCOL_FTP, 21, 0),
			  FTP_PROTOCOL_FTP);
	check_int("keep ftp account login on port 22",
			  ftp_protocol_infer_from_connection(FTP_PROTOCOL_FTP, 22, 0),
			  FTP_PROTOCOL_FTP);
	check_int("keep ftp for anonymous port 22",
			  ftp_protocol_infer_from_connection(FTP_PROTOCOL_FTP, 22, 1),
			  FTP_PROTOCOL_FTP);
	check_int("keep explicit sftp",
			  ftp_protocol_infer_from_connection(FTP_PROTOCOL_SFTP, 21, 1),
			  FTP_PROTOCOL_SFTP);
}

static void test_protocol_url_schemes(void)
{
	const char *body = NULL;
	int protocol = -1;

	check_true("ftp url", ftp_protocol_from_url_scheme("ftp://example.com/pub", &body, &protocol));
	check_int("ftp url protocol", protocol, FTP_PROTOCOL_FTP);
	check_string("ftp url body", body, "example.com/pub");

	body = NULL;
	protocol = -1;
	check_true("ftps url", ftp_protocol_from_url_scheme("FTPS://example.com/pub", &body, &protocol));
	check_int("ftps url protocol", protocol, FTP_PROTOCOL_FTP);
	check_string("ftps url body", body, "example.com/pub");

	body = NULL;
	protocol = -1;
	check_true("sftp url", ftp_protocol_from_url_scheme("sftp://example.com/pub", &body, &protocol));
	check_int("sftp url protocol", protocol, FTP_PROTOCOL_SFTP);
	check_string("sftp url body", body, "example.com/pub");

	check_false("reject missing scheme", ftp_protocol_from_url_scheme("example.com/pub", &body, &protocol));
	check_false("reject http scheme", ftp_protocol_from_url_scheme("http://example.com/pub", &body, &protocol));
}

int main(void)
{
	test_protocol_text();
	test_protocol_properties();
	test_protocol_url_schemes();

	if (failures)
	{
		printf("%d ftp_protocol test(s) failed\n", failures);
		return EXIT_FAILURE;
	}

	printf("ftp_protocol tests passed\n");
	return EXIT_SUCCESS;
}
