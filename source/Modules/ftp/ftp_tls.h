/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#ifndef _FTP_TLS_H
#define _FTP_TLS_H

enum {
	FTP_TLS_MODE_OFF,
	FTP_TLS_MODE_EXPLICIT,
};

struct ftp_tls_session
{
	void *ctx;
	void *ssl;
	int socket;
	int active;
};

const char *ftp_tls_backend_name(void);
const char *ftp_tls_mode_name(int mode);
int ftp_tls_mode_from_text(const char *text, int *mode);
int ftp_tls_mode_uses_control_tls(int mode);
int ftp_tls_mode_uses_data_tls(int mode);
void ftp_tls_session_init(struct ftp_tls_session *session);
void ftp_tls_session_cleanup(struct ftp_tls_session *session);
int ftp_tls_connect(struct ftp_tls_session *session, int socket, const char *host, int verify_peer);
int ftp_tls_read(struct ftp_tls_session *session, void *buf, int len);
int ftp_tls_write(struct ftp_tls_session *session, const void *buf, int len);

#endif /* _FTP_TLS_H */
