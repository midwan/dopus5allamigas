/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#ifndef _FTP_SFTP_H
#define _FTP_SFTP_H

#define FTP_SFTP_PATH_BUFSIZE 256
#define FTP_SFTP_NAME_BUFSIZE 256
#define FTP_SFTP_COMMENT_BUFSIZE 80
#define FTP_SFTP_ERROR_BUFSIZE 160

enum {
	FTP_SFTP_ERROR_NONE,
	FTP_SFTP_ERROR_BACKEND,
	FTP_SFTP_ERROR_SESSION,
	FTP_SFTP_ERROR_CONNECT,
	FTP_SFTP_ERROR_AUTH,
	FTP_SFTP_ERROR_INIT,
	FTP_SFTP_ERROR_PATH,
	FTP_SFTP_ERROR_OP,
	FTP_SFTP_ERROR_HOSTKEY,
	FTP_SFTP_ERROR_ABORTED,
};

struct ftp_sftp_entry
{
	char name[FTP_SFTP_NAME_BUFSIZE + 1];
	unsigned long size;
	int type;
	unsigned long seconds;
	unsigned long unixprot;
	char comment[FTP_SFTP_COMMENT_BUFSIZE + 1];
};

struct ftp_sftp_session
{
	void *ssh;
	void *sftp;
	int socket;
	char cwd[FTP_SFTP_PATH_BUFSIZE + 1];
	int active;
	int backend_acquired;
	int last_error;
	unsigned long last_status;
	char last_detail[FTP_SFTP_ERROR_BUFSIZE + 1];
};

typedef int (*ftp_sftp_list_callback)(void *userdata, const struct ftp_sftp_entry *entry);
typedef int (*ftp_sftp_xfer_callback)(void *userdata, unsigned int total, unsigned int done);
typedef void (*ftp_sftp_connect_update_callback)(void *userdata, const char *text);
typedef int (*ftp_sftp_connect_abort_callback)(void *userdata);
typedef int (*ftp_sftp_hostkey_callback)(void *userdata,
										 const char *host,
										 int port,
										 const char *fingerprint,
										 const char *known_hosts_path);

struct ftp_sftp_connect_options
{
	int timeout_secs;
	const char *known_hosts_path;
	ftp_sftp_connect_update_callback update;
	ftp_sftp_connect_abort_callback abort;
	ftp_sftp_hostkey_callback hostkey;
	void *userdata;
};

const char *ftp_sftp_backend_name(void);
void ftp_sftp_session_init(struct ftp_sftp_session *session);
void ftp_sftp_session_cleanup(struct ftp_sftp_session *session);
int ftp_sftp_session_error(const struct ftp_sftp_session *session);
unsigned long ftp_sftp_status(const struct ftp_sftp_session *session);
const char *ftp_sftp_error_message(const struct ftp_sftp_session *session);
int ftp_sftp_connect(struct ftp_sftp_session *session, const char *host, int port, const char *user, const char *pass);
int ftp_sftp_connect_with_options(struct ftp_sftp_session *session,
								  const char *host,
								  int port,
								  const char *user,
								  const char *pass,
								  const struct ftp_sftp_connect_options *options);
int ftp_sftp_path_join(char *out, int outsize, const char *cwd, const char *path);
int ftp_sftp_parent_path(char *out, int outsize, const char *path);
unsigned long ftp_sftp_unix_to_amiga_seconds(unsigned long unix_seconds);
int ftp_sftp_cwd(struct ftp_sftp_session *session, const char *path);
int ftp_sftp_cdup(struct ftp_sftp_session *session);
int ftp_sftp_pwd(struct ftp_sftp_session *session, char *out, int outsize);
int ftp_sftp_list(struct ftp_sftp_session *session,
				  const char *path,
				  ftp_sftp_list_callback callback,
				  void *userdata);
int ftp_sftp_stat(struct ftp_sftp_session *session, const char *path, struct ftp_sftp_entry *entry);
int ftp_sftp_mkdir(struct ftp_sftp_session *session, const char *path);
int ftp_sftp_delete(struct ftp_sftp_session *session, const char *path);
int ftp_sftp_rmdir(struct ftp_sftp_session *session, const char *path);
int ftp_sftp_rename(struct ftp_sftp_session *session, const char *oldpath, const char *newpath);
int ftp_sftp_chmod(struct ftp_sftp_session *session, const char *path, unsigned long mode);
unsigned int ftp_sftp_get(struct ftp_sftp_session *session,
						  ftp_sftp_xfer_callback callback,
						  void *userdata,
						  const char *remote,
						  const char *local,
						  int restart);
unsigned int ftp_sftp_put(struct ftp_sftp_session *session,
						  ftp_sftp_xfer_callback callback,
						  void *userdata,
						  const char *local,
						  const char *remote,
						  unsigned int restart);

#endif /* _FTP_SFTP_H */
