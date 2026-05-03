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

#ifndef _DOPUS_BUFFERS
#define _DOPUS_BUFFERS

#define GETDIRF_CANMOVEEMPTY (1 << 0)
#define GETDIRF_CANCHECKBUFS (1 << 1)
#define GETDIRF_REREADINGOLD (1 << 2)
#define GETDIRF_RESELECT (1 << 3)
#define GETDIRF_DOCHECKBUFS (1 << 4)
#define GETDIRF_GET_ICONS (1 << 5)
#define GETDIRF_CUSTOM (1 << 6)

#define BRLF_MUST_EXIST (1 << 0)
#define BRLF_NO_LOCK (1 << 1)

extern struct Interrupt buffer_mem_interrupt;

// Prototypes
DirBuffer *buffer_new(void);
void buffer_free(DirBuffer *);
BOOL buffer_freedir(DirBuffer *, BOOL);
#ifdef USE_64BIT
DirEntry *create_file_entry(DirBuffer *,
							BPTR,
							char *,
							UQUAD,
							short,
							struct DateStamp *,
							char *,
							ULONG,
							short,
							char *,
							char *,
							NetworkInfo *);
#else
DirEntry *create_file_entry(DirBuffer *,
							BPTR,
							char *,
							unsigned long,
							short,
							struct DateStamp *,
							char *,
							ULONG,
							short,
							char *,
							char *,
							NetworkInfo *);
#endif
DirEntry *create_file_entry_fib(DirBuffer *, BPTR, struct FileInfoBlock *, short, char *, char *, NetworkInfo *);
DirEntry *copy_file_entry(DirBuffer *, DirEntry *);
DirEntry *add_file_entry(DirBuffer *, DirEntry *, DirEntry *);
void remove_file_entry(DirBuffer *, DirEntry *);
void free_file_memory(DirEntry *);
BOOL buffer_test_file(DirEntry *, DirBuffer *);
void buffer_reject_file(DirEntry *, DirBuffer *);
void buffer_do_rejects(DirBuffer *);
void buffer_replace_rejects(DirBuffer *buffer, BOOL);

void buffer_copy(DirBuffer *, DirBuffer *, Lister *);
void buffers_clear(BOOL);
void renamebuffers(char *, char *);
void buffer_change_volume_name(char *, char *, struct DateStamp *);
int check_special_buffer(Lister *, BOOL);
void buffer_show_special(Lister *, char *);
void lister_check_diskchange(Lister *lister);

void buffer_lock(DirBuffer *, BOOL);
void buffer_unlock(DirBuffer *);
void buffer_clear_lock(DirBuffer *buffer, short);
void buffer_inactive(DirBuffer *buffer, short arg);
void buffer_active(DirBuffer *buffer, short arg);

BOOL buffer_show_filetypes(ListFormat *format);
BOOL buffer_show_versions(ListFormat *format);

void buffer_sort_list(DirBuffer *buffer, struct MinList *file_list, long file_count, long dir_count);
void buffer_sort_array(DirEntry **array, long count, short sort_method);
void buffer_insert_sort_list(DirBuffer *buffer, struct MinList *file_list);
int ASM namesort(REG(a0, char *str1), REG(a1, char *str2));
void buffer_sort_reversesep(DirBuffer *buffer, BOOL do_reverse);
void buffer_sort_selected(DirBuffer *buffer);

int ASM buffer_sort_entries_name(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_size(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_protection(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_date(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_comment(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_description(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_version(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_owner(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_group(REG(a0, DirEntry *), REG(a1, DirEntry *));
int ASM buffer_sort_entries_netprot(REG(a0, DirEntry *), REG(a1, DirEntry *));

void direntry_add_network(DirBuffer *, DirEntry *entry, NetworkInfo *network_info);
BOOL direntry_add_string(DirBuffer *buffer, DirEntry *entry, ULONG type, char *string);
void direntry_add_version(DirBuffer *buffer, DirEntry *entry, UWORD ver, UWORD rev, UWORD days);

void buffer_mem_handler(ULONG);

#endif
