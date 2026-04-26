/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#include "ftp_tls.h"

#include <ctype.h>
#include <string.h>

#if defined(__amigaos3__) || defined(__amigaos4__)
	#define FTP_TLS_BACKEND_AMISSL
#elif defined(__MORPHOS__)
	#define FTP_TLS_BACKEND_OPENSSL
#endif

#if defined(FTP_TLS_BACKEND_AMISSL)
	#include "ftp.h"
	#include "ftp_opusftp.h"

	#include <libraries/amisslmaster.h>
	#include <proto/amisslmaster.h>
	#include <proto/amissl.h>
	#include <proto/exec.h>

struct Library *AmiSSLBase = NULL;
struct Library *AmiSSLExtBase = NULL;
struct Library *AmiSSLMasterBase = NULL;
	#if defined(__amigaos4__)
struct AmiSSLIFace *IAmiSSL = NULL;
struct AmiSSLMasterIFace *IAmiSSLMaster = NULL;
	#endif
static int ftp_tls_amissl_users = 0;

static int ftp_tls_amissl_open(void)
{
	LONG err;

	#if defined(__amigaos4__)
	if (IAmiSSL)
		return 1;
	#else
	if (AmiSSLBase)
		return 1;
	#endif

	if (!SocketBase)
		return 0;

	if (!(AmiSSLMasterBase = OpenLibrary("amisslmaster.library", AMISSLMASTER_MIN_VERSION)))
		return 0;

	#if defined(__amigaos4__)
	if (!(IAmiSSLMaster = (struct AmiSSLMasterIFace *)GetInterface(AmiSSLMasterBase, "main", 1, NULL)))
	{
		CloseLibrary(AmiSSLMasterBase);
		AmiSSLMasterBase = NULL;
		return 0;
	}

	err = OpenAmiSSLTags(AMISSL_CURRENT_VERSION,
						 AmiSSL_UsesOpenSSLStructs,
						 1,
						 AmiSSL_GetIAmiSSL,
						 &IAmiSSL,
						 AmiSSL_ISocket,
						 ISocket,
						 TAG_DONE);
	#else
	err = OpenAmiSSLTags(AMISSL_CURRENT_VERSION,
						 AmiSSL_UsesOpenSSLStructs,
						 1,
						 AmiSSL_GetAmiSSLBase,
						 &AmiSSLBase,
						 AmiSSL_GetAmiSSLExtBase,
						 &AmiSSLExtBase,
						 AmiSSL_SocketBase,
						 SocketBase,
						 TAG_DONE);
	#endif

	if (err)
	{
		#if defined(__amigaos4__)
		if (IAmiSSLMaster)
		{
			DropInterface((struct Interface *)IAmiSSLMaster);
			IAmiSSLMaster = NULL;
		}
		#endif

		CloseLibrary(AmiSSLMasterBase);
		AmiSSLMasterBase = NULL;
		return 0;
	}

	return 1;
}

static void ftp_tls_amissl_close(void)
{
	#if defined(__amigaos4__)
	if (IAmiSSL)
	{
		CloseAmiSSL();
		IAmiSSL = NULL;
	}
	#else
	if (AmiSSLBase)
	{
		CloseAmiSSL();
		AmiSSLBase = NULL;
		AmiSSLExtBase = NULL;
	}
	#endif

	#if defined(__amigaos4__)
	if (IAmiSSLMaster)
	{
		DropInterface((struct Interface *)IAmiSSLMaster);
		IAmiSSLMaster = NULL;
	}
	#endif

	if (AmiSSLMasterBase)
	{
		CloseLibrary(AmiSSLMasterBase);
		AmiSSLMasterBase = NULL;
	}
}

	#define FTP_TLS_HAVE_OPENSSL_API
#elif defined(FTP_TLS_BACKEND_OPENSSL)
	#include <openssl/err.h>
	#include <openssl/ssl.h>

	#define FTP_TLS_HAVE_OPENSSL_API
#endif

#if defined(FTP_TLS_BACKEND_AMISSL)
static int ftp_tls_backend_acquire(void)
{
	if (!ftp_tls_amissl_open())
		return 0;

	++ftp_tls_amissl_users;
	return 1;
}
#elif defined(FTP_TLS_BACKEND_OPENSSL)
static int ftp_tls_backend_acquire(void)
{
	SSL_library_init();
	SSL_load_error_strings();
	return 1;
}
#endif

#if defined(FTP_TLS_BACKEND_AMISSL) || defined(FTP_TLS_BACKEND_OPENSSL)
static void ftp_tls_backend_release(void)
{
#if defined(FTP_TLS_BACKEND_AMISSL)
	if (ftp_tls_amissl_users > 0)
	{
		--ftp_tls_amissl_users;
		if (ftp_tls_amissl_users == 0)
			ftp_tls_amissl_close();
	}
#endif
}
#endif

static int ftp_tls_text_equals(const char *text, const char *word)
{
	const char *end;

	while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n')
		++text;

	end = text + strlen(text);
	while (end > text && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
		--end;

	while (text < end && *word)
	{
		if (tolower((unsigned char)*text) != tolower((unsigned char)*word))
			return 0;

		++text;
		++word;
	}

	return text == end && *word == 0;
}

const char *ftp_tls_backend_name(void)
{
#if defined(FTP_TLS_BACKEND_AMISSL)
	return "AmiSSL";
#elif defined(FTP_TLS_BACKEND_OPENSSL)
	return "OpenSSL";
#else
	return "none";
#endif
}

const char *ftp_tls_mode_name(int mode)
{
	switch (mode)
	{
	case FTP_TLS_MODE_OFF:
		return "off";
	case FTP_TLS_MODE_EXPLICIT:
		return "explicit";
	}

	return "unknown";
}

int ftp_tls_mode_from_text(const char *text, int *mode)
{
	if (!text || !*text || ftp_tls_text_equals(text, "off") || ftp_tls_text_equals(text, "ftp") ||
		ftp_tls_text_equals(text, "none") || ftp_tls_text_equals(text, "0"))
	{
		if (mode)
			*mode = FTP_TLS_MODE_OFF;
		return 1;
	}

	if (ftp_tls_text_equals(text, "explicit") || ftp_tls_text_equals(text, "ftps") ||
		ftp_tls_text_equals(text, "tls") || ftp_tls_text_equals(text, "auth tls") ||
		ftp_tls_text_equals(text, "1"))
	{
		if (mode)
			*mode = FTP_TLS_MODE_EXPLICIT;
		return 1;
	}

	return 0;
}

int ftp_tls_mode_uses_control_tls(int mode)
{
	return mode == FTP_TLS_MODE_EXPLICIT;
}

int ftp_tls_mode_uses_data_tls(int mode)
{
	return mode == FTP_TLS_MODE_EXPLICIT;
}

void ftp_tls_session_init(struct ftp_tls_session *session)
{
	if (!session)
		return;

	session->ctx = NULL;
	session->ssl = NULL;
	session->socket = -1;
	session->active = 0;
}

void ftp_tls_session_cleanup(struct ftp_tls_session *session)
{
#if defined(FTP_TLS_HAVE_OPENSSL_API)
	int was_active;
#endif

	if (!session)
		return;

#if defined(FTP_TLS_HAVE_OPENSSL_API)
	was_active = session->active;

	if (session->ssl)
	{
		SSL_shutdown((SSL *)session->ssl);
		SSL_free((SSL *)session->ssl);
	}

	if (session->ctx)
		SSL_CTX_free((SSL_CTX *)session->ctx);
#endif

	ftp_tls_session_init(session);

#if defined(FTP_TLS_HAVE_OPENSSL_API)
	if (was_active)
		ftp_tls_backend_release();
#endif
}

int ftp_tls_connect(struct ftp_tls_session *session, int socket, const char *host, int verify_peer)
{
#if defined(FTP_TLS_HAVE_OPENSSL_API)
	SSL_CTX *ctx;
	SSL *ssl;

	if (!session || socket < 0)
		return 0;

	ftp_tls_session_cleanup(session);

	if (!ftp_tls_backend_acquire())
		return 0;

	if (!(ctx = SSL_CTX_new(TLS_client_method())))
	{
		ftp_tls_backend_release();
		return 0;
	}

	SSL_CTX_set_verify(ctx, verify_peer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);

	if (!(ssl = SSL_new(ctx)))
	{
		SSL_CTX_free(ctx);
		ftp_tls_backend_release();
		return 0;
	}

	if (host && *host)
		SSL_set_tlsext_host_name(ssl, host);

	if (!SSL_set_fd(ssl, socket) || SSL_connect(ssl) != 1)
	{
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ftp_tls_backend_release();
		return 0;
	}

	session->ctx = ctx;
	session->ssl = ssl;
	session->socket = socket;
	session->active = 1;
	return 1;
#else
	(void)session;
	(void)socket;
	(void)host;
	(void)verify_peer;
	return 0;
#endif
}

int ftp_tls_read(struct ftp_tls_session *session, void *buf, int len)
{
#if defined(FTP_TLS_HAVE_OPENSSL_API)
	if (!session || !session->active || !session->ssl || !buf || len <= 0)
		return -1;

	return SSL_read((SSL *)session->ssl, buf, len);
#else
	(void)session;
	(void)buf;
	(void)len;
	return -1;
#endif
}

int ftp_tls_write(struct ftp_tls_session *session, const void *buf, int len)
{
#if defined(FTP_TLS_HAVE_OPENSSL_API)
	if (!session || !session->active || !session->ssl || !buf || len <= 0)
		return -1;

	return SSL_write((SSL *)session->ssl, buf, len);
#else
	(void)session;
	(void)buf;
	(void)len;
	return -1;
#endif
}
