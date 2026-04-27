/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#include "../ftp_listcmd.h"

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

static void check_string(const char *name, const char *actual, const char *expected)
{
	if (strcmp(actual, expected))
	{
		printf("FAIL: %s: got '%s', expected '%s'\n", name, actual, expected);
		++failures;
	}
}

static void check_int(const char *name, int actual, int expected)
{
	if (actual != expected)
	{
		printf("FAIL: %s: got %d, expected %d\n", name, actual, expected);
		++failures;
	}
}

static void check_next(const char *name, const char *current, const char *fallback_default, const char *expected)
{
	char next[32] = "";

	check_true(name, ftp_listcmd_next_after_failure(current, fallback_default, next, sizeof(next)));
	check_string(name, next, expected);
}

int main(void)
{
	char next[32] = "";
	char small[4] = "";

	check_next("mlsd falls back to default", FTP_LISTCMD_MLSD, FTP_LISTCMD_DEFAULT, FTP_LISTCMD_DEFAULT);
	check_next("mlsd case insensitive", "mlsd", FTP_LISTCMD_DEFAULT, FTP_LISTCMD_DEFAULT);
	check_next("mlsd ignores padding", " \tMLSD  ", FTP_LISTCMD_DEFAULT, FTP_LISTCMD_DEFAULT);
	check_next("mlsd falls back to dgux default", FTP_LISTCMD_MLSD, FTP_LISTCMD_DGUX, FTP_LISTCMD_DGUX);
	check_next("mlsd trims padded default", FTP_LISTCMD_MLSD, " \tLIST -alF ", FTP_LISTCMD_DEFAULT);
	check_next("default falls back to plain list", FTP_LISTCMD_DEFAULT, FTP_LISTCMD_DEFAULT, FTP_LISTCMD_PLAIN);
	check_next("default ignores padding", " LIST -alF ", FTP_LISTCMD_DEFAULT, FTP_LISTCMD_PLAIN);
	check_next("dgux default falls back to plain list", FTP_LISTCMD_DGUX, FTP_LISTCMD_DGUX, FTP_LISTCMD_PLAIN);
	check_next("empty default uses default list", FTP_LISTCMD_MLSD, "", FTP_LISTCMD_DEFAULT);
	check_next("blank default uses default list", FTP_LISTCMD_MLSD, " \t ", FTP_LISTCMD_DEFAULT);

	check_false("plain list has no fallback",
				ftp_listcmd_next_after_failure(FTP_LISTCMD_PLAIN, FTP_LISTCMD_DEFAULT, next, sizeof(next)));
	check_false("same mlsd default rejected",
				ftp_listcmd_next_after_failure(FTP_LISTCMD_MLSD, FTP_LISTCMD_MLSD, next, sizeof(next)));
	check_false("custom command has no fallback",
				ftp_listcmd_next_after_failure("LIST -la", FTP_LISTCMD_DEFAULT, next, sizeof(next)));
	check_false("small output buffer rejected",
				ftp_listcmd_next_after_failure(FTP_LISTCMD_MLSD, FTP_LISTCMD_DEFAULT, small, sizeof(small)));
	check_false("null current rejected",
				ftp_listcmd_next_after_failure(NULL, FTP_LISTCMD_DEFAULT, next, sizeof(next)));

	check_int("successful final list reply", ftp_listcmd_result_after_reply(0, 226), 0);
	check_int("successful alternate final list reply", ftp_listcmd_result_after_reply(0, 250), 0);
	check_int("transient final list failure", ftp_listcmd_result_after_reply(0, 451), -2);
	check_int("permanent final list failure", ftp_listcmd_result_after_reply(0, 550), -2);
	check_int("connection final list failure", ftp_listcmd_result_after_reply(0, 421), -2);
	check_int("preserves callback failure", ftp_listcmd_result_after_reply(-1, 226), -1);
	check_int("preserves command failure", ftp_listcmd_result_after_reply(-2, 226), -2);

	check_true("detects mlsd command", ftp_listcmd_is_mlsd(FTP_LISTCMD_MLSD));
	check_true("detects mlsd case insensitive", ftp_listcmd_is_mlsd("mlsd"));
	check_true("detects padded mlsd", ftp_listcmd_is_mlsd(" MLSD\t"));
	check_false("rejects list as mlsd", ftp_listcmd_is_mlsd(FTP_LISTCMD_PLAIN));
	check_false("rejects mlsd with arguments", ftp_listcmd_is_mlsd("MLSD /pub"));

	if (failures)
	{
		printf("ftp_listcmd tests failed: %d\n", failures);
		return EXIT_FAILURE;
	}

	printf("ftp_listcmd tests passed\n");
	return EXIT_SUCCESS;
}
