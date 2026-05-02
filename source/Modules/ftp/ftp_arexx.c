/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

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
 *	18-09-95	Added "lister refresh full" after "lister set title"
 *	22-09-95	Added "lister set namelength 256" and "lister set case on" to lst_new
 *	25-09-95	Now that 'lister set busy' is synchronous, lst_lock() doesn't poll
 *	06-10-95	On very fast machines 'lister set busy' didn't work first time
 *				so the polling is back in lst_lock()
 *	12-10-95	Added lst_query_numselentries()
 *	 1-02-96	Converting progress meters to 5.1165 style
 *	 7-02-96	Now shows name of file being transferred when iconified
 *	 4-06-96	lst_set_busy now uses 'lister set busy on wait'
 *
 *	30-01-97	GJP various changes
 *			Changed mechanism of returning RC or RESULT
 *			added lst_title
 *			added lst_findcache
 *
 * 	18.4.97 	Added namelength and case on to get new buffer obtained
 *			from lister empty to support ftp parameters.
 *
 *	 3-12-97	Renamed lst_ functions to rexx_ to differentiate from cbak_
 *				Added rexx_getstring()
 *
 */

#include "ftp.h"
#include "ftp_arexx.h"
#include "ftp_opusftp.h"

/*
	#define	DEBUG
	#define LOUD 1
*/

// Debugging info from this file can be annoying
#undef DEBUG
#define LOUD 0

/*

// call "lister getstring" if we have a handle, "dopus getstring" otherwise
char *rexx_getstring(
	const char *opus,
	ULONG       handle,
	char       *text,
	BOOL        secure,
	int         length,
	char       *deflt,
	char       *buttons )
{
char *result = 0;

if	(handle)
	result = (char *)send_rexxa( opus, REXX_REPLY_RESULT,
		"lister getstring %lu '%s'%s%ld '%s' %s",
		handle,
		text,
		secure ? " secure " : " ",
		length,
		deflt,
		buttons );
else
	result = (char *)send_rexxa( opus, REXX_REPLY_RESULT,
		"dopus getstring '%s'%s%ld '%s' %s",
		text,
		secure ? " secure " : " ",
		length,
		deflt,
		buttons );

return result;
}
*/

// Add an entry to a lister which is NOT under our control

void rexx_lst_add(const char *opus,
				  IPTR handle,
				  char *name,
				  unsigned int size,
				  int type,
				  ULONG seconds,
				  LONG prot,
				  char *comment)
{
	char protbuf[9];

	// Convert protection bits to ascii
	protbuf[0] = prot & FIBF_HIDDEN ? 'h' : '-';
	protbuf[1] = prot & FIBF_SCRIPT ? 's' : '-';
	protbuf[2] = prot & FIBF_PURE ? 'p' : '-';
	protbuf[3] = prot & FIBF_ARCHIVE ? 'a' : '-';
	protbuf[4] = prot & FIBF_READ ? '-' : 'r';
	protbuf[5] = prot & FIBF_WRITE ? '-' : 'w';
	protbuf[6] = prot & FIBF_EXECUTE ? '-' : 'e';
	protbuf[7] = prot & FIBF_DELETE ? '-' : 'd';
	protbuf[8] = 0;

	send_rexxa(opus,
			   FALSE,
			   "lister add %lu \"%s\" %lu %ld %lu %s%s%s",
			   handle,
			   name,
			   size,
			   type,
			   seconds,
			   protbuf,
			   comment ? " " : "",
			   comment ? comment : "");
}

/********************************/

void rexx_doubleclick(const char *opus, const char *path)
{
	send_rexxa(opus, REXX_REPLY_NONE, "command doubleclick %s", path);
}

/********************************/

//
//	Refresh a lister
//
void rexx_lst_refresh(const char *opus, IPTR handle, int date)
{
	D(bug("Refresh lister - start - "));

	send_rexxa(opus, FALSE, "lister refresh %lu", handle);
	if (date == REFRESH_DATE)
		send_rexxa(opus, FALSE, "lister refresh %lu date", handle);

	D(bug("end\n"));
}

/********************************/

/*
 *	Set or clear a lister's busy state
 */

void rexx_lst_busy(const char *opus, IPTR handle, int val)
{
	/*
	if	(val)
		D(bug( "rexx_lst_busy()\n" ));
	else
		D(bug( "rexx_lst_unlock()\n" ));
	*/

	send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu busy %d wait", handle, val);
}

/********************************/

/*
 *	Tell Opus to cache the current buffer and moves to a new one
 *	18.4.97 Added namelength and case on to et new buffer obtained
 *	from lister empty to support ftp parameters.
 *
 */

void rexx_lst_empty(const char *opus, IPTR handle)
{
	D(bug("rexx_lst_empty(%ld)\n", handle));

	send_rexxa(opus, REXX_REPLY_NONE, "lister empty %lu", handle);
	send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu namelength 256", handle);
	send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu case on", handle);
}

/********************************/

/*
 *	Opus clears the current buffer, does not move to a new one
 */

void rexx_lst_clear(const char *opus, IPTR handle)
{
	D(bug("rexx_lst_clear(%ld)\n", handle));

	send_rexxa(opus, REXX_REPLY_NONE, "lister clear %lu", handle);
}

/********************************/

/*
 *	Close a lister and clear cache buffers
 */

void rexx_lst_close(const char *opus, IPTR handle)
{
	// new command : lister freecaches <handle> <handler>
	// eg, lister freecaches 129384849 _OPUS_FTP_
	send_rexxa(opus, REXX_REPLY_NONE, "lister freecaches %lu " PORTNAME "", handle);

	// Remove handler so we don't get 'inactive' msg
	send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu handler ''", handle);

	// Close lister
	send_rexxa(opus, REXX_REPLY_NONE, "lister close %lu", handle);
}

/*
 *	check if dir is in Opus cache. If so then Opus it will display it
 *
 *	returns 1/0 for entry cached or not
 */
int rexx_lst_findcache(const char *opus, IPTR handle, char *path)
{
	char *string;
	int value = 0;

	if ((string = (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister findcache %lu %s", handle, path)))
	{
		value = atoi(string);
		DeleteArgstring(string);
	}

	return value;
}

// Remember to do a refresh after this
void rexx_lst_title(const char *opus, IPTR handle, char *title)
{
	if (strncmp(title, "FTP:", 4) && strncmp(title, "SFTP:", 5))
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu title FTP:%s", handle, title);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu title %s", handle, title);
}

// Remember to do a refresh after this
char *rexx_lst_title_swap(const char *opus, IPTR handle, char *title)
{
	if (strncmp(title, "FTP:", 4) && strncmp(title, "SFTP:", 5))
		return (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister set %lu title FTP:%s", handle, title);
	else
		return (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister set %lu title %s", handle, title);
}

/********************************/

//
//	Set the label that the lister will have while iconified
//
void rexx_lst_label(const char *opus, IPTR handle, char *pref, char *label, char *suff)
{
	if (!pref)
		pref = "";
	if (!suff)
		suff = "";

	if (label)
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu label (%s%s%s)", handle, pref, label, suff);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu label", handle);
}

/********************************/

//
//	Ensure a lister becomes busy
//
void rexx_lst_lock(const char *opus, IPTR handle)
{
	int notdone = 1;
	char *r;

	D(bug("rexx_lst_lock()\n"));

	while (notdone)
	{
		rexx_lst_busy(opus, handle, 1);

		if ((r = (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister query %lu busy", handle)))
		{
			if (*r == '1')
				notdone = 0;

			DeleteArgstring(r);
		}

		if (notdone)
			Delay(5);
	}
}

/********************************/

//
//	Create a new lister and set it's title and handler
//
IPTR rexx_lst_new(const char *opus, IPTR handle, char *host, const char *toolbar)
{
	char *asciihandle;
	ULONG new_handle;
	BPTR dir_lock = 0, cd = 0, toolbar_lock = 0;

	if (toolbar && *toolbar && (dir_lock = Lock("DOpus5:Buttons/", ACCESS_READ)))
	{
		cd = CurrentDir(dir_lock);
		toolbar_lock = Lock((char *)toolbar, ACCESS_READ);
	}

	if (handle)
	{
		new_handle = handle;

		//	TODO DeviceList causes problems when the list is not cleared
		//	rexx_lst_clear( opus, new_handle )

		// if there is special toolbar then set it.
		if (toolbar_lock)
			send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu toolbar %s", new_handle, toolbar);
	}
	else
	{
		if (toolbar_lock)
			asciihandle = (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister new toolbar %s", toolbar);
		else
			asciihandle = (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister new");
		new_handle = atoi(asciihandle);
		DeleteArgstring(asciihandle);
	}

	if (new_handle)
	{
		send_rexxa(
			opus, REXX_REPLY_NONE, "lister set %lu handler " PORTNAME " quotes editing subdrop synctraps", new_handle);

#if 0
	send_rexxa( opus, REXX_REPLY_NONE, "lister set %lu title FTP:%s", new_handle, host );
	send_rexxa( opus, REXX_REPLY_NONE, "lister set %lu namelength 256", new_handle );
	send_rexxa( opus, REXX_REPLY_NONE, "lister set %lu case on", new_handle );
	send_rexxa( opus, REXX_REPLY_NONE, "lister refresh %lu full", new_handle );
	rexx_lst_label( opus, new_handle, "FTP:", host, NULL );
#endif

		send_rexxa(opus, REXX_REPLY_NONE, "lister wait %lu quick", new_handle);
	}

	if (toolbar_lock)
		UnLock(toolbar_lock);

	if (dir_lock)
	{
		CurrentDir(cd);
		UnLock(dir_lock);
	}

	return new_handle;
}

/*********************************/

BOOL rexx_lst_query_handler(const char *opus, IPTR handle)
{
	char *handler;
	BOOL result = FALSE;

	if ((handler = (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister query %lu handler", handle)))
	{
		if (*handler)
			result = TRUE;

		DeleteArgstring(handler);
	}

	return result;
}

/*********************************/

//
//	Is lister visible?
//	Still there after an inactive msg?
//
int rexx_lst_query_visible(const char *opus, IPTR handle)
{
	char *string;
	int value = 0;

	if ((string = (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister query %lu visible", handle)))
	{
		D(bug("** query visible '%s'\n", string));
		value = atoi(string);
		DeleteArgstring(string);
	}

	return value;
}

/********************************/

/*
 *	Returns 1st dest handle
 */

ULONG rexx_lst_query_dest1(const char *opus)
{
	char *dst;
	ULONG result = 0;

	if ((dst = (char *)send_rexx(opus, REXX_REPLY_RESULT, "lister query dest")))
	{
		result = atoi(dst);
		DeleteArgstring(dst);
	}

	return result;
}

/********************************/

/*
 *	Get the fileinfo string for a list entry
 */

char *rexx_lst_query_entry(const char *opus, IPTR handle, char *entry)
{
	return (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister query %lu entry \"%s\"", handle, entry);
}

/********************************/

static int rexx_lst_query_numblah(const char *opus, IPTR handle, const char *blah)
{
	char *s;
	int n = 0;

	if ((s = (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister query %lu num%s", handle, blah)))
	{
		n = atoi(s);
		DeleteArgstring(s);
	}

	return n;
}

/********************************/

int rexx_lst_query_numentries(const char *opus, IPTR handle)
{
	return rexx_lst_query_numblah(opus, handle, "entries");
}

/********************************/

int rexx_lst_query_numfiles(const char *opus, IPTR handle)
{
	return rexx_lst_query_numblah(opus, handle, "files");
}

/********************************/

int rexx_lst_query_numseldirs(const char *opus, IPTR handle)
{
	return rexx_lst_query_numblah(opus, handle, "seldirs");
}

/********************************/

int rexx_lst_query_numselentries(const char *opus, IPTR handle)
{
	return rexx_lst_query_numblah(opus, handle, "selentries");
}

/********************************/

int rexx_lst_query_numselfiles(const char *opus, IPTR handle)
{
	return rexx_lst_query_numblah(opus, handle, "selfiles");
}

/********************************/

char *rexx_lst_query_path(const char *opus, IPTR handle)
{
	char *path;

	D(bug("rexx_lst_query_path(%ld)\n", handle));

	path = (char *)send_rexxa(opus, REXX_REPLY_RESULT, "lister query %lu path", handle);

	D(bug("-> '%s'\n", path));

	return path;
}

/********************************/

//
//	Returns 1st source handle
//
ULONG rexx_lst_query_src1(const char *opus)
{
	char *src;
	ULONG result = 0;

	if ((src = (char *)send_rexx(opus, REXX_REPLY_RESULT, "lister query source")))
	{
		result = atoi(src);
		DeleteArgstring(src);
	}

	return result;
}

/********************************/

void rexx_lst_remove(const char *opus, IPTR handle, char *name)
{
	send_rexxa(opus, REXX_REPLY_NONE, "lister remove %lu \"%s\"", handle, name);
}

/********************************/

/*
 *	Select or deselect an entry in a lister
 */

void rexx_lst_select(const char *opus, IPTR handle, char *name, int state)
{
	send_rexxa(opus, REXX_REPLY_NONE, "lister select %lu \"%s\" %d", handle, name, state);
}

/********************************/

void rexx_lst_set_path(const char *opus, IPTR handle, char *path)
{
	D(bug("rexx_lst_set_path(%ld)\n", handle));
	D(bug("-> '%s'\n", path));

	send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu path %s", handle, path);

	// DeleteArgstring( rexx_lst_query_path( opus, handle ) );
}

/********************************/

void rexx_prog_bar(const char *opus, IPTR handle, int type, int total, int count)
{
	if (type == PROGRESS_FREE)
		send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu bar %ld %ld", handle, total, count);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress bar %ld %ld", handle, total, count);
}

/********************************/

void rexx_prog_bytes(const char *opus, IPTR handle, int type, int total, int count)
{
	if (type == PROGRESS_FREE)
		send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu file %ld %ld", handle, total, count);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress file %ld %ld", handle, total, count);
}

/********************************/

void rexx_prog_clear(const char *opus, IPTR handle, int type)
{
	if (type == PROGRESS_FREE)
		send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu off", handle);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister clear %lu progress", handle);
}

/********************************/

//
//	Initialize a progress bar.
//	If no handle is passed in, a free-floating progress bar will be created.
//	Returns the handle passed in or the handle of the free-floating bar.
//
IPTR rexx_prog_init(const char *opus, IPTR handle, int type, char *title, char *info, char *name, int file, int bar)
{
	char buf[80];
	char *str;

	if (type == PROGRESS_FREE)
		sprintf(buf, "dopus progress abort", handle);
	else
		sprintf(buf, "lister set %lu newprogress abort", handle);

	if (info)
		strcat(buf, " info");

	if (name)
		strcat(buf, " name");
	if (file)
		strcat(buf, " file");
	if (bar)
		strcat(buf, " bar");

	if (type == PROGRESS_FREE)
	{
		if ((str = (char *)send_rexx(opus, REXX_REPLY_RESULT, buf)))
		{
			handle = atoi(str);
			DeleteArgstring(str);
		}
	}
	else
		send_rexx(opus, REXX_REPLY_NONE, buf);

	if (handle)
	{
		if (title && *title)
		{
			if (type == PROGRESS_FREE)
				send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu title %s", handle, title);
			else
				send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress title %s", handle, title);
		}
		if (info && *info)
		{
			if (type == PROGRESS_FREE)
				send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu info %s", handle, info);
			else
				send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress info %s", handle, info);
		}
		if (name && *name)
		{
			if (type == PROGRESS_FREE)
				send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu name %s", handle, name);
			else
				send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress name %s", handle, name);
		}
	}

	return handle;
}

/********************************/

//
//	Initialize a progress bar.
//	If no handle is passed in, a free-floating progress bar will be created.
//	Returns the handle passed in or the handle of the free-floating bar.
//
IPTR rexx_prog_init3(const char *opus,
					  IPTR handle,
					  int type,
					  char *title,
					  char *info,

					  char *info2,
					  char *info3,

					  char *name,
					  int file,
					  int bar)
{
	char buf[80];
	char *str;

	if (type == PROGRESS_FREE)
		sprintf(buf, "dopus progress abort", handle);
	else
		sprintf(buf, "lister set %lu newprogress abort", handle);

	if (info)
		strcat(buf, " info");
	if (info2)
		strcat(buf, " info2");
	if (info3)
		strcat(buf, " info3");
	if (name)
		strcat(buf, " name");
	if (file)
		strcat(buf, " file");
	if (bar)
		strcat(buf, " bar");

	if (type == PROGRESS_FREE)
	{
		if ((str = (char *)send_rexx(opus, REXX_REPLY_RESULT, buf)))
		{
			handle = atoi(str);
			DeleteArgstring(str);
		}
	}
	else
		send_rexx(opus, REXX_REPLY_NONE, buf);

	if (handle)
	{
		if (title && *title)
		{
			if (type == PROGRESS_FREE)
				send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu title %s", handle, title);
			else
				send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress title %s", handle, title);
		}
		if (info && *info)
		{
			if (type == PROGRESS_FREE)
				send_rexxa(
					opus, REXX_REPLY_NONE, "dopus progress %lu info %s info2 %s info3 %s ", handle, info, info2, info3);
			else
				send_rexxa(opus,
						   REXX_REPLY_NONE,
						   "lister set %lu newprogress info %s info2 %s info3 %s ",
						   handle,
						   info,
						   info2,
						   info3);
		}
		if (name && *name)
		{
			if (type == PROGRESS_FREE)
				send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu name %s", handle, name);
			else
				send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress name %s", handle, name);
		}
	}

	return handle;
}

/********************************/

void rexx_prog_name(const char *opus, IPTR handle, int type, char *name)
{
	if (type == PROGRESS_FREE)
		send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu name %s", handle, name);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress name %s", handle, name);
}

/********************************/

void rexx_prog_info(const char *opus, IPTR handle, int type, char *info)
{
	if (type == PROGRESS_FREE)
		send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu info %s", handle, info);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress info %s", handle, info);
}

void rexx_prog_info2(const char *opus, IPTR handle, int type, char *info)
{
	if (type == PROGRESS_FREE)
		send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu info2 %s", handle, info);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress info2 %s", handle, info);
}

void rexx_prog_info3(const char *opus, IPTR handle, int type, char *info)
{
	if (type == PROGRESS_FREE)
		send_rexxa(opus, REXX_REPLY_NONE, "dopus progress %lu info %s", handle, info);
	else
		send_rexxa(opus, REXX_REPLY_NONE, "lister set %lu newprogress info3 %s", handle, info);
}

/********************************/

/*
 *	Send an Arexx command, get an optional result back
 */

IPTR STDARGS send_rexxa(const char *dest_portname, int reply_type, const char *fmt, ...)
{
	va_list ap;
	IPTR retval = 0;
	char *buf;

	if ((buf = AllocVec(1024, MEMF_ANY)))
	{
		va_start(ap, fmt);
		vsprintf(buf, fmt, ap);
		va_end(ap);

		retval = send_rexx(dest_portname, reply_type, buf);
		FreeVec(buf);
	}

	return retval;
}

/********************************/

//
//	Send a command to Dopus
//	 Will return a LONG from result1 for numeric results
//	 or a char* from result2 for string results
//	 or -1 if it couldn't even be sent
//
//	gjp changed to make return RC/RESULT easier by using
//	reply_type is emum REXX_REPLY_NONE,REXX_REPLY_RESULT,REXX_REPLY_RC;
//
IPTR send_rexx(const char *dest_portname, int reply_type, const char *cmd)
{
	IPTR *retptr, retval = -1;		// The result and a pointer to it
	struct MsgPort *destport, *rp;	// The destination and reply ports
	UBYTE *argstr;					// The arexx argument string
	struct RexxMsg *rxmsg;			// The arexx message

#ifdef DEBUG
	BOOL quiet = FALSE;
	BOOL show_debug = FALSE;
#endif

	if ((rp = CreateMsgPort()))
	{
		if ((rxmsg = CreateRexxMsg(rp, NULL, NULL)))
		{
			if (reply_type == REXX_REPLY_RC)
				retptr = &rxmsg->rm_Result1;
			else
				retptr = &rxmsg->rm_Result2;

#ifdef DEBUG
			quiet = !(strnicmp(cmd, "lister add", 10) && strnicmp(cmd, "lister refresh", 13) &&
					  (!strstr(cmd, "progress count")) && (!strstr(cmd, "newprogress file")) &&
					  (!strstr(cmd, "abort")) /*&& (!strstr(cmd,"busy"))*/
					  && (!strstr(cmd, "addtrap")));

			if (LOUD || !quiet)
			{
				show_debug = TRUE;
				D(bug("%s: '%s'\n", dest_portname, cmd));
			}
#endif
			if ((argstr = CreateArgstring((char *)cmd, strlen(cmd))))
			{
				// We only send commands
				rxmsg->rm_Action = RXCOMM;

				if (reply_type != REXX_REPLY_NONE)
				{
					// Do we need a reply?
					rxmsg->rm_Action |= RXFF_RESULT;
				}

				rxmsg->rm_Args[0] = (IPTR)argstr;

				Forbid();
				if ((destport = FindPort((char *)dest_portname)))
					PutMsg(destport, (struct Message *)rxmsg);
				Permit();
				if (destport)
				{
					WaitPort(rp);
					GetMsg(rp);
					retval = *retptr;
#ifdef DEBUG
					if (reply_type != REXX_REPLY_NONE && show_debug)
					{
						D(bug("RC==> 1: %ld  ", rxmsg->rm_Result1));
						if (retptr == &rxmsg->rm_Result2)
							D(bug("RES==> 2: '%s'\n", retval));
						D(bug("\n"));
					}
#endif
				}

				DeleteArgstring(argstr);
			}
			DeleteRexxMsg(rxmsg);
		}
		DeleteMsgPort(rp);
	}

	return retval;
}

/********************************/

//
//	Reply to an arexx message
//
void reply_rexx(struct RexxMsg *msg, LONG r1, LONG r2)
{
	if (msg)
	{
		msg->rm_Result1 = r1;

		if (msg->rm_Action & RXFF_RESULT)
			msg->rm_Result2 = r2;

		ReplyMsg((struct Message *)msg);
	}
}

/********************************/

//
//	Flush ARexx port
//
void flush_arexxport(struct MsgPort *port)
{
	struct RexxMsg *msg;

	if (port)
	{
		while ((msg = (struct RexxMsg *)GetMsg(port)))
			reply_rexx(msg, 10, 0);
	}
}
