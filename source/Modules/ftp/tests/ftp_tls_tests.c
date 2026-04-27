/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#include "../ftp_tls.h"

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

static void test_tls_modes(void)
{
	int mode = -1;

	check_true("tls parses empty as off", ftp_tls_mode_from_text("", &mode));
	check_int("tls empty mode", mode, FTP_TLS_MODE_OFF);

	mode = -1;
	check_true("tls parses ftp as off", ftp_tls_mode_from_text("ftp", &mode));
	check_int("tls ftp mode", mode, FTP_TLS_MODE_OFF);

	mode = -1;
	check_true("tls parses plain as off", ftp_tls_mode_from_text("plain", &mode));
	check_int("tls plain mode", mode, FTP_TLS_MODE_OFF);

	mode = -1;
	check_true("tls parses no as off", ftp_tls_mode_from_text(" no ", &mode));
	check_int("tls no mode", mode, FTP_TLS_MODE_OFF);

	mode = -1;
	check_true("tls parses false as off", ftp_tls_mode_from_text("false", &mode));
	check_int("tls false mode", mode, FTP_TLS_MODE_OFF);

	mode = -1;
	check_true("tls parses explicit", ftp_tls_mode_from_text("explicit", &mode));
	check_int("tls explicit mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses ftps", ftp_tls_mode_from_text(" FTPS ", &mode));
	check_int("tls ftps mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses ftpes", ftp_tls_mode_from_text("ftpes", &mode));
	check_int("tls ftpes mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses auth tls", ftp_tls_mode_from_text("\tAUTH TLS\r\n", &mode));
	check_int("tls auth tls mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses starttls", ftp_tls_mode_from_text("STARTTLS", &mode));
	check_int("tls starttls mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses on", ftp_tls_mode_from_text("on", &mode));
	check_int("tls on mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses yes", ftp_tls_mode_from_text("yes", &mode));
	check_int("tls yes mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses true", ftp_tls_mode_from_text(" TRUE ", &mode));
	check_int("tls true mode", mode, FTP_TLS_MODE_EXPLICIT);

	check_false("tls rejects implicit", ftp_tls_mode_from_text("implicit", &mode));
	check_false("tls rejects sftp", ftp_tls_mode_from_text("sftp", &mode));
}

static void test_tls_mode_properties(void)
{
	check_string("tls off name", ftp_tls_mode_name(FTP_TLS_MODE_OFF), "off");
	check_string("tls explicit name", ftp_tls_mode_name(FTP_TLS_MODE_EXPLICIT), "explicit");
	check_string("tls unknown name", ftp_tls_mode_name(99), "unknown");

	check_false("tls off has plain control", ftp_tls_mode_uses_control_tls(FTP_TLS_MODE_OFF));
	check_false("tls off has plain data", ftp_tls_mode_uses_data_tls(FTP_TLS_MODE_OFF));
	check_true("tls explicit protects control", ftp_tls_mode_uses_control_tls(FTP_TLS_MODE_EXPLICIT));
	check_true("tls explicit protects data", ftp_tls_mode_uses_data_tls(FTP_TLS_MODE_EXPLICIT));

	check_true("plain ftp allows server transfer",
			   ftp_tls_modes_allow_server_transfer(FTP_TLS_MODE_OFF, FTP_TLS_MODE_OFF));
	check_false("source ftps blocks server transfer",
				ftp_tls_modes_allow_server_transfer(FTP_TLS_MODE_EXPLICIT, FTP_TLS_MODE_OFF));
	check_false("dest ftps blocks server transfer",
				ftp_tls_modes_allow_server_transfer(FTP_TLS_MODE_OFF, FTP_TLS_MODE_EXPLICIT));
	check_false("ftps pair blocks server transfer",
				ftp_tls_modes_allow_server_transfer(FTP_TLS_MODE_EXPLICIT, FTP_TLS_MODE_EXPLICIT));
}

static void test_tls_url_schemes(void)
{
	const char *body = NULL;
	int mode = -1;

	check_true("tls parses ftp url scheme", ftp_tls_mode_from_url_scheme("ftp://example.com/pub", &body, &mode));
	check_int("tls ftp url mode", mode, FTP_TLS_MODE_OFF);
	check_string("tls ftp url body", body, "example.com/pub");

	body = NULL;
	mode = -1;
	check_true("tls parses ftps url scheme", ftp_tls_mode_from_url_scheme("FTPS://example.com/pub", &body, &mode));
	check_int("tls ftps url mode", mode, FTP_TLS_MODE_EXPLICIT);
	check_string("tls ftps url body", body, "example.com/pub");

	check_false("tls rejects url without scheme", ftp_tls_mode_from_url_scheme("example.com/pub", &body, &mode));
	check_false("tls rejects sftp url scheme", ftp_tls_mode_from_url_scheme("sftp://example.com/pub", &body, &mode));
}

static void test_tls_session_defaults(void)
{
	struct ftp_tls_session session;
	int mode = 0;

	session.ctx = (void *)1;
	session.ssl = (void *)2;
	session.socket = 42;
	session.active = 1;

	ftp_tls_session_init(&session);
	check_true("tls session clears ctx", session.ctx == NULL);
	check_true("tls session clears ssl", session.ssl == NULL);
	check_int("tls session clears socket", session.socket, -1);
	check_false("tls session inactive", session.active);
	check_int("tls session clears error", ftp_tls_session_error(&session), FTP_TLS_ERROR_NONE);
	check_int("tls null error", ftp_tls_session_error(NULL), FTP_TLS_ERROR_NONE);
	check_int("tls inactive pending", ftp_tls_pending(&session), 0);
	check_int("tls inactive read fails", ftp_tls_read(&session, &mode, sizeof(mode)), -1);
}

static void test_tls_connect_failures(void)
{
	struct ftp_tls_session session;
	struct ftp_tls_session reuse_session;

	ftp_tls_session_init(&session);
	ftp_tls_session_init(&reuse_session);
	check_false("tls connect rejects invalid socket", ftp_tls_connect(&session, -1, "example.com", 1));
	check_int("tls invalid socket error", ftp_tls_session_error(&session), FTP_TLS_ERROR_CONTEXT);
	check_false("tls invalid socket inactive", session.active);
	check_false("tls reuse rejects invalid socket",
				ftp_tls_connect_reuse(&session, -1, "example.com", 1, &reuse_session));
	check_int("tls reuse invalid socket error", ftp_tls_session_error(&session), FTP_TLS_ERROR_CONTEXT);
	check_false("tls reuse invalid socket inactive", session.active);

	if (!strcmp(ftp_tls_backend_name(), "none"))
	{
		session.active = 1;
		session.socket = 42;
		session.last_error = FTP_TLS_ERROR_HANDSHAKE;

		check_false("tls connect reports missing backend", ftp_tls_connect(&session, 0, "example.com", 0));
		check_int("tls missing backend error", ftp_tls_session_error(&session), FTP_TLS_ERROR_BACKEND);
		check_false("tls missing backend inactive", session.active);
		check_int("tls missing backend clears socket", session.socket, -1);

		check_false("tls reuse reports missing backend",
					ftp_tls_connect_reuse(&session, 0, "example.com", 0, &reuse_session));
		check_int("tls reuse missing backend error", ftp_tls_session_error(&session), FTP_TLS_ERROR_BACKEND);
		check_false("tls reuse missing backend inactive", session.active);
	}
}

int main(void)
{
	test_tls_modes();
	test_tls_mode_properties();
	test_tls_url_schemes();
	test_tls_session_defaults();
	test_tls_connect_failures();

	if (failures)
	{
		printf("%d ftp_tls test(s) failed\n", failures);
		return EXIT_FAILURE;
	}

	printf("ftp_tls tests passed (%s backend)\n", ftp_tls_backend_name());
	return EXIT_SUCCESS;
}
