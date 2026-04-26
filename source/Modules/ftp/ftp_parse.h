/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#ifndef _FTP_PARSE_H
#define _FTP_PARSE_H

enum {
	FTP_PARSE_FEAT_MLST = 1u << 0,
	FTP_PARSE_FEAT_EPSV = 1u << 1,
	FTP_PARSE_FEAT_SIZE = 1u << 2,
	FTP_PARSE_FEAT_MDTM = 1u << 3,
	FTP_PARSE_FEAT_REST_STREAM = 1u << 4,
	FTP_PARSE_FEAT_UTF8 = 1u << 5,
};

int ftp_parse_has_eol(const char *text);
unsigned int ftp_parse_feat_line(const char *line);
unsigned int ftp_parse_feat_reply(const char *reply);
int ftp_parse_epsv_port(const char *buf, unsigned int *port);
int ftp_parse_pasv_tuple(const char *buf, unsigned int values[6]);
int ftp_parse_ipv4_is_non_global(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
int ftp_parse_pasv_should_use_control_peer(const unsigned int pasv[4], const unsigned int peer[4]);

#endif /* _FTP_PARSE_H */
