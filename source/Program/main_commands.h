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

enum {
	MAINCMD_BASE = 0x40000,
	MAINCMD_RESET,			  // Reset display
	MAINCMD_CLOSE_DISPLAY,	  // Close display
	MAINCMD_OPEN_DISPLAY,	  // Open display
	MAINCMD_GET_SCREEN_DATA,  // Get screen data
	MAINCMD_RESET_TOOLMENU,	  // Reset Tools menu
	MAINCMD_ADD_APPWINDOW,
	MAINCMD_REM_APPWINDOW,
	GROUP_NEW_FONT,		 // New font for Groups
	GROUP_NEW_NAME,		 // Group's name changed
	GROUP_ADD_ICON,		 // Show icon in group
	GROUP_NEW_BACKFILL,	 // Backfill pattern changed

	REXXCMD_INCREMENT_MSG,	// Increment message count

	GROUP_DELETE,  // Delete from group

	MAINCMD_COMMAND,  // Command

	MAINCMD_GET_LIST,  // Get command list

	MAINCMD_SAVE_POSITIONS,	 // Save position list
	MAINCMD_LOAD_POSITIONS,	 // Load position list

	REXXCMD_SEND_MSG,	 // Send a REXX command
	REXXCMD_SEND_RXMSG,	 // Send a REXX message

	MAINCMD_GET_PEN,	  // Get custom pen
	MAINCMD_RELEASE_PEN,  // Free custom pen

	MAINCMD_GET_ICON,  // Get an icon

	MAINCMD_SAVE_ENV,  // Save environment
};
