/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#ifndef _FTP_PROTOCOL_H
#define _FTP_PROTOCOL_H

enum {
	FTP_PROTOCOL_FTP,
	FTP_PROTOCOL_SFTP,
};

const char *ftp_protocol_name(int protocol);
int ftp_protocol_default_port(int protocol);
int ftp_protocol_from_text(const char *text, int *protocol);
int ftp_protocol_from_url_scheme(const char *url, const char **without_scheme, int *protocol);
int ftp_protocol_infer_from_connection(int protocol, int port, int anonymous);

#endif /* _FTP_PROTOCOL_H */
