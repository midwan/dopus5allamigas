#include "../ftp_parse.h"

#include <stdio.h>
#include <stdlib.h>

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

static void check_uint(const char *name, unsigned int actual, unsigned int expected)
{
	if (actual != expected)
	{
		printf("FAIL: %s: got %u expected %u\n", name, actual, expected);
		++failures;
	}
}

static void test_epsv(void)
{
	unsigned int port = 0;

	check_true("epsv parses standard reply",
			   ftp_parse_epsv_port("229 Entering Extended Passive Mode (|||49152|)", &port));
	check_uint("epsv standard port", port, 49152);

	port = 0;
	check_true("epsv parses custom delimiter",
			   ftp_parse_epsv_port("229 EPSV ok (!!!6446!)", &port));
	check_uint("epsv custom delimiter port", port, 6446);

	check_false("epsv rejects missing tuple",
				ftp_parse_epsv_port("229 Entering Extended Passive Mode |||49152|", &port));
	check_false("epsv rejects zero port",
				ftp_parse_epsv_port("229 Entering Extended Passive Mode (|||0|)", &port));
	check_false("epsv rejects port overflow",
				ftp_parse_epsv_port("229 Entering Extended Passive Mode (|||65536|)", &port));
	check_false("epsv rejects unclosed tuple",
				ftp_parse_epsv_port("229 Entering Extended Passive Mode (|||49152|", &port));
	check_false("epsv rejects junk after tuple",
				ftp_parse_epsv_port("229 Entering Extended Passive Mode (|||49152|x)", &port));
}

static void test_pasv(void)
{
	unsigned int values[6];

	check_true("pasv parses standard reply",
			   ftp_parse_pasv_tuple("227 Entering Passive Mode (192,0,2,10,195,80)", values));
	check_uint("pasv host 0", values[0], 192);
	check_uint("pasv host 1", values[1], 0);
	check_uint("pasv host 2", values[2], 2);
	check_uint("pasv host 3", values[3], 10);
	check_uint("pasv port high", values[4], 195);
	check_uint("pasv port low", values[5], 80);

	check_true("pasv parses loose tuple without parentheses",
			   ftp_parse_pasv_tuple("227 Passive mode ok 10, 0, 0, 1, 4, 1", values));
	check_uint("pasv loose port", (values[4] << 8) + values[5], 1025);

	check_false("pasv rejects octet overflow",
				ftp_parse_pasv_tuple("227 Entering Passive Mode (256,0,2,10,195,80)", values));
	check_false("pasv rejects missing fields",
				ftp_parse_pasv_tuple("227 Entering Passive Mode (192,0,2,10,195)", values));
	check_false("pasv rejects zero port",
				ftp_parse_pasv_tuple("227 Entering Passive Mode (192,0,2,10,0,0)", values));
	check_false("pasv rejects unclosed tuple",
				ftp_parse_pasv_tuple("227 Entering Passive Mode (192,0,2,10,195,80", values));
}

static void test_eol(void)
{
	check_false("eol rejects null", ftp_parse_has_eol(0));
	check_false("eol accepts plain text", ftp_parse_has_eol("LIST /pub"));
	check_true("eol detects cr", ftp_parse_has_eol("LIST /pub\rNOOP"));
	check_true("eol detects lf", ftp_parse_has_eol("LIST /pub\nNOOP"));
}

int main(void)
{
	test_epsv();
	test_pasv();
	test_eol();

	if (failures)
	{
		printf("%d ftp_parse test(s) failed\n", failures);
		return EXIT_FAILURE;
	}

	printf("ftp_parse tests passed\n");
	return EXIT_SUCCESS;
}
