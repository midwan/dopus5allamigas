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

#ifndef _FTP_LIST_H
#define _FTP_LIST_H

//
//	Stuff to do with parsing LIST output
//

// Sizes of fields for FTP LIST ouput conversion routines
#define OGBLEN 32
#define GBMLEN 32
#define BDLEN 12
#define SECONDSLEN 12

#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct ls_parse_buf
{
	char ls_own_grp_byt[OGBLEN + 1];	//	Could be Owner, Group, or Bytes
	char ls_grp_byt_mon[GBMLEN + 1];	//	Could be Group, Bytes, or Month
	char ls_byt_dat[BDLEN + 1];			//	Could be Bytes or Date
	char ls_name[FILENAMELEN + 2 + 3];	//	Allow for quotes + link symbol
	char ls_seconds[SECONDSLEN + 1];	//	Seconds from 1/1/78 (ascii)
	LONG ls_protect;					//	Protection flags
	char ls_comment[COMMENTLEN + 1];	//	Used for link's real path
};
#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
