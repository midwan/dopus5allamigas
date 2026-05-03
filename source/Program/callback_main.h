#ifndef __CALLBACK_MAIN_H__
#define __CALLBACK_MAIN_H__

#if defined(__MORPHOS__)

	#define CALLBACK_0(ret, name)                             \
		static ret name##_stubs(void) { return (ret)name(); } \
		static struct EmulLibEntry name##_trap = {TRAP_LIB, 0, (APTR)&name##_stubs};

	#define CALLBACK_1(ret, name, r1, t1, n1) \
		static ret name##_stubs(void)         \
		{                                     \
			t1 n1 = (t1)REG_##r1;             \
			return (ret)name(n1);             \
		}                                     \
		static struct EmulLibEntry name##_trap = {TRAP_LIB, 0, (APTR)&name##_stubs};

	#define CALLBACK_2(ret, name, r1, t1, n1, r2, t2, n2) \
		static ret name##_stubs(void)                     \
		{                                                 \
			t1 n1 = (t1)REG_##r1;                         \
			t2 n2 = (t2)REG_##r2;                         \
			return (ret)name(n1, n2);                     \
		}                                                 \
		static struct EmulLibEntry name##_trap = {TRAP_LIB, 0, (APTR)&name##_stubs};

	#define CALLBACK_3(ret, name, r1, t1, n1, r2, t2, n2, r3, t3, n3) \
		static ret name##_stubs(void)                                 \
		{                                                             \
			t1 n1 = (t1)REG_##r1;                                     \
			t2 n2 = (t2)REG_##r2;                                     \
			t3 n3 = (t3)REG_##r3;                                     \
			return (ret)name(n1, n2, n3);                             \
		}                                                             \
		static struct EmulLibEntry name##_trap = {TRAP_LIB, 0, (APTR)&name##_stubs};

	#define CALLBACK_4(ret, name, r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4) \
		static ret name##_stubs(void)                                             \
		{                                                                         \
			t1 n1 = (t1)REG_##r1;                                                 \
			t2 n2 = (t2)REG_##r2;                                                 \
			t3 n3 = (t3)REG_##r3;                                                 \
			t4 n4 = (t4)REG_##r4;                                                 \
			return (ret)name(n1, n2, n3, n4);                                     \
		}                                                                         \
		static struct EmulLibEntry name##_trap = {TRAP_LIB, 0, (APTR)&name##_stubs};

	#define CALLBACK_5(ret, name, r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4, r5, t5, n5) \
		static ret name##_stubs(void)                                                         \
		{                                                                                     \
			t1 n1 = (t1)REG_##r1;                                                             \
			t2 n2 = (t2)REG_##r2;                                                             \
			t3 n3 = (t3)REG_##r3;                                                             \
			t4 n4 = (t4)REG_##r4;                                                             \
			t5 n5 = (t5)REG_##r5;                                                             \
			return (ret)name(n1, n2, n3, n4, n5);                                             \
		}                                                                                     \
		static struct EmulLibEntry name##_trap = {TRAP_LIB, 0, (APTR)&name##_stubs};

	#define CALLBACK_6(ret, name, r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4, r5, t5, n5, r6, t6, n6) \
		static ret name##_stubs(void)                                                                     \
		{                                                                                                 \
			t1 n1 = (t1)REG_##r1;                                                                         \
			t2 n2 = (t2)REG_##r2;                                                                         \
			t3 n3 = (t3)REG_##r3;                                                                         \
			t4 n4 = (t4)REG_##r4;                                                                         \
			t5 n5 = (t5)REG_##r5;                                                                         \
			t6 n6 = (t6)REG_##r6;                                                                         \
			return (ret)name(n1, n2, n3, n4, n5, n6);                                                     \
		}                                                                                                 \
		static struct EmulLibEntry name##_trap = {TRAP_LIB, 0, (APTR)&name##_stubs};

	#define CALLBACK_7(ret, name, r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4, r5, t5, n5, r6, t6, n6, r7, t7, n7) \
		static ret name##_stubs(void)                                                                                 \
		{                                                                                                             \
			t1 n1 = (t1)REG_##r1;                                                                                     \
			t2 n2 = (t2)REG_##r2;                                                                                     \
			t3 n3 = (t3)REG_##r3;                                                                                     \
			t4 n4 = (t4)REG_##r4;                                                                                     \
			t5 n5 = (t5)REG_##r5;                                                                                     \
			t6 n6 = (t6)REG_##r6;                                                                                     \
			t7 n7 = (t7)REG_##r7;                                                                                     \
			return (ret)name(n1, n2, n3, n4, n5, n6, n7);                                                             \
		}                                                                                                             \
		static struct EmulLibEntry name##_trap = {TRAP_LIB, 0, (APTR)&name##_stubs};

	#define GET_DOPUS_CALLBACK(name) (APTR)&name##_trap

#else
	#define GET_DOPUS_CALLBACK(name) (APTR)&name
#endif

//#if defined(__amigaos4__) || defined(__MORPHOS__)
#if defined(__MORPHOS__)
CALLBACK_3(DirEntry *, HookCreateFileEntry, a0, Lister *, lister, a1, struct FileInfoBlock *, fib, d0, BPTR, lock)

CALLBACK_3(DirEntry *, HookAddFileEntry, a0, Lister *, lister, a1, DirEntry *, entry, d0, BOOL, sort)

CALLBACK_3(void, HookFileSet, a0, Lister *, lister, a1, DirEntry *, entry, a2, struct TagItem *, tags)

CALLBACK_4(void,
		   HookSortFileList,
		   a0,
		   Lister *,
		   lister,
		   a1,
		   struct List *,
		   list,
		   d0,
		   long,
		   file_count,
		   d1,
		   long,
		   dir_count)

CALLBACK_2(void, HookResortLister, a0, Lister *, lister, a1, ListFormat *, format)

CALLBACK_2(void, HookRefreshLister, a0, Lister *, lister, d0, ULONG, flags)

CALLBACK_2(void, HookLockFileList, a0, Lister *, lister, d0, BOOL, exclusive)

CALLBACK_1(void, HookUnlockFileList, a0, Lister *, lister)

CALLBACK_2(DirEntry *, HookFindFileEntry, a0, Lister *, lister, a1, char *, name)

CALLBACK_3(BOOL, HookSetFileComment, a0, Lister *, lister, a1, char *, name, a2, char *, comment)

CALLBACK_2(void, HookRemoveFileEntry, a0, Lister *, lister, a1, DirEntry *, entry)

CALLBACK_3(BOOL, HookFileQuery, a0, Lister *, lister, a1, DirEntry *, entry, a2, struct TagItem *, tags)

CALLBACK_2(void, HookShowHelp, a0, char *, file_name, a1, char *, node_name)

CALLBACK_1(APTR, HookConvertEntry, a0, FunctionEntry *, entry)

CALLBACK_1(Lister *, HookGetLister, a0, PathNode *, path)

CALLBACK_2(PathNode *, HookGetSource, a0, FunctionHandle *, handle, a1, char *, pathbuf)

CALLBACK_2(PathNode *, HookNextSource, a0, FunctionHandle *, handle, a1, char *, pathbuf)

CALLBACK_1(void, HookUnlockSource, a0, FunctionHandle *, handle)

CALLBACK_2(PathNode *, HookGetDest, a0, FunctionHandle *, handle, a1, char *, pathbuf)

CALLBACK_2(void, HookEndSource, a0, FunctionHandle *, handle, d0, long, complete)

CALLBACK_2(void, HookEndDest, a0, FunctionHandle *, handle, d0, long, complete)

CALLBACK_1(void, HookFirstEntry, a0, FunctionHandle *, handle)

CALLBACK_1(APTR, HookGetEntry, a0, FunctionHandle *, handle)

CALLBACK_2(ULONG, HookExamineEntry, a0, FunctionEntry *, entry, d0, long, type)

CALLBACK_3(void, HookEndEntry, a0, FunctionHandle *, handle, a1, APTR, entry, d0, BOOL, deselect)

CALLBACK_1(void, HookRemoveEntry, a0, FunctionEntry *, entry)

CALLBACK_1(long, HookEntryCount, a0, FunctionHandle *, handle)

CALLBACK_2(void, HookReloadEntry, a0, FunctionHandle *, handle, a1, FunctionEntry *, entry)

CALLBACK_4(void,
		   HookAddFile,
		   a0,
		   FunctionHandle *,
		   handle,
		   a1,
		   char *,
		   path,
		   a2,
		   struct FileInfoBlock *,
		   fib,
		   a3,
		   Lister *,
		   lister)

CALLBACK_4(void, HookDelFile, a0, FunctionHandle *, handle, a1, char *, path, a2, char *, name, a3, Lister *, lister)

CALLBACK_5(void,
		   HookLoadFile,
		   a0,
		   FunctionHandle *,
		   handle,
		   a1,
		   char *,
		   path,
		   a2,
		   char *,
		   name,
		   d0,
		   long,
		   flags,
		   d1,
		   BOOL,
		   reload)

CALLBACK_1(void, HookDoChanges, a0, FunctionHandle *, handle)

CALLBACK_1(BOOL, HookCheckAbort, a0, FunctionHandle *, handle)

CALLBACK_1(struct Window *, HookGetWindow, a0, PathNode *, path)

CALLBACK_1(struct MsgPort *, HookGetPort, a0, char *, portname)

CALLBACK_1(struct Screen *, HookGetScreen, a0, char *, screenname)

CALLBACK_0(DOpusScreenData *, HookGetScreenData)

CALLBACK_1(void, HookFreeScreenData, a0, APTR, data)

CALLBACK_3(void, HookOpenProgress, a0, PathNode *, path, a1, char *, operation, d0, long, total)

CALLBACK_3(void, HookUpdateProgress, a0, PathNode *, path, a1, char *, name, d0, long, count)

CALLBACK_1(void, HookCloseProgress, a0, PathNode *, path)

CALLBACK_6(long,
		   HookReplaceReq,
		   a0,
		   struct Window *,
		   window,
		   a1,
		   struct Screen *,
		   screen,
		   a2,
		   IPCData *,
		   ipc,
		   a3,
		   struct FileInfoBlock *,
		   file1,
		   a4,
		   struct FileInfoBlock *,
		   file2,
		   d0,
		   long,
		   default_option)

CALLBACK_1(ULONG, HookGetPointer, a0, struct pointer_packet *, ptr)

CALLBACK_1(void, HookFreePointer, a0, struct pointer_packet *, ptr)

CALLBACK_4(ULONG,
		   HookSendCommand,
		   a0,
		   FunctionHandle *,
		   handle,
		   a1,
		   char *,
		   command,
		   a2,
		   char **,
		   result,
		   d0,
		   ULONG,
		   flags)

CALLBACK_1(void, HookCheckDesktop, a0, char *, path)

CALLBACK_1(UWORD, HookGetDesktop, a0, char *, path)

CALLBACK_1(void, HookGetThemes, a0, char *, path)

CALLBACK_2(BOOL, HookScript, a0, char *, name, a1, char *, data)

CALLBACK_1(short, HookDesktopPopup, a0, ULONG, flags)

CALLBACK_5(long,
		   HookRexxCommand,
		   a0,
		   char *,
		   command,
		   a1,
		   char *,
		   result,
		   d0,
		   long,
		   length,
		   a2,
		   struct MsgPort *,
		   replyport,
		   d1,
		   ULONG,
		   flags)

CALLBACK_6(long,
		   HookFileRequest,
		   a0,
		   struct Window *,
		   parent,
		   a1,
		   char *,
		   title,
		   a2,
		   char *,
		   initial_path,
		   a3,
		   char *,
		   pathname,
		   d0,
		   ULONG,
		   flags,
		   d1,
		   char *,
		   pattern)
#endif

#endif /* __CALLBACK_MAIN_H__ */
