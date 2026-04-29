/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

*/

#include "../ftp_sftp.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int failures = 0;

#define LIVE_PAYLOAD_SIZE 33000

#if defined(FTP_SFTP_BACKEND_LIBSSH2) && defined(FTP_SFTP_TESTING)
int ftp_sftp_test_known_hosts_check_line(char *line,
										 const char *host,
										 int port,
										 int key_type,
										 const char *key_hex);
int ftp_sftp_test_known_hosts_check_file(struct ftp_sftp_session *session, const char *path, int *matched);
#endif

static void check_true(const char *name, int value)
{
	if (!value)
	{
		printf("FAIL: %s\n", name);
		++failures;
	}
}

static void check_false(const char *name, int value)
{
	if (value)
	{
		printf("FAIL: %s\n", name);
		++failures;
	}
}

static void check_int(const char *name, int actual, int expected)
{
	if (actual != expected)
	{
		printf("FAIL: %s: got %d expected %d\n", name, actual, expected);
		++failures;
	}
}

static void check_string(const char *name, const char *actual, const char *expected)
{
	if (!actual || strcmp(actual, expected))
	{
		printf("FAIL: %s: got %s expected %s\n", name, actual ? actual : "(null)", expected);
		++failures;
	}
}

static void report_sftp_error(const char *operation, const struct ftp_sftp_session *session)
{
	printf("INFO: %s failed: %s (error=%d sftp_status=%lu)\n",
		   operation,
		   ftp_sftp_error_message(session),
		   ftp_sftp_session_error(session),
		   ftp_sftp_status(session));
}

static void test_session_defaults(void)
{
	struct ftp_sftp_session session;

	memset(&session, 1, sizeof(session));
	ftp_sftp_session_init(&session);

	check_true("session clears ssh", session.ssh == NULL);
	check_true("session clears sftp", session.sftp == NULL);
	check_int("session clears socket", session.socket, -1);
	check_false("session inactive", session.active);
	check_string("session cwd", session.cwd, ".");
	check_int("session error", ftp_sftp_session_error(&session), FTP_SFTP_ERROR_NONE);
	check_int("null session error", ftp_sftp_session_error(NULL), FTP_SFTP_ERROR_NONE);
}

static void test_paths(void)
{
	char path[FTP_SFTP_PATH_BUFSIZE + 1];

	check_true("absolute path", ftp_sftp_path_join(path, sizeof(path), "/home/user", "/var/tmp"));
	check_string("absolute result", path, "/var/tmp");

	check_true("root relative path", ftp_sftp_path_join(path, sizeof(path), "/", "pub"));
	check_string("root relative result", path, "/pub");

	check_true("nested relative path", ftp_sftp_path_join(path, sizeof(path), "/home/user", "pub"));
	check_string("nested relative result", path, "/home/user/pub");

	check_true("dot cwd relative path", ftp_sftp_path_join(path, sizeof(path), ".", "pub"));
	check_string("dot cwd relative result", path, "pub");

	check_true("dot keeps cwd", ftp_sftp_path_join(path, sizeof(path), "/home/user", "."));
	check_string("dot result", path, "/home/user");

	check_true("empty keeps cwd", ftp_sftp_path_join(path, sizeof(path), "/home/user", ""));
	check_string("empty result", path, "/home/user");

	check_true("parent nested", ftp_sftp_parent_path(path, sizeof(path), "/home/user/pub"));
	check_string("parent nested result", path, "/home/user");

	check_true("parent root child", ftp_sftp_parent_path(path, sizeof(path), "/home"));
	check_string("parent root child result", path, "/");

	check_true("parent root", ftp_sftp_parent_path(path, sizeof(path), "/"));
	check_string("parent root result", path, "/");

	check_false("join overflow", ftp_sftp_path_join(path, 5, "/home", "user"));
	check_false("parent overflow", ftp_sftp_parent_path(path, 5, "/home/user"));
}

static void test_time_conversion(void)
{
	check_int("unix epoch clamps", ftp_sftp_unix_to_amiga_seconds(0), 0);
	check_int("amiga epoch", ftp_sftp_unix_to_amiga_seconds(252460800UL), 0);
	check_int("one minute after amiga epoch", ftp_sftp_unix_to_amiga_seconds(252460860UL), 60);
}

static void test_missing_backend(void)
{
	struct ftp_sftp_session session;

	ftp_sftp_session_init(&session);

	if (!strcmp(ftp_sftp_backend_name(), "none"))
	{
		check_false("connect without backend", ftp_sftp_connect(&session, "example.com", 22, "user", "pass"));
		check_int("connect backend error", ftp_sftp_session_error(&session), FTP_SFTP_ERROR_BACKEND);
		check_string("connect backend message",
					 ftp_sftp_error_message(&session),
					 "SFTP support is not available in this build");

		check_false("list without backend", ftp_sftp_list(&session, ".", NULL, NULL));
		check_int("list backend error", ftp_sftp_session_error(&session), FTP_SFTP_ERROR_BACKEND);
	}
}

#if defined(FTP_SFTP_BACKEND_LIBSSH2) && defined(FTP_SFTP_TESTING)
static void test_known_hosts_lines(void)
{
	char line[256];

	strcpy(line, "dopus5-sftp\texample.com\t22\t1\tabcdef\n");
	check_int("known hosts exact match",
			  ftp_sftp_test_known_hosts_check_line(line, "example.com", 22, 1, "abcdef"),
			  1);

	strcpy(line, "dopus5-sftp\texample.com\t22\t1\tabcdef\r\n");
	check_int("known hosts crlf match",
			  ftp_sftp_test_known_hosts_check_line(line, "example.com", 22, 1, "abcdef"),
			  1);

	strcpy(line, "dopus5-sftp\texample.com\t22\t1\tabcdef\n");
	check_int("known hosts key mismatch",
			  ftp_sftp_test_known_hosts_check_line(line, "example.com", 22, 1, "123456"),
			  -1);

	strcpy(line, "dopus5-sftp\texample.com\t22\t1\tabcdef\n");
	check_int("known hosts host mismatch",
			  ftp_sftp_test_known_hosts_check_line(line, "other.example.com", 22, 1, "abcdef"),
			  0);

	strcpy(line, "dopus5-sftp\texample.com\t2222\t1\tabcdef\n");
	check_int("known hosts port mismatch",
			  ftp_sftp_test_known_hosts_check_line(line, "example.com", 22, 1, "abcdef"),
			  0);

	strcpy(line, "not-dopus\texample.com\t22\t1\tabcdef\n");
	check_int("known hosts magic mismatch",
			  ftp_sftp_test_known_hosts_check_line(line, "example.com", 22, 1, "abcdef"),
			  0);

	strcpy(line, "dopus5-sftp\texample.com\t22\n");
	check_int("known hosts malformed line",
			  ftp_sftp_test_known_hosts_check_line(line, "example.com", 22, 1, "abcdef"),
			  0);
}

static void test_known_hosts_files(void)
{
	static const char detail_prefix[] = "Could not read SFTP known-hosts file: ";
	struct ftp_sftp_session session;
	char bad_path[8192];
	int matched = 123;

	ftp_sftp_session_init(&session);
	check_true("known hosts missing file allowed",
			   ftp_sftp_test_known_hosts_check_file(&session, "/tmp/dopus5-sftp-missing-known-hosts", &matched));
	check_int("known hosts missing file unmatched", matched, 0);
	check_string("known hosts missing file detail", session.last_detail, "");

	memset(bad_path, 'x', sizeof(bad_path) - 1);
	bad_path[0] = '/';
	bad_path[sizeof(bad_path) - 1] = 0;

	matched = 123;
	check_false("known hosts open error fails", ftp_sftp_test_known_hosts_check_file(&session, bad_path, &matched));
	check_int("known hosts open error unmatched", matched, 0);
	check_true("known hosts open error detail", !strncmp(session.last_detail, detail_prefix, strlen(detail_prefix)));
}
#endif

#if defined(FTP_SFTP_BACKEND_LIBSSH2)
static int abort_connect_callback(void *userdata)
{
	int *calls = userdata;

	if (calls)
		++*calls;

	return 1;
}

static void test_connect_abort_callback(void)
{
	struct ftp_sftp_session session;
	struct ftp_sftp_connect_options options = {0};
	int abort_calls = 0;

	ftp_sftp_session_init(&session);

	options.timeout_secs = 60;
	options.abort = abort_connect_callback;
	options.userdata = &abort_calls;

	errno = 0;
	check_false("connect abort callback stops connect",
				ftp_sftp_connect_with_options(&session, "127.0.0.1", 22, "user", "pass", &options));
	check_int("connect abort callback called", abort_calls, 1);
	check_int("connect abort errno", errno, EINTR);
	check_int("connect abort error", ftp_sftp_session_error(&session), FTP_SFTP_ERROR_CONNECT);
}
#endif

static int list_count_callback(void *userdata, const struct ftp_sftp_entry *entry)
{
	int *count = userdata;

	if (entry && entry->name[0])
		++*count;

	return 1;
}

struct xfer_counter
{
	unsigned int bytes;
	int zero_updates;
};

static int xfer_count_callback(void *userdata, unsigned int total, unsigned int bytes)
{
	struct xfer_counter *counter = userdata;

	(void)total;

	if (!counter)
		return 0;

	if (bytes)
		counter->bytes += bytes;
	else
		++counter->zero_updates;

	return 0;
}

static int live_trust_hostkey_callback(void *userdata,
									   const char *host,
									   int port,
									   const char *fingerprint,
									   const char *known_hosts_path)
{
	(void)userdata;

	printf("INFO: trusting live SFTP host key for %s:%d (%s) in %s\n",
		   host ? host : "",
		   port,
		   fingerprint ? fingerprint : "unavailable",
		   known_hosts_path ? known_hosts_path : "");

	return 1;
}

static void test_optional_live_sftp(void)
{
	struct ftp_sftp_session session;
	struct ftp_sftp_entry entry;
	struct ftp_sftp_connect_options options = {0};
	char dirname[128];
	char remote_name[256];
	char renamed_name[256];
	char local_upload[256];
	char local_download[256];
	char *payload = NULL;
	char *downloaded = NULL;
	FILE *file;
	const char *host = getenv("DOPUS_SFTP_TEST_HOST");
	const char *user = getenv("DOPUS_SFTP_TEST_USER");
	const char *pass = getenv("DOPUS_SFTP_TEST_PASS");
	const char *port_text = getenv("DOPUS_SFTP_TEST_PORT");
	const char *path = getenv("DOPUS_SFTP_TEST_PATH");
	const char *write_enabled = getenv("DOPUS_SFTP_TEST_WRITE");
	const char *write_path = getenv("DOPUS_SFTP_TEST_WRITE_PATH");
	const char *known_hosts_path = getenv("DOPUS_SFTP_TEST_KNOWN_HOSTS");
	int port = port_text && *port_text ? atoi(port_text) : 22;
	int count = 0;
	int made_remote_dir = 0;
	int uploaded = 0;
	int renamed = 0;
	size_t i;
	size_t read_size;
	struct xfer_counter put_counter = {0};
	struct xfer_counter get_counter = {0};

	if (!host || !*host || !user || !*user)
		return;

	ftp_sftp_session_init(&session);
	options.known_hosts_path = known_hosts_path;
	options.hostkey = live_trust_hostkey_callback;
	check_true("live connect", ftp_sftp_connect_with_options(&session, host, port, user, pass, &options));
	if (!session.active)
	{
		report_sftp_error("live connect", &session);
		goto cleanup;
	}

	if (path && *path)
		check_true("live cwd", ftp_sftp_cwd(&session, path));

	check_true("live pwd stat", ftp_sftp_stat(&session, ".", &entry));
	check_true("live pwd is directory", entry.type >= 0);
	check_true("live list", ftp_sftp_list(&session, ".", list_count_callback, &count));

	if (!write_enabled || !*write_enabled || !strcmp(write_enabled, "0"))
		goto cleanup;

	if (write_path && !*write_path)
	{
		printf("SKIP: live write test needs DOPUS_SFTP_TEST_WRITE_PATH or an unset WRITE_PATH\n");
		goto cleanup;
	}

	if (write_path && *write_path)
	{
		int ok = ftp_sftp_cwd(&session, write_path);
		check_true("live write cwd", ok);
		if (!ok)
		{
			report_sftp_error("live write cwd", &session);
			goto cleanup;
		}
	}

	snprintf(dirname, sizeof(dirname), "dopus-sftp-test-%lu", (unsigned long)time(NULL));
	snprintf(remote_name, sizeof(remote_name), "%s/upload.txt", dirname);
	snprintf(renamed_name, sizeof(renamed_name), "%s/renamed.txt", dirname);
	snprintf(local_upload, sizeof(local_upload), "/tmp/%s-upload.txt", dirname);
	snprintf(local_download, sizeof(local_download), "/tmp/%s-download.txt", dirname);

	payload = malloc(LIVE_PAYLOAD_SIZE + 1);
	check_true("live allocate payload", payload != NULL);
	if (!payload)
		goto cleanup;
	for (i = 0; i < LIVE_PAYLOAD_SIZE; ++i)
		payload[i] = 'A' + (int)(i % 26);
	payload[LIVE_PAYLOAD_SIZE - 1] = '\n';
	payload[LIVE_PAYLOAD_SIZE] = 0;

	file = fopen(local_upload, "wb");
	check_true("live create local upload", file != NULL);
	if (!file)
		goto cleanup;
	fwrite(payload, 1, LIVE_PAYLOAD_SIZE, file);
	fclose(file);

	made_remote_dir = ftp_sftp_mkdir(&session, dirname);
	check_true("live mkdir", made_remote_dir);
	if (!made_remote_dir)
	{
		report_sftp_error("live mkdir", &session);
		goto cleanup;
	}

	uploaded = ftp_sftp_put(&session, xfer_count_callback, &put_counter, local_upload, remote_name, 0) ==
			   LIVE_PAYLOAD_SIZE;
	check_true("live put", uploaded);
	check_int("live put progress", put_counter.bytes, LIVE_PAYLOAD_SIZE);
	if (!uploaded)
		goto cleanup;

	memset(&entry, 0, sizeof(entry));
	check_true("live stat uploaded", ftp_sftp_stat(&session, remote_name, &entry));
	check_int("live uploaded size", entry.size, LIVE_PAYLOAD_SIZE);
	check_true("live chmod", ftp_sftp_chmod(&session, remote_name, 0644));
	renamed = ftp_sftp_rename(&session, remote_name, renamed_name);
	check_true("live rename", renamed);
	if (!renamed)
		goto cleanup;

	memset(&entry, 0, sizeof(entry));
	check_true("live stat renamed", ftp_sftp_stat(&session, renamed_name, &entry));
	check_true("live get",
			   ftp_sftp_get(&session, xfer_count_callback, &get_counter, renamed_name, local_download, 0) ==
				   LIVE_PAYLOAD_SIZE);
	check_int("live get progress", get_counter.bytes, LIVE_PAYLOAD_SIZE);

	file = fopen(local_download, "rb");
	check_true("live open downloaded", file != NULL);
	if (file)
	{
		downloaded = malloc(LIVE_PAYLOAD_SIZE + 1);
		check_true("live allocate downloaded", downloaded != NULL);
		if (downloaded)
		{
			read_size = fread(downloaded, 1, LIVE_PAYLOAD_SIZE, file);
			downloaded[read_size] = 0;
		}
		fclose(file);
		if (downloaded)
			check_string("live downloaded payload", downloaded, payload);
	}

	check_true("live delete", ftp_sftp_delete(&session, renamed_name));
	uploaded = 0;
	renamed = 0;

cleanup:
	if (session.active)
	{
		if (renamed)
			ftp_sftp_delete(&session, renamed_name);
		else if (uploaded)
			ftp_sftp_delete(&session, remote_name);
	}
	if (made_remote_dir)
		check_true("live rmdir", ftp_sftp_rmdir(&session, dirname));
		else if (session.active)
			ftp_sftp_rmdir(&session, dirname);
		free(payload);
		free(downloaded);
		remove(local_upload);
		remove(local_download);

	ftp_sftp_session_cleanup(&session);
}

int main(void)
{
	test_session_defaults();
	test_paths();
	test_time_conversion();
	test_missing_backend();
#if defined(FTP_SFTP_BACKEND_LIBSSH2)
	#if defined(FTP_SFTP_TESTING)
	test_known_hosts_lines();
	test_known_hosts_files();
	#endif
	test_connect_abort_callback();
#endif
	test_optional_live_sftp();

	if (failures)
	{
		printf("%d ftp_sftp test(s) failed\n", failures);
		return EXIT_FAILURE;
	}

	printf("ftp_sftp tests passed (%s backend)\n", ftp_sftp_backend_name());
	return EXIT_SUCCESS;
}
