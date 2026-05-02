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

/*
 *	20-05-96	This file seperated from lister.c
 *	13-06-96	Now strips trailing '#' from filenames as well as '/', '*', and '@'
 */

#include "ftp.h"
#include "ftp_lister.h"
#include "ftp_arexx.h"
#include "ftp_ipc.h"
#include "ftp_opusftp.h"
#include "ftp_list.h"
#include "ftp_recursive.h"
#include "ftp_util.h"

#define min(a, b) (((a) <= (b)) ? (a) : (b))

extern const char *months[];

/********************************/

static const char *lister_default_list_command(struct ftp_node *ftpnode)
{
	if (ftpnode->fn_systype == FTP_DGUX)
		return FTP_LISTCMD_DGUX;
	if (ftpnode->fn_systype == FTP_OS2 || ftpnode->fn_systype == FTP_WINDOWSNT)
		return FTP_LISTCMD_PLAIN;

	return FTP_LISTCMD_DEFAULT;
}

static void lister_restore_list_parser(struct ftp_node *ftpnode)
{
	if (ftpnode->fn_systype == FTP_WINDOWSNT)
		ftpnode->fn_ls_to_entryinfo = nt_line_to_entryinfo;
	else
		ftpnode->fn_ls_to_entryinfo = unix_line_to_entryinfo;
}

static void lister_reset_index_state(struct ftp_node *ftpnode)
{
	ftpnode->fn_ftp.fi_found_index = 0;
	ftpnode->fn_ftp.fi_found_index_size = 0;
	ftpnode->fn_ftp.fi_found_fbbs_size = 0;
}

static void lister_clear_listing(struct ftp_node *ftpnode, IPTR handle, BOOL redo_cache)
{
	if (redo_cache)
		rexx_lst_clear(ftpnode->fn_opus, handle);
	else
		rexx_lst_empty(ftpnode->fn_opus, handle);

	rexx_lst_set_path(ftpnode->fn_opus, handle, ftpnode->fn_site.se_path);
	lister_reset_index_state(ftpnode);
}

BOOL lister_fallback_list_command(struct ftp_node *ftpnode)
{
	char next_cmd[LSCMDLEN + 1] = "";

	if (!ftpnode)
		return FALSE;

	if (!ftp_listcmd_next_after_failure(ftpnode->fn_lscmd,
										lister_default_list_command(ftpnode),
										next_cmd,
										sizeof(next_cmd)))
		return FALSE;

	D(bug("list command '%s' failed; trying '%s'\n", ftpnode->fn_lscmd, next_cmd));
	stccpy(ftpnode->fn_lscmd, next_cmd, LSCMDLEN + 1);
	lister_restore_list_parser(ftpnode);

	return TRUE;
}

/********************************/

//
//	Gets called for each line returned by the 'LIST' FTP command
//	Translates the info for each entry and adds it to the lister
//
static int list_update(struct update_info *ui, char *line)
{
	struct entry_info entry = {0};
#if defined(__AROS__) || defined(__amigaos3__)
	struct Device *TimerBase = (struct Device *)GetTimerBase();
#else
	struct Library *TimerBase = GetTimerBase();
#endif
#ifdef __amigaos4__
	struct TimerIFace *ITimer = (struct TimerIFace *)GetInterface(TimerBase, "main", 1, NULL);
#endif

	// Valid?
	if (!ui || !line)
		return 1;

	// Is this needed any more?
	ObtainSemaphore(&ui->ui_sem);

	if (ui->ui_ftpnode->fn_ls_to_entryinfo(&entry, line, ui->ui_flags))
	{
		lister_add(ui->ui_ftpnode,
				   entry.ei_name,
				   entry.ei_size,
				   entry.ei_type,
				   entry.ei_seconds,
				   entry.ei_prot,
				   entry.ei_comment);

		GetSysTime(&ui->ui_curr);
		// gettimeofday(&ui->ui_curr, NULL);

		if (ui->ui_curr.tv_secs - ui->ui_last.tv_secs >= ui->ui_ftpnode->fn_site.se_env->e_list_update)
		{
			ftplister_refresh(ui->ui_ftpnode, REFRESH_NODATE);
			ui->ui_last = ui->ui_curr;
		}
	}

	ReleaseSemaphore(&ui->ui_sem);

#ifdef __amigaos4__
	DropInterface((struct Interface *)ITimer);
#endif

	return 1;
}

/***************************/

static int sftp_list_update(void *userdata, const struct ftp_sftp_entry *entry)
{
	struct update_info *ui = userdata;
#if defined(__AROS__) || defined(__amigaos3__)
	struct Device *TimerBase;
#else
	struct Library *TimerBase;
#endif
#ifdef __amigaos4__
	struct TimerIFace *ITimer;
#endif

	if (!ui || !entry)
		return 1;

#if defined(__AROS__) || defined(__amigaos3__)
	TimerBase = (struct Device *)GetTimerBase();
#else
	TimerBase = GetTimerBase();
#endif
#ifdef __amigaos4__
	ITimer = (struct TimerIFace *)GetInterface(TimerBase, "main", 1, NULL);
#endif

	ObtainSemaphore(&ui->ui_sem);

	lister_add(ui->ui_ftpnode,
			   (char *)entry->name,
			   entry->size,
			   entry->type,
			   ftp_sftp_unix_to_amiga_seconds(entry->seconds),
			   prot_unix_to_amiga(entry->unixprot),
			   (char *)entry->comment);

	GetSysTime(&ui->ui_curr);
	if (ui->ui_curr.tv_secs - ui->ui_last.tv_secs >= ui->ui_ftpnode->fn_site.se_env->e_list_update)
	{
		ftplister_refresh(ui->ui_ftpnode, REFRESH_NODATE);
		ui->ui_last = ui->ui_curr;
	}

	ReleaseSemaphore(&ui->ui_sem);

#ifdef __amigaos4__
	DropInterface((struct Interface *)ITimer);
#endif

	return !(ui->ui_ftpnode->fn_flags & LST_ABORT);
}

/***************************/

//
//	Ask if user wishes to grab index files
//
static int ask_index(struct ftp_node *ftpnode, char *name, int size)
{
	Lister *lister;
	int result = 0;
	char *bytes;

	if (!ftpnode->fn_og->og_noreq && (lister = (Lister *)ftpnode->fn_handle))
	{
		if (size > 1024)
		{
			size = size / 1024;
			bytes = GetString(locale, MSG_KILOBYTES);
		}
		else
			bytes = GetString(locale, MSG_FTP_XFER_BYTES);

		result = lister_request_tags(ftpnode,
									 FR_FormatString,
									 GetString(locale, MSG_QUERYINDEX),
									 AR_Message,
									 name,
									 AR_Message,
									 size,
									 AR_Message,
									 bytes,
									 FR_ButtonNum,
									 MSG_FTP_OK,
									 FR_ButtonNum,
									 MSG_FTP_CANCEL,
									 TAG_DONE);
	}

	return result;
}

#define FNAMESIZE 32

/***********************************/

//
//	split details from index file and update Lister
//	This ASSUMES a standard format followed by
//	AMINET index and Files.BBS where data starts on column 40
//	In some cases this will not work so a simple back check is done
//	for a ' ' or '+' as a starting marker
//
static void update_lister_comments(struct ftp_node *ftpnode, char *indexname)
{
	char fname[FNAMESIZE];
	char comment[COMMENTLEN];
	char buf[256];
	BPTR cf;
	APTR lister;
	DOpusCallbackInfo *infoptr = &ftpnode->fn_og->og_hooks;

	lister = (APTR)ftpnode->fn_handle;

	// lock list.  lister is still busy from previous fn call
	DC_CALL2(infoptr, dc_LockFileList, DC_REGA0, lister, DC_REGD0, TRUE);
	// ftpnode->fn_og->og_hooks.dc_LockFileList( (ULONG)lister, TRUE );

	// open file
	if ((cf = Open(indexname, MODE_OLDFILE)))
	{
		while (FGets(cf, buf, 256))
		{
			*fname = *comment = 0;

			if (*buf == '|' || *buf == '\n')
				continue;

			// strip,name and comment

			stptok(buf, fname, sizeof(fname), " ");

			if (strlen(buf) > 39)
			{
				// check if we have a ' ' or + on pos 39
				// if not then search backwards to up to 20 chars
				char *x;
				int limit = 20;

				x = &buf[39];

				while (--limit && *x != ' ' && *x != '+')
					--x;

				stptok(x + 1, comment, sizeof(comment), "\"\r\n");
			}

			// update the lister
			if (*fname && *comment)
				DC_CALL3(infoptr, dc_SetFileComment, DC_REGA0, lister, DC_REGA1, fname, DC_REGA2, comment);
			// ftpnode->fn_og->og_hooks.dc_SetFileComment( (ULONG)lister, fname, comment );
		}
		Close(cf);
	}

	// unlock list
	DC_CALL1(infoptr, dc_UnlockFileList, DC_REGA0, lister);
	// ftpnode->fn_og->og_hooks.dc_UnlockFileList( (ULONG)lister );

	// refresh lister
	ftplister_refresh(ftpnode, REFRESH_NODATE);
}

/***********************************
 * set index type and get index fromm remote site
 * and update lister
 */

static void get_index(struct ftp_node *ftpnode)
{
	char localname[80];
	char *remotename;
	int size;
	BPTR file;

	if (ftpnode->fn_ftp.fi_found_index & INDEX_INDEX)
	{
		remotename = NAME_INDEX;
		size = ftpnode->fn_ftp.fi_found_index_size;
	}
	else
	{
		remotename = NAME_FBBS;
		size = ftpnode->fn_ftp.fi_found_fbbs_size;
	}

	if ((size / 1024) > ftpnode->fn_site.se_env->e_indexsize)
	{
		if (ftpnode->fn_site.se_env->e_index_auto	   // if auto and >max then skip
			|| !ask_index(ftpnode, remotename, size))  // or user cancels
			return;
	}

	// sprintf( localname, "T:do_idx#%lu", ftpnode->fn_handle );
	if ((file = open_temp_file(localname, ftpnode->fn_ipc)))
		Close(file);

	if (lister_xferindex(ftpnode, localname, remotename, size))	 // Get OK
		update_lister_comments(ftpnode, localname);

	// NOTE: lister_xferindex left lister locked
	rexx_lst_unlock(ftpnode->fn_opus, ftpnode->fn_handle);

	// delete index or part index
	DeleteFile(localname);
}

/********************************/

//
//	Load directory into a lister, updates the path from the ftp_node structure
//
int lister_list(struct opusftp_globals *ogp, struct ftp_node *ftpnode, BOOL redo_cache)
{
	int retval = 0, buffered = 0;
	int lsresult;
	BOOL abort = FALSE;
	struct update_info *ui;
	IPTR handle;
#if defined(__AROS__) || defined(__amigaos3__)
	struct Device *TimerBase = (struct Device *)GetTimerBase();
#else
	struct Library *TimerBase = GetTimerBase();
#endif
#ifdef __amigaos4__
	struct TimerIFace *ITimer = (struct TimerIFace *)GetInterface(TimerBase, "main", 1, NULL);
#endif
	int progress = ftpnode->fn_site.se_env->e_progress_window;

	D(bug("lister_list()\n"));

	// Prepare for index if found
	lister_reset_index_state(ftpnode);

	// Passive mode required?
	ftpnode->fn_ftp.fi_flags &= ~FTP_PASSIVE;
	if (ftpnode->fn_site.se_env->e_passive)
		ftpnode->fn_ftp.fi_flags |= FTP_PASSIVE;

	if ((ui = AllocVec(sizeof(struct update_info), MEMF_CLEAR)))
	{
		handle = ftpnode->fn_handle;

		rexx_lst_lock(ftpnode->fn_opus, handle);

		if (!redo_cache)
			buffered = rexx_lst_findcache(ftpnode->fn_opus, handle, ftpnode->fn_site.se_path);

		if (!buffered || redo_cache)
		{
			lister_clear_listing(ftpnode, handle, redo_cache);

			InitSemaphore(&ui->ui_sem);

			if (ftpnode->fn_systype == FTP_MACOS)
				ui->ui_flags |= UI_NO_LINK_FIELD;

			if (ftpnode->fn_ftp.fi_flags & FTP_IS_UNIX)
				ui->ui_flags |= UI_DOT_HIDDEN;

			if (ftpnode->fn_site.se_env->e_unk_links_file)
				ui->ui_flags |= UI_LINKS_ARE_FILES;

			/*gp changed to use flag and not use fred_hack env var */

			if (ftpnode->fn_site.se_env->e_special_dir)
				ui->ui_flags |= UI_SPECIAL_DIR;

			ui->ui_og = ogp;
			ui->ui_ftpnode = ftpnode;
			ui->ui_handle = handle;
			ui->ui_opus = ftpnode->fn_opus;
			ui->ui_abort = &abort;

			if (progress)
			{
				lister_prog_init(ftpnode,
								 ftpnode->fn_site.se_host,
								 GetString(locale, MSG_READING_DIR),
								 ftpnode->fn_site.se_path,
								 PROGRESS_FILE_OFF,
								 PROGRESS_BAR_OFF);
			}
			else
			{
				rexx_lst_title(ftpnode->fn_opus, handle, GetString(locale, MSG_READING_DIR));
				send_rexxa(ftpnode->fn_opus, FALSE, "lister refresh %lu full", ftpnode->fn_handle);
			}

			lst_addabort(ftpnode, SIGBREAKF_CTRL_D, 0);

			GetSysTime(&ui->ui_last);
			// gettimeofday(&ui->ui_last, NULL);

			for (;;)
			{
				if (ftpnode->fn_protocol == FTP_PROTOCOL_SFTP)
				{
					ftpnode->fn_ftp.fi_aborted = 0;
					*ftpnode->fn_ftp.fi_serverr = 0;
					lsresult = ftp_sftp_list(&ftpnode->fn_sftp, "", sftp_list_update, ui) ? 0 : -1;
					if (lsresult)
					{
						if (ftp_sftp_session_error(&ftpnode->fn_sftp) == FTP_SFTP_ERROR_ABORTED)
							ftpnode->fn_ftp.fi_aborted = 1;
						else
							sprintf(ftpnode->fn_ftp.fi_serverr, "550 %s\r\n", ftp_sftp_error_message(&ftpnode->fn_sftp));
					}
				}
				else
					// Send the LIST command
					lsresult = list(&ftpnode->fn_ftp, list_update, ui, ftpnode->fn_lscmd, "");

				D(bug("list command result %d\n", lsresult));

				if (ftpnode->fn_protocol != FTP_PROTOCOL_SFTP && lsresult == -2 && lister_fallback_list_command(ftpnode))
				{
					lister_clear_listing(ftpnode, handle, redo_cache);
					GetSysTime(&ui->ui_last);
				}
				else
					break;
			}

			if (lsresult)
				lister_clear_listing(ftpnode, handle, redo_cache);

			ftplister_refresh(ftpnode, REFRESH_NODATE);

			retval = lsresult ? FALSE : TRUE;

			lst_remabort(ftpnode);

			if (progress)
				lister_prog_clear(ftpnode);
		}

		rexx_lst_label(ftpnode->fn_opus,
					   handle,
					   ftpnode->fn_protocol == FTP_PROTOCOL_SFTP ? "SFTP:" : "FTP:",
					   ftpnode->fn_site.se_host,
					   NULL);
		if (ftpnode->fn_protocol == FTP_PROTOCOL_SFTP)
		{
			char title[HOSTNAMELEN + 6];

			sprintf(title, "SFTP:%s", ftpnode->fn_site.se_host);
			rexx_lst_title(ftpnode->fn_opus, handle, title);
		}
		else
			rexx_lst_title(ftpnode->fn_opus, handle, ftpnode->fn_site.se_host);
		send_rexxa(ftpnode->fn_opus, FALSE, "lister refresh %lu full", handle);

		rexx_lst_unlock(ftpnode->fn_opus, handle);

		FreeVec(ui);
	}

	// Get index if option is enabled and it exists
	if (ftpnode->fn_site.se_env->e_index_enable && ftpnode->fn_ftp.fi_found_index)
		get_index(ftpnode);

#ifdef __amigaos4__
	DropInterface((struct Interface *)ITimer);
#endif

	return retval;
}
