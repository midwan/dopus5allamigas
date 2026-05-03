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

void main(int argc, char **argv)
{
	struct RexxMsg *msg;
	struct MsgPort *port, *dopus;

	// Allocate message
	if (!(msg = AllocVec(sizeof(struct RexxMsg), MEMF_CLEAR)))
		exit(0);

	// Create port
	if (!(port = CreateMsgPort()))
	{
		FreeVec(msg);
		exit(0);
	}

	// Set reply port
	msg->rm_Node.mn_ReplyPort = port;

	// Set pointer to arg string
	msg->rm_Args[0] = argv[2];

	// Find port
	Forbid();
	if (dopus = FindPort(argv[1]))
	{
		// Send message
		PutMsg(dopus, (struct Message *)msg);
	}
	Permit();

	// Wait for reply?
	if (dopus)
	{
		WaitPort(port);
		GetMsg(port);
	}

	// Free message and port
	FreeVec(msg);
	DeleteMsgPort(port);

	exit(0);
}
