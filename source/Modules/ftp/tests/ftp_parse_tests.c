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

static void check_mask(const char *name, unsigned int actual, unsigned int expected)
{
	if ((actual & expected) != expected)
	{
		printf("FAIL: %s: got mask 0x%x expected 0x%x\n", name, actual, expected);
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

static void test_feat(void)
{
	unsigned int caps;

	caps = ftp_parse_feat_reply("211-Features:\r\n"
								" UTF8\r\n"
								" EPSV\r\n"
								" MDTM\r\n"
								" REST STREAM\r\n"
								" SIZE\r\n"
								" MLST type*;size*;modify*;\r\n"
								"211 End\r\n");

	check_mask("feat parses standard multiline reply",
			   caps,
			   FTP_PARSE_FEAT_UTF8 | FTP_PARSE_FEAT_EPSV | FTP_PARSE_FEAT_MDTM |
				   FTP_PARSE_FEAT_REST_STREAM | FTP_PARSE_FEAT_SIZE | FTP_PARSE_FEAT_MLST);

	caps = ftp_parse_feat_reply("211-Extensions supported:\n"
								" mLsD\n"
								" Rest Stream\n"
								"211 End\n");
	check_mask("feat parses mixed-case extensions",
			   caps,
			   FTP_PARSE_FEAT_MLST | FTP_PARSE_FEAT_REST_STREAM);

	check_true("feat parses coded feature line",
			   ftp_parse_feat_line("211 EPSV") & FTP_PARSE_FEAT_EPSV);
	check_false("feat ignores reply banner",
				ftp_parse_feat_line("211-Features:") & FTP_PARSE_FEAT_MLST);
	check_false("feat rejects feature prefix",
				ftp_parse_feat_line(" SIZES") & FTP_PARSE_FEAT_SIZE);
	check_false("feat requires rest stream mode",
				ftp_parse_feat_line(" REST") & FTP_PARSE_FEAT_REST_STREAM);
}

static void test_ipv4_scope(void)
{
	check_false("ipv4 accepts global", ftp_parse_ipv4_is_non_global(8, 8, 8, 8));
	check_false("ipv4 accepts adjacent public range",
				ftp_parse_ipv4_is_non_global(172, 32, 0, 1));
	check_false("ipv4 accepts public cgnat neighbor",
				ftp_parse_ipv4_is_non_global(100, 128, 0, 1));

	check_true("ipv4 rejects zero network", ftp_parse_ipv4_is_non_global(0, 0, 0, 0));
	check_true("ipv4 rejects private 10", ftp_parse_ipv4_is_non_global(10, 1, 2, 3));
	check_true("ipv4 rejects private 172", ftp_parse_ipv4_is_non_global(172, 16, 0, 1));
	check_true("ipv4 rejects private 192", ftp_parse_ipv4_is_non_global(192, 168, 1, 1));
	check_true("ipv4 rejects loopback", ftp_parse_ipv4_is_non_global(127, 0, 0, 1));
	check_true("ipv4 rejects link local", ftp_parse_ipv4_is_non_global(169, 254, 1, 1));
	check_true("ipv4 rejects cgnat", ftp_parse_ipv4_is_non_global(100, 64, 0, 1));
	check_true("ipv4 rejects test net", ftp_parse_ipv4_is_non_global(192, 0, 2, 1));
	check_true("ipv4 rejects multicast", ftp_parse_ipv4_is_non_global(224, 0, 0, 1));
	check_true("ipv4 rejects octet overflow", ftp_parse_ipv4_is_non_global(256, 0, 0, 1));
}

static void check_pasv_peer(const char *name,
							unsigned int p0,
							unsigned int p1,
							unsigned int p2,
							unsigned int p3,
							unsigned int r0,
							unsigned int r1,
							unsigned int r2,
							unsigned int r3,
							int expected)
{
	unsigned int pasv[4];
	unsigned int peer[4];

	pasv[0] = p0;
	pasv[1] = p1;
	pasv[2] = p2;
	pasv[3] = p3;
	peer[0] = r0;
	peer[1] = r1;
	peer[2] = r2;
	peer[3] = r3;

	if (expected)
		check_true(name, ftp_parse_pasv_should_use_control_peer(pasv, peer));
	else
		check_false(name, ftp_parse_pasv_should_use_control_peer(pasv, peer));
}

static void test_pasv_peer(void)
{
	check_pasv_peer("pasv keeps matching peer", 203, 0, 113, 1, 203, 0, 113, 1, 0);
	check_pasv_peer("pasv keeps public alternate host", 1, 1, 1, 1, 8, 8, 8, 8, 0);
	check_pasv_peer("pasv keeps private peer and private host", 192, 168, 1, 9, 10, 0, 0, 1, 0);

	check_pasv_peer("pasv rewrites zero host", 0, 0, 0, 0, 8, 8, 8, 8, 1);
	check_pasv_peer("pasv rewrites private host", 10, 0, 0, 5, 8, 8, 8, 8, 1);
	check_pasv_peer("pasv rewrites loopback host", 127, 0, 0, 1, 8, 8, 8, 8, 1);
	check_pasv_peer("pasv rewrites link local host", 169, 254, 1, 2, 8, 8, 8, 8, 1);
	check_pasv_peer("pasv rewrites multicast host", 224, 0, 0, 1, 8, 8, 8, 8, 1);
}

int main(void)
{
	test_epsv();
	test_pasv();
	test_eol();
	test_feat();
	test_ipv4_scope();
	test_pasv_peer();

	if (failures)
	{
		printf("%d ftp_parse test(s) failed\n", failures);
		return EXIT_FAILURE;
	}

	printf("ftp_parse tests passed\n");
	return EXIT_SUCCESS;
}
