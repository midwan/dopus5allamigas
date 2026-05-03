/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas (dopus5allamigas fork)

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

#include "dopus.h"
#include "scripts.h"

#include <proto/newicon.h>
#include <proto/multiuser.h>

// Quit the program
void quit(BOOL script)
{
	// If main status window is open, close it
	if (main_status)
	{
		CloseProgressWindow(main_status);
		main_status = 0;
	}

	if (GUI)
	{
		// Clear 'startup' flag for scripts
		GUI->flags &= ~GUIF_DONE_STARTUP;

		// Close commodities
		cx_remove(GUI->cx);

		// Update environment settings
		env_update_settings(1);

		// Stop notify request
		RemoveNotifyRequest(GUI->notify_req);
		GUI->notify_req = 0;

		// Is there a hide appicon?
		if (GUI->hide_appicon)
		{
			RemoveAppIcon(GUI->hide_appicon);
			FreeCachedDiskObject(GUI->hide_diskobject);
		}

		// Or an appmenuitem?
		if (GUI->hide_appitem)
			RemoveAppMenuItem(GUI->hide_appitem);

		// Launch shutdown script
		if (script)
			RunScript(SCRIPT_SHUTDOWN, 0);

		// Set quit flag
		GUI->flags |= GUIF_PENDING_QUIT;

		// Shut the display down
		close_display(CLOSE_ALL, TRUE);

		// Send quit notifications
		quit_notify();

		// Stop notifications
		stop_file_notify(GUI->pattern_notify);
		stop_file_notify(GUI->font_notify);
		stop_file_notify(GUI->modules_notify);
		stop_file_notify(GUI->commands_notify);
		stop_file_notify(GUI->env_notify);
		stop_file_notify(GUI->desktop_notify);
		stop_file_notify(GUI->filetype_notify);

		// Free application port
		if (GUI->appmsg_port)
		{
			DOpusAppMessage *amsg;
			RemPort(GUI->appmsg_port);
			while ((amsg = (DOpusAppMessage *)GetMsg(GUI->appmsg_port)))
				ReplyAppMessage(amsg);
			DeleteMsgPort(GUI->appmsg_port);
			GUI->appmsg_port = 0;

			// Remove public semaphore
			RemSemaphore((struct SignalSemaphore *)&pub_semaphore);
		}

		// Flush IPC port
		IPC_Flush(&main_ipc);

		// Close all processes
		IPC_ListQuit(&GUI->lister_list, &main_ipc, 0, TRUE);
		IPC_ListQuit(&GUI->group_list, &main_ipc, 0, TRUE);
		IPC_ListQuit(&GUI->buttons_list, &main_ipc, 0, TRUE);
		IPC_ListQuit(&GUI->startmenu_list, &main_ipc, 0, TRUE);
		IPC_ListQuit(&GUI->process_list, &main_ipc, 0, TRUE);
		IPC_ListQuit(&GUI->function_list, &main_ipc, 0, TRUE);

		// Free buffers
		buffers_clear(0);

		// Remove all handlers
		RemFunctionTrap("*", "#?");

		// Free filetypes
		FreeMemHandle(GUI->filetype_memory);

		// Flush the filetype cache
		ClearFiletypeCache();

		// Free lister toolbar
		FreeToolBar(GUI->toolbar);

		// Free menus and hotkeys
		CloseButtonBank(GUI->lister_menu);
		CloseButtonBank(GUI->hotkeys);

		// Free scripts bank
		CloseButtonBank(GUI->scripts);

		// Free user menus
		CloseButtonBank(GUI->user_menu);
		FreeVec(GUI->user_menu_data);

		// Free backdrop list
		backdrop_free(GUI->backdrop);

		// Free icons
		if (GUI->lister_icon)
			FreeCachedDiskObject(GUI->lister_icon);
		if (GUI->button_icon)
			FreeCachedDiskObject(GUI->button_icon);

		// Free arrow image
		CloseImage(GUI->toolbar_arrow_image);

#ifdef __amigaos3__
		FreeVec(arrow_hi_data_chip);
		FreeVec(arrow_lo_data_chip);
		FreeVec(small_arrow_chip);
		FreeVec(big_arrow_chip);
	#ifndef USE_SCREENTITLE
		FreeVec(moon_big_data_chip);
		FreeVec(moon_small_data_chip);
	#endif
		FreeVec(command_arrow_chip);
		FreeVec(parent_arrow_chip);
#endif

		// Free screen signal
		if (GUI->screen_signal != -1)
			FreeSignal(GUI->screen_signal);

		// Delete notify port
		if (GUI->notify_port)
			DeleteMsgPort(GUI->notify_port);

		// Free position memory
		FreeMemHandle(GUI->position_memory);

		// Free command history
		Att_RemList(GUI->command_history, 0);

		// Delete icon positioning port
		DeleteMsgPort(GUI->iconpos_port);

		// Free popup menu
		PopUpFreeHandle(GUI->desktop_menu);

		// Clear requester pattern hook in library
		if (GUI->flags2 & GUIF2_BACKFILL_SET)
			SetReqBackFill(0, 0);
	}

	// Free scripts
	FreeScripts();

	// Free environment
	environment_free(environment);

	// Delete main message ports
	IPC_Flush(&main_ipc);
	DeleteMsgPort(main_ipc.command_port);
	DeleteMsgPort(main_ipc.reply_port);

	// Pause here for a couple of seconds to let everything clean up
	Delay(3 * 50);

	// Free global data
	FreeMemHandle(global_memory_pool);

	// Delete any temporary files
	delete_temp_files(0);

	// Free locale data
	free_locale_data(&locale);

	// Close input device
	if (InputBase)
	{
#ifdef __amigaos4__
		DropInterface((struct Interface *)IInput);
#endif
		CloseDevice((struct IORequest *)&input_req);
	}

	// Close timer device
	if (TimerBase)
	{
#ifdef __amigaos4__
		DropInterface((struct Interface *)ITimer);
#endif
		CloseDevice((struct IORequest *)&timer_req);
	}

	// Close console device
	if (ConsoleDevice)
	{
#ifdef __amigaos4__
		DropInterface((struct Interface *)IConsole);
#endif
		CloseDevice((struct IORequest *)&console_req);
	}

// Close libraries
#ifdef __amigaos4__
	DropInterface((struct Interface *)Imu);
#endif
	CloseLibrary(muBase);

#ifndef __amigaos3__
	#ifdef __amigaos4__
	DropInterface((struct Interface *)INewIcon);
	#endif
	CloseLibrary((struct Library *)NewIconBase);
#endif

#ifdef __amigaos4__
	// Guard DropInterface: interface acquisition in startup can fail and
	// leave IP96/ICyberGfx at NULL even though the base pointer is also
	// NULL. DropInterface(NULL) is not safe on all OS4 SDK versions.
	if (IP96)
		DropInterface((struct Interface *)IP96);
#endif
	CloseLibrary(P96Base);

#ifdef __amigaos4__
	if (ICyberGfx)
		DropInterface((struct Interface *)ICyberGfx);
#endif
	CloseLibrary(CyberGfxBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IAmigaGuide);
#endif
	CloseLibrary(AmigaGuideBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IDataTypes);
#endif
	CloseLibrary(DataTypesBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IAsl);
#endif
	CloseLibrary(AslBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IIcon);
#endif
	CloseLibrary(IconBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IDiskfont);
#endif
	CloseLibrary(DiskfontBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IWorkbench);
#endif
	CloseLibrary(WorkbenchBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)ICommodities);
#endif
	CloseLibrary(CxBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IRexxSys);
#endif
	CloseLibrary((struct Library *)RexxSysBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IUtility);
#endif
	CloseLibrary(UtilityBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IGadTools);
#endif
	CloseLibrary(GadToolsBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)ILayers);
#endif
	CloseLibrary((struct Library *)LayersBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IGraphics);
#endif
	CloseLibrary((struct Library *)GfxBase);

#ifdef __amigaos4__
	DropInterface((struct Interface *)IIntuition);
#endif
	CloseLibrary((struct Library *)IntuitionBase);

	// Restore old current directory?
	if (old_current_dir)
	{
		UnLock(CurrentDir(old_current_dir));
	}

// Close the dopus5.library
#ifdef __amigaos4__
	DropInterface((struct Interface *)IDOpus);
#endif
	CloseLibrary(DOpusBase);

	// Outahere!
	exit(0);
}

// Delete any temporary files
void delete_temp_files(struct DateStamp *last)
{
	struct AnchorPath *anchor;
	BPTR old, lock;

	// Lock T: and cd
	if ((lock = Lock("t:", ACCESS_READ)))
	{
		// CD to t:
		old = CurrentDir(lock);

		// Allocate an anchor
		if ((anchor = AllocVec(sizeof(struct AnchorPath), MEMF_CLEAR)))
		{
			short error;
			struct FileInfoBlock fibcopy;

			// See if any script files already exist
			error = MatchFirst("T:dopus#?tmp#?", anchor);
			while (!error)
			{
				// Copy fib
				fibcopy = anchor->ap_Info;

				// Get next
				error = MatchNext(anchor);

				// Check datestamp
				if (!last || CompareDates(&fibcopy.fib_Date, last) >= 0)
				{
					// Try to delete file
					DeleteFile(fibcopy.fib_FileName);
				}
			}

			// Cleanup anchor
			MatchEnd(anchor);
			FreeVec(anchor);
		}

		UnLock(CurrentDir(old));
	}
}

// Notify people we're quitting
BOOL quit_notify(void)
{
	APTR trap;
	char port[40];

	// Lock trap list
	trap = LockTrapList();

	// Look for quit traps
	while ((trap = FindTrapEntry(trap, "quit", port)))
	{
		// Send abort message
		rexx_handler_msg(port, 0, 0, HA_String, 0, "quit", TAG_END);
	}

	// Unlock trap list
	UnlockTrapList();

	// Broadcast notify message
	SendNotifyMsg(DN_OPUS_QUIT, GUI->dopus_copy, 0, 0, GUI->rexx_port_name, 0);

	return 1;
}
