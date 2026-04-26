/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#ifndef _FTP_PARSE_H
#define _FTP_PARSE_H

int ftp_parse_has_eol(const char *text);
int ftp_parse_epsv_port(const char *buf, unsigned int *port);
int ftp_parse_pasv_tuple(const char *buf, unsigned int values[6]);

#endif /* _FTP_PARSE_H */
