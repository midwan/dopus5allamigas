/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

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
	#include <openssl/x509_vfy.h>
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
	#include <openssl/x509_vfy.h>

	#define FTP_TLS_HAVE_OPENSSL_API
#endif

#if defined(FTP_TLS_BACKEND_AMISSL)
int ftp_tls_backend_acquire(void)
{
	if (!ftp_tls_amissl_open())
		return 0;

	++ftp_tls_amissl_users;
	return 1;
}
#elif defined(FTP_TLS_BACKEND_OPENSSL)
int ftp_tls_backend_acquire(void)
{
	SSL_library_init();
	SSL_load_error_strings();
	return 1;
}
#else
int ftp_tls_backend_acquire(void)
{
	return 0;
}
#endif

#if defined(FTP_TLS_BACKEND_AMISSL) || defined(FTP_TLS_BACKEND_OPENSSL)
void ftp_tls_backend_release(void)
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
#else
void ftp_tls_backend_release(void)
{
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

static int ftp_tls_text_starts_with(const char *text, const char *prefix)
{
	if (!text || !prefix)
		return 0;

	while (*prefix)
	{
		if (tolower((unsigned char)*text) != tolower((unsigned char)*prefix))
			return 0;

		++text;
		++prefix;
	}

	return 1;
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
		ftp_tls_text_equals(text, "plain") || ftp_tls_text_equals(text, "none") || ftp_tls_text_equals(text, "no") ||
		ftp_tls_text_equals(text, "false") || ftp_tls_text_equals(text, "0"))
	{
		if (mode)
			*mode = FTP_TLS_MODE_OFF;
		return 1;
	}

	if (ftp_tls_text_equals(text, "explicit") || ftp_tls_text_equals(text, "ftps") ||
		ftp_tls_text_equals(text, "ftpes") || ftp_tls_text_equals(text, "tls") ||
		ftp_tls_text_equals(text, "auth tls") || ftp_tls_text_equals(text, "starttls") ||
		ftp_tls_text_equals(text, "on") || ftp_tls_text_equals(text, "yes") || ftp_tls_text_equals(text, "true") ||
		ftp_tls_text_equals(text, "1"))
	{
		if (mode)
			*mode = FTP_TLS_MODE_EXPLICIT;
		return 1;
	}

	return 0;
}

int ftp_tls_mode_from_url_scheme(const char *url, const char **without_scheme, int *mode)
{
	if (ftp_tls_text_starts_with(url, "ftp://"))
	{
		if (without_scheme)
			*without_scheme = url + 6;
		if (mode)
			*mode = FTP_TLS_MODE_OFF;
		return 1;
	}

	if (ftp_tls_text_starts_with(url, "ftps://"))
	{
		if (without_scheme)
			*without_scheme = url + 7;
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

int ftp_tls_modes_allow_server_transfer(int source_mode, int dest_mode)
{
	return !ftp_tls_mode_uses_data_tls(source_mode) && !ftp_tls_mode_uses_data_tls(dest_mode);
}

void ftp_tls_session_init(struct ftp_tls_session *session)
{
	if (!session)
		return;

	session->ctx = NULL;
	session->ssl = NULL;
	session->socket = -1;
	session->active = 0;
	session->last_error = FTP_TLS_ERROR_NONE;
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

int ftp_tls_session_error(const struct ftp_tls_session *session)
{
	return session ? session->last_error : FTP_TLS_ERROR_NONE;
}

static int ftp_tls_connect_fail(struct ftp_tls_session *session, int error)
{
	if (session)
		session->last_error = error;

	return 0;
}

#if defined(FTP_TLS_HAVE_OPENSSL_API)
static int ftp_tls_set_verify_name(SSL *ssl, const char *host)
{
	X509_VERIFY_PARAM *param;

	if (!ssl || !host || !*host || !(param = SSL_get0_param(ssl)))
		return 0;

	if (X509_VERIFY_PARAM_set1_ip_asc(param, host))
		return 1;

	return SSL_set1_host(ssl, host);
}

static SSL_CTX *ftp_tls_new_context(int verify_peer, int *error)
{
	SSL_CTX *ctx;

	if (!(ctx = SSL_CTX_new(TLS_client_method())))
	{
		if (error)
			*error = FTP_TLS_ERROR_CONTEXT;
		return NULL;
	}

	SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_CLIENT);
	SSL_CTX_set_verify(ctx, verify_peer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);

	if (verify_peer && !SSL_CTX_set_default_verify_paths(ctx))
	{
		SSL_CTX_free(ctx);
		if (error)
			*error = FTP_TLS_ERROR_VERIFY_PATHS;
		return NULL;
	}

	if (error)
		*error = FTP_TLS_ERROR_NONE;

	return ctx;
}

static SSL_CTX *ftp_tls_context_for_session(const struct ftp_tls_session *reuse_session, int verify_peer, int *error)
{
	if (reuse_session && reuse_session->active && reuse_session->ctx &&
		SSL_CTX_up_ref((SSL_CTX *)reuse_session->ctx))
	{
		if (error)
			*error = FTP_TLS_ERROR_NONE;
		return (SSL_CTX *)reuse_session->ctx;
	}

	return ftp_tls_new_context(verify_peer, error);
}

static SSL_SESSION *ftp_tls_get_reusable_session(const struct ftp_tls_session *reuse_session)
{
	if (!reuse_session || !reuse_session->active || !reuse_session->ssl)
		return NULL;

	return SSL_get1_session((SSL *)reuse_session->ssl);
}
#endif

int ftp_tls_connect_reuse(struct ftp_tls_session *session,
						  int socket,
						  const char *host,
						  int verify_peer,
						  const struct ftp_tls_session *reuse_session)
{
	const struct ftp_tls_session *resume_source = reuse_session == session ? NULL : reuse_session;

	if (!session)
		return ftp_tls_connect_fail(session, FTP_TLS_ERROR_CONTEXT);

	ftp_tls_session_cleanup(session);

	if (socket < 0)
		return ftp_tls_connect_fail(session, FTP_TLS_ERROR_CONTEXT);

#if defined(FTP_TLS_HAVE_OPENSSL_API)
	SSL_CTX *ctx;
	SSL *ssl;
	SSL_SESSION *resume_session = NULL;
	int context_error = FTP_TLS_ERROR_NONE;

	if (!ftp_tls_backend_acquire())
		return ftp_tls_connect_fail(session, FTP_TLS_ERROR_BACKEND);

	if (!(ctx = ftp_tls_context_for_session(resume_source, verify_peer, &context_error)))
	{
		ftp_tls_backend_release();
		return ftp_tls_connect_fail(session, context_error);
	}

	if (!(ssl = SSL_new(ctx)))
	{
		SSL_CTX_free(ctx);
		ftp_tls_backend_release();
		return ftp_tls_connect_fail(session, FTP_TLS_ERROR_CONTEXT);
	}

	SSL_set_verify(ssl, verify_peer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);

	if (host && *host)
		SSL_set_tlsext_host_name(ssl, host);

	if (verify_peer && !ftp_tls_set_verify_name(ssl, host))
	{
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ftp_tls_backend_release();
		return ftp_tls_connect_fail(session, FTP_TLS_ERROR_HOSTNAME);
	}

	if ((resume_session = ftp_tls_get_reusable_session(resume_source)))
	{
		if (!SSL_set_session(ssl, resume_session))
		{
			SSL_SESSION_free(resume_session);
			SSL_free(ssl);
			SSL_CTX_free(ctx);
			ftp_tls_backend_release();
			return ftp_tls_connect_fail(session, FTP_TLS_ERROR_CONTEXT);
		}
	}

	if (!SSL_set_fd(ssl, socket) || SSL_connect(ssl) != 1)
	{
		int error = FTP_TLS_ERROR_HANDSHAKE;

		if (verify_peer && SSL_get_verify_result(ssl) != X509_V_OK)
			error = FTP_TLS_ERROR_VERIFY_RESULT;

		if (resume_session)
			SSL_SESSION_free(resume_session);
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ftp_tls_backend_release();
		return ftp_tls_connect_fail(session, error);
	}

	if (verify_peer && SSL_get_verify_result(ssl) != X509_V_OK)
	{
		if (resume_session)
			SSL_SESSION_free(resume_session);
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ftp_tls_backend_release();
		return ftp_tls_connect_fail(session, FTP_TLS_ERROR_VERIFY_RESULT);
	}

	if (resume_session)
		SSL_SESSION_free(resume_session);

	session->ctx = ctx;
	session->ssl = ssl;
	session->socket = socket;
	session->active = 1;
	return 1;
#else
	(void)socket;
	(void)host;
	(void)verify_peer;
	(void)resume_source;
	return ftp_tls_connect_fail(session, FTP_TLS_ERROR_BACKEND);
#endif
}

int ftp_tls_connect(struct ftp_tls_session *session, int socket, const char *host, int verify_peer)
{
	return ftp_tls_connect_reuse(session, socket, host, verify_peer, NULL);
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

int ftp_tls_pending(struct ftp_tls_session *session)
{
#if defined(FTP_TLS_HAVE_OPENSSL_API)
	if (!session || !session->active || !session->ssl)
		return 0;

	return SSL_pending((SSL *)session->ssl);
#else
	(void)session;
	return 0;
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
