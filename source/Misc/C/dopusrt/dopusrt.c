/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
AROS Public License for more details.

The release of Directory Opus 5 under the GPL in NO WAY affects
the existing commercial status of Directory Opus for Windows.

For more information on Directory Opus for Windows please see:

				 http://www.gpsoft.com.au

*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <signal.h>
#include <stdio.h>

#include <proto/dopus5.h>
#include <dopus/version.h>
#include <dopus/debug.h>

#include <SDI/SDI_compiler.h>

const char USED_VAR version[] = "\0$VER: DOpusRT " CMD_STRING;

int main(int argc, char **argv)
{
	struct Library *DOpusBase = NULL;
#ifdef __amigaos4__
	struct DOpusIFace *IDOpus = NULL;
#endif
	char buffer[1024] = {0};
	char *command = buffer;

	signal(SIGINT, SIG_IGN);

	// Valid arguments?
	if (argc < 2)
		return (RETURN_ERROR);

	if (argc < 3)
		sprintf(buffer, "\"%s\"", argv[1]);
	else
		sprintf(buffer, "\"%s\" \"%s\"", argv[1], argv[2]);

	// Open dopus library
	if (!(DOpusBase = OpenLibrary("dopus5:libs/dopus5.library", LIB_VERSION)))
		return (RETURN_ERROR);

#ifdef __amigaos4__
	if (!(IDOpus = (struct DOpusIFace *)GetInterface(DOpusBase, "main", 1, NULL)))
	{
		CloseLibrary(DOpusBase);
		return (RETURN_ERROR);
	}
#endif

	// Launch program
	WB_Launch(command, NULL, LAUNCH_WAIT);

// Close library
#ifdef __amigaos4__
	DropInterface((struct Interface *)IDOpus);
#endif
	CloseLibrary(DOpusBase);

	return (RETURN_OK);
}
