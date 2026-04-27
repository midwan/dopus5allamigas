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
	check_next("mlsd falls back to dgux default", FTP_LISTCMD_MLSD, FTP_LISTCMD_DGUX, FTP_LISTCMD_DGUX);
	check_next("default falls back to plain list", FTP_LISTCMD_DEFAULT, FTP_LISTCMD_DEFAULT, FTP_LISTCMD_PLAIN);
	check_next("dgux default falls back to plain list", FTP_LISTCMD_DGUX, FTP_LISTCMD_DGUX, FTP_LISTCMD_PLAIN);
	check_next("empty default uses default list", FTP_LISTCMD_MLSD, "", FTP_LISTCMD_DEFAULT);

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

	if (failures)
	{
		printf("ftp_listcmd tests failed: %d\n", failures);
		return EXIT_FAILURE;
	}

	printf("ftp_listcmd tests passed\n");
	return EXIT_SUCCESS;
}
