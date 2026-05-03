/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#ifndef _FTP_LISTCMD_H
#define _FTP_LISTCMD_H

#include <stddef.h>

#define FTP_LISTCMD_DEFAULT "LIST -alF"
#define FTP_LISTCMD_DGUX "NLST -alF"
#define FTP_LISTCMD_MLSD "MLSD"
#define FTP_LISTCMD_PLAIN "LIST"

int ftp_listcmd_next_after_failure(const char *current_cmd,
								   const char *default_cmd,
								   char *next_cmd,
								   size_t next_cmd_size);
int ftp_listcmd_result_after_reply(int current_result, int final_reply);
int ftp_listcmd_is_mlsd(const char *cmd);

#endif /* _FTP_LISTCMD_H */
