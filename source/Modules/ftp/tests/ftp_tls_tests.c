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
	check_true("tls parses explicit", ftp_tls_mode_from_text("explicit", &mode));
	check_int("tls explicit mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses ftps", ftp_tls_mode_from_text(" FTPS ", &mode));
	check_int("tls ftps mode", mode, FTP_TLS_MODE_EXPLICIT);

	mode = -1;
	check_true("tls parses auth tls", ftp_tls_mode_from_text("\tAUTH TLS\r\n", &mode));
	check_int("tls auth tls mode", mode, FTP_TLS_MODE_EXPLICIT);

	check_false("tls rejects implicit", ftp_tls_mode_from_text("implicit", &mode));
	check_false("tls rejects starttls", ftp_tls_mode_from_text("starttls", &mode));
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
}

static void test_tls_session_defaults(void)
{
	struct ftp_tls_session session;

	session.ctx = (void *)1;
	session.ssl = (void *)2;
	session.socket = 42;
	session.active = 1;

	ftp_tls_session_init(&session);
	check_true("tls session clears ctx", session.ctx == NULL);
	check_true("tls session clears ssl", session.ssl == NULL);
	check_int("tls session clears socket", session.socket, -1);
	check_false("tls session inactive", session.active);
}

int main(void)
{
	test_tls_modes();
	test_tls_mode_properties();
	test_tls_session_defaults();

	if (failures)
	{
		printf("%d ftp_tls test(s) failed\n", failures);
		return EXIT_FAILURE;
	}

	printf("ftp_tls tests passed (%s backend)\n", ftp_tls_backend_name());
	return EXIT_SUCCESS;
}
