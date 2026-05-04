/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

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

#ifndef _FTP_UTF8_H
#define _FTP_UTF8_H

// Forward declarations
struct ftp_info;
struct entry_info;

// UTF-8 conversion functions
char *ftp_utf8_to_local(const char *utf8_name);
char *ftp_local_to_utf8(const char *local_name);
void ftp_convert_filename_from_utf8(struct entry_info *entry, struct ftp_info *info);
char *ftp_convert_path_to_utf8(const char *local_path, struct ftp_info *info);
void ftp_convert_inplace_utf8_to_local(char *buf, int buf_size, struct ftp_info *info);
void ftp_store_utf8_path_as_local(char *dest, int dest_size, const char *utf8_path, struct ftp_info *info);

// Utility functions
BOOL ftp_codesets_available(void);
BOOL ftp_is_utf8_server(struct ftp_info *info);
void ftp_codesets_free(void *str);

#endif /* _FTP_UTF8_H */
