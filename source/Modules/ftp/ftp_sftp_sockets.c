/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#if defined(FTP_SFTP_BACKEND_LIBSSH2) && \
	(defined(__amigaos3__) || defined(__amigaos4__) || defined(__MORPHOS__) || defined(__AROS__))

#include "ftp.h"
#include "ftp_opusftp.h"

#if defined(__amigaos4__)
#include <sys/types.h>
#endif

static LONG ftp_sftp_socket_send(LONG sock, const void *buf, LONG len, LONG flags)
{
#if defined(__MORPHOS__)
	return send(sock, (const UBYTE *)buf, len, flags);
#else
	return send(sock, (APTR)buf, len, flags);
#endif
}

static LONG ftp_sftp_socket_recv(LONG sock, APTR buf, LONG len, LONG flags)
{
#if defined(__MORPHOS__)
	return recv(sock, (UBYTE *)buf, len, flags);
#else
	return recv(sock, buf, len, flags);
#endif
}

static LONG ftp_sftp_socket_select(LONG nfds,
								   fd_set *readfds,
								   fd_set *writefds,
								   fd_set *exceptfds,
								   struct timeval *timeout,
								   ULONG *signals)
{
	return WaitSelect(nfds, readfds, writefds, exceptfds, timeout, signals);
}

#ifdef send
#undef send
#endif
#ifdef recv
#undef recv
#endif
#ifdef select
#undef select
#endif
#ifdef WaitSelect
#undef WaitSelect
#endif

#if defined(__MORPHOS__)
LONG send(LONG sock, const UBYTE *buf, LONG len, LONG flags)
{
	return ftp_sftp_socket_send(sock, buf, len, flags);
}

LONG recv(LONG sock, UBYTE *buf, LONG len, LONG flags)
{
	return ftp_sftp_socket_recv(sock, buf, len, flags);
}

LONG WaitSelect(LONG nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout, ULONG *signals)
{
	return ftp_sftp_socket_select(nfds, readfds, writefds, exceptfds, timeout, signals);
}
#else
ssize_t send(int sock, const void *buf, size_t len, int flags)
{
	return (ssize_t)ftp_sftp_socket_send((LONG)sock, buf, (LONG)len, (LONG)flags);
}

ssize_t recv(int sock, void *buf, size_t len, int flags)
{
	return (ssize_t)ftp_sftp_socket_recv((LONG)sock, buf, (LONG)len, (LONG)flags);
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	return (int)ftp_sftp_socket_select((LONG)nfds, readfds, writefds, exceptfds, timeout, NULL);
}
#endif

#endif
