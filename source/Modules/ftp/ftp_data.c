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

#include "ftp.h"
#include "ftp_opusftp.h"

ModuleInfo_7 module_info = {1,				// Version
							"ftp.module",	// Module name
							"ftp.catalog",	// Catalog name
							0,				// Flags (0 for now)
							7,				// Number of functions in module
							{{0, "FTPAddressBook", MSG_FTP_ADDRBOOK, 0, ADDRBOOK_TEMPLATE},
							 {1, "FTPConnect", MSG_FTP_CONNECT_SITE, 0, CONNECT_TEMPLATE},
							 {2, "FTPCommand", MSG_FTP_COMMAND, FUNCF_NEED_SOURCE, COMMAND_TEMPLATE},
							 {3, "FTPSetVar", MSG_FTP_SETVAR, FUNCF_WANT_SOURCE, SETVAR_TEMPLATE},
							 {4, "FTPOptions", MSG_FTP_OPTIONS, FUNCF_WANT_SOURCE, OPTIONS_TEMPLATE},
							 {5, "FTPAdd", MSG_FTP_FTPADD, FUNCF_NEED_SOURCE, ADD_TEMPLATE},
							 {6, "FTPQuit", MSG_FTP_QUIT, 0, QUIT_TEMPLATE}}};

/************** gui stuff *****************************/

struct TagItem
	ftp_main_layout_tags[] = {{GTCustom_LayoutRel, GAD_FTP_MAIN_LAYOUT}, {GTCustom_CopyTags, TRUE}, {TAG_DONE}},

	ftp_sites_tags[] = {{DLV_ShowSelected, 0},
						// The docs say 1 for drag in all directions, 2 for sideways only
						// In reality, it's the other way around
						{DLV_DragNotify, 2},
						{TAG_MORE, (IPTR)ftp_main_layout_tags}},

	ftp_rhs_layout_tags[] = {{GTCustom_LayoutRel, GAD_FTP_SITE_LAYOUT}, {GTCustom_CopyTags, TRUE}, {TAG_DONE}},

	ftp_anon_tags[] = {{GA_Disabled, TRUE},
					   {GTCustom_NoGhost, TRUE},
					   {GTCustom_Recessed, TRUE},
					   {TAG_MORE, (IPTR)ftp_rhs_layout_tags}},

	ftp_port_tags[] = {{GTCustom_Justify, JUSTIFY_CENTER}, {TAG_MORE, (IPTR)ftp_rhs_layout_tags}},

	ftp_custom_relative_tags[] = {{GTCustom_LayoutRel, GAD_FTP_CUSTOM_LAYOUT}, {GTCustom_CopyTags, TRUE}, {TAG_DONE}},

	// Options
	ftp_customs_options_taglist[] = {{GA_Disabled, TRUE},
									 {GTCustom_NoGhost, TRUE},
									 {GTCustom_Recessed, TRUE},
									 {TAG_MORE, (IPTR)ftp_custom_relative_tags}};

#define STRHGT 6
#define CHECKBOX_HGT STRHGT

#define BUTWIDTH 7
#define BUTWID2 9

ConfigWindow ftp_main_window = {{POS_CENTER, POS_CENTER, 62, 10}, {0, 0, 46, 64}};

ObjectDef ftp_main_objects[] = {

	// Background area
	{OD_AREA,
	 0,
	 {0, 0, 30, SIZE_MAX_LESS - 1},
	 {2, 2, -2, -10},
	 0,
	 AREAFLAG_RAISED | AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_FTP_MAIN_LAYOUT,
	 0},

	// FTP sites
	{OD_GADGET,
	 MY_LISTVIEW_KIND,
	 {0, 1, SIZE_MAX_LESS - 1, SIZE_MAX_LESS - 2},
	 {4, 4, 0, -2},
	 MSG_FTP_SITES,
	 LISTVIEWFLAG_CURSOR_KEYS,
	 GAD_FTP_SITES,
	 ftp_sites_tags},

	// New
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, BUTWIDTH, 1},
	 {4, -2, 0, 6},
	 MSG_FTP_NEW,
	 0,
	 GAD_FTP_NEW,
	 ftp_main_layout_tags},

	// Edit
	{OD_GADGET,
	 BUTTON_KIND,
	 {BUTWIDTH, POS_RIGHT_JUSTIFY, BUTWIDTH, 1},
	 {4, -2, 0, 6},
	 MSG_FTP_EDIT,
	 0,
	 GAD_FTP_EDIT,
	 ftp_main_layout_tags},

	// Delete
	{OD_GADGET,
	 BUTTON_KIND,
	 {BUTWIDTH * 2, POS_RIGHT_JUSTIFY, BUTWIDTH, 1},
	 {4, -2, 0, 6},
	 MSG_DELETE,
	 0,
	 GAD_FTP_DELETE,
	 ftp_main_layout_tags},

	// Sort
	{OD_GADGET,
	 BUTTON_KIND,
	 {BUTWIDTH * 3, POS_RIGHT_JUSTIFY, BUTWIDTH, 1},
	 {4, -2, 0, 6},
	 MSG_SORT,
	 0,
	 GAD_FTP_SORT,
	 ftp_main_layout_tags},

	// area rhs
	{OD_AREA,
	 0,
	 {30, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {2, 2, -2, -10},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_FTP_SITE_LAYOUT,
	 0},

	// Frame top rhs
	{OD_AREA,
	 TEXTPEN,
	 {31, 1, SIZE_MAX_LESS - 1, SIZE_MAX_LESS - 2},
	 {2, 2, -2, -10},
	 MSG_SITE_DETAILS,
	 AREAFLAG_TITLE,
	 0 /*GAD_FTP_SITE_LAYOUT*/,
	 0},

	// Site name
	{OD_GADGET,
	 TEXT_KIND,
	 {8, 1, SIZE_MAX_LESS - 2, 1},
	 {4, 12, -4, STRHGT},
	 MSG_SITE_NAME,
	 POSFLAG_ADJUST_POS_X,
	 GAD_FTP_SITE_NAME,
	 ftp_rhs_layout_tags},

	// Host name
	{OD_GADGET,
	 TEXT_KIND,
	 {8, 2, SIZE_MAX_LESS - 2, 1},
	 {4, 20, -4, STRHGT},
	 MSG_SITE_HOST,
	 POSFLAG_ADJUST_POS_X,
	 GAD_FTP_HOST_NAME,
	 ftp_rhs_layout_tags},

	// Anonymous
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {8, 3, 0, 1},
	 {4, 28, 24, CHECKBOX_HGT},
	 MSG_SITE_ANON,
	 POSFLAG_ADJUST_POS_X | PLACETEXT_LEFT,
	 GAD_FTP_ANON,
	 ftp_anon_tags},

	// Port name
	{OD_GADGET,
	 NUMBER_KIND,
	 {POS_RIGHT_JUSTIFY - 3, 3, 7, 1},
	 {4, 28, -4, STRHGT},
	 MSG_SITE_PORT,
	 POSFLAG_ADJUST_POS_X,
	 GAD_FTP_PORT,
	 ftp_port_tags},

	// Frame area custom rhs middle
	{OD_AREA,
	 TEXTPEN,
	 {31, 5, SIZE_MAX_LESS - 1, SIZE_MAX_LESS - 2},
	 {2, 2 + 8 * 4, -2, -10},
	 MSG_ADVANCED_SETTINGS,
	 AREAFLAG_TITLE | AREAFLAG_RECESSED,
	 GAD_FTP_CUSTOM_LAYOUT,
	 0},

	// Custom Options
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {1, 0, 0, 1},
	 {4, 6, 24, STRHGT},
	 MSG_CUSTOM_OPTIONS,
	 0,
	 GAD_FTP_CUSTOM_OPTIONS,
	 ftp_customs_options_taglist},

	// Save
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {2, -2, 0, 6},
	 MSG_FTP_OK,  // MSG_FTP_SAVE
	 0,
	 GAD_FTP_SAVE,
	 0},

	// Connect
	{OD_GADGET, BUTTON_KIND, {16, POS_RIGHT_JUSTIFY, 12, 1}, {0, -2, 0, 6}, MSG_FTP_CONNECT, 0, GAD_FTP_CONNECT, 0},

	// Default Options
	{OD_GADGET,
	 BUTTON_KIND,
	 {32, POS_RIGHT_JUSTIFY, 20, 1},
	 {0, -2, 0, 6},
	 MSG_DEFAULT_OPTIONS,
	 0,
	 GAD_FTP_SET_DEFAULT_OPTIONS,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-2, -2, 0, 6},
	 MSG_FTP_CANCEL_ULINE,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_FTP_CANCEL,
	 0},

	{OD_END}};

/******************************************************************
 *	new stuff
 *
 ******************************************************************/

// ftp environment window dimensions
ConfigWindow ftp_environment_window = {{POS_CENTER, POS_CENTER, 58 /*64*/, 13}, {0, 0, 36, 64 + 4}};

// Labels
static UWORD allow_resume_labels[] = {MSG_REPLACE_ALWAYS, MSG_REPLACE_NEVER, MSG_REPLACE_ASK, 0},

			 unklinks_labels[] = {MSG_FTP_UNK_LINKS_DIR, MSG_FTP_UNK_LINKS_FILE, 0},

			 copy_type_labels[] = {MSG_COPY_DEFAULT, MSG_COPY_UPDATE, MSG_COPY_NEWER, 0},

			 tls_mode_labels[] = {MSG_FTP_TLS_PLAIN, MSG_FTP_TLS_EXPLICIT, 0},

			 addr_dc_labels[] = {MSG_ADR_CONNECT, MSG_ADR_EDIT, 0},

			 connection_labels[] = {MSG_FTP_CONNECTION_STANDARD, MSG_FTP_CONNECTION_FTPS, MSG_FTP_CONNECTION_SFTP, 0};

// Tags for gadgets
static struct TagItem

	// Option lister
	ftp_lister_taglist[] = {{DLV_ShowSelected, 0}, {DLV_NoScroller, 1}, {DLV_TopJustify, 1}, {TAG_END, 0}},

	// Relative to area tags[]={
	ftp_relative_taglist[] = {{GTCustom_LayoutRel, GAD_ENV_EDIT_AREA}, {TAG_END, 0}},

	ftp_options_anon_tags[] = {{GTST_MaxChars, PASSWORDLEN}, {TAG_MORE, (IPTR)ftp_relative_taglist}},

	ftp_options_log_tags[] = {{GTST_MaxChars, LOGNAMELEN}, {TAG_MORE, (IPTR)ftp_relative_taglist}},

	// string gadget for 4 chars
	ftp_integer_field_4[] = {{GTIN_MaxChars, 4}, {TAG_MORE, (IPTR)ftp_relative_taglist}},

	ftp_text_field_256[] = {{GTST_MaxChars, 256}, {TAG_MORE, (IPTR)ftp_relative_taglist}},

	allow_resume_taglist[] = {{GTCustom_LocaleLabels, (IPTR)allow_resume_labels},
							  {GTCustom_CopyTags, TRUE},
							  {TAG_MORE, (IPTR)ftp_relative_taglist}},

	copy_type_taglist[] = {{GTCustom_LocaleLabels, (IPTR)copy_type_labels},
						   {GTCustom_CopyTags, TRUE},
						   {TAG_MORE, (IPTR)ftp_relative_taglist}},

	tls_mode_taglist[] = {{GTCustom_LocaleLabels, (IPTR)tls_mode_labels},
						  {GTCustom_CopyTags, TRUE},
						  {TAG_MORE, (IPTR)ftp_relative_taglist}},

	unklinks_taglist[] = {{GTCustom_LocaleLabels, (IPTR)unklinks_labels},
						  {GTCustom_CopyTags, TRUE},
						  {TAG_MORE, (IPTR)ftp_relative_taglist}},

	addr_dc_taglist[] = {{GTCustom_LocaleLabels, (IPTR)addr_dc_labels},
						 {GTCustom_CopyTags, TRUE},
						 {TAG_MORE, (IPTR)ftp_relative_taglist}},

	ftp_custom_format_taglist[] = {{GA_Disabled, TRUE},
								   {GTCustom_NoGhost, TRUE},
								   {GTCustom_Recessed, TRUE},
								   {TAG_MORE, (IPTR)ftp_relative_taglist}};

#define LEFT_EDGE_GAD 10

// Flags objects
ObjectDef ftp_options_objects[] =
	{

		// Menu item lister
		{OD_GADGET,
		 MY_LISTVIEW_KIND,
		 {0, 0, 16, SIZE_MAX_LESS - 1},
		 {2, 2, 4, -10},
		 0,
		 LISTVIEWFLAG_CURSOR_KEYS,
		 GAD_ENV_ITEM_LISTER,
		 ftp_lister_taglist},

		// Edit area
		{OD_AREA,
		 0,
		 {16, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {8, 2, -2, -10},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_ENV_EDIT_AREA,
		 0},

		// Save
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 16, 1},
		 {2, -2, 4, 6},
		 MSG_FTP_SAVE,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_ENV_SAVE,
		 0},

		// Reset to Default
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_CENTER, POS_RIGHT_JUSTIFY, 24, 1},
		 {2, -2, 4, 6},
		 MSG_RESET_TO_DEFAULT,
		 0,
		 GAD_ENV_RESET_DEFAULT,
		 0},

		// Cancel
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 16, 1},
		 {-2, -2, 4, 6},
		 MSG_FTP_CANCEL_ULINE,
		 BUTTONFLAG_CANCEL_BUTTON,
		 GAD_ENV_CANCEL,
		 0},
		{OD_END}},

		  ftp_opt_connecton[] =
			  {
				  // Reconnection title
				  {OD_TEXT, TEXTPEN, {2, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_SUB_RECONNECTION, 0, 0, ftp_relative_taglist},

				  // Enable Retry
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 2, 0, 1},
				   {4, 4, 26, CHECKBOX_HGT},
				   MSG_FTP_OPTIONS_ENABLE_RETRY,
				   PLACETEXT_RIGHT,
				   GAD_ENV_ENABLE_RETRY,
				   ftp_relative_taglist},

				  // Retry Count
				  {OD_GADGET,
				   INTEGER_KIND,
				   {LEFT_EDGE_GAD, 3, 6, 1},
				   {4, 12, 4, STRHGT},
				   MSG_FTP_OPTIONS_RETRY_COUNT,
				   PLACETEXT_RIGHT,
				   GAD_ENV_RETRY_COUNT,
				   ftp_integer_field_4},

				  // Retry Time
				  {OD_GADGET,
				   INTEGER_KIND,
				   {LEFT_EDGE_GAD, 4, 6, 1},
				   {4, 20, 4, STRHGT},
				   MSG_FTP_OPTIONS_RETRY_DELAY,
				   PLACETEXT_RIGHT,
				   GAD_ENV_RETRY_DELAY,
				   ftp_integer_field_4},

				  // Retry on lost connect
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 5, 0, 1},
				   {4, 30, 26, CHECKBOX_HGT},
				   MSG_FTP_OPTIONS_ENABLE_RETRY_LOST,
				   PLACETEXT_RIGHT,
				   GAD_ENV_ENABLE_RETRY_LOST,
				   ftp_relative_taglist},

				  // Send NOOPs
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 6, 0, 1},
				   {4, 38, 26, CHECKBOX_HGT},
				   MSG_FTP_OPTIONS_NOOPS,
				   PLACETEXT_RIGHT,
				   GAD_ENV_NOOPS,
				   ftp_relative_taglist},

				  // FTP security mode
				  {OD_GADGET,
				   CYCLE_KIND,
				   {LEFT_EDGE_GAD, 7, 15, 1},
				   {4, 46, 24, 6},
				   MSG_FTP_SECURITY,
				   PLACETEXT_LEFT,
				   GAD_ENV_TLS_MODE,
				   tls_mode_taglist},

				  // Verify FTPS certificates
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 8, 0, 1},
				   {4, 56, 26, CHECKBOX_HGT},
				   MSG_FTP_VERIFY_TLS,
				   PLACETEXT_RIGHT,
				   GAD_ENV_TLS_VERIFY,
				   ftp_relative_taglist},
				  {OD_END}},

		  ftp_opt_misc[] =
			  {
				  // Miscellaneous
				  {OD_TEXT, TEXTPEN, {2, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_SUB_MISC, 0, 0, ftp_relative_taglist},

				  // Keep last dir
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 2, 0, 1},
				   {4, 4, 26, CHECKBOX_HGT},
				   MSG_FTP_KEEP_LAST_DIR,
				   PLACETEXT_RIGHT,
				   GAD_ENV_KEEP_LAST_DIR,
				   ftp_relative_taglist},

				  // Firewall - Passive Transfers
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 3, 0, 1},
				   {4, 12, 26, CHECKBOX_HGT},
				   MSG_FTP_PASSIVE_TRANSFER,
				   PLACETEXT_RIGHT,
				   GAD_ENV_PASSIVE,
				   ftp_relative_taglist},

				  // non-standard directories old fred_hack env
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 3 + 1, 0, 1},
				   {4, 12 + 8, 26, CHECKBOX_HGT},
				   MSG_FTP_SPECIAL_DIR,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SPECIAL_DIR,
				   ftp_relative_taglist},

				  // Timeout
				  {OD_GADGET,
				   INTEGER_KIND,
				   {LEFT_EDGE_GAD, 4 + 1, 6, 1},
				   {4, 20 + 8, 4, STRHGT},
				   MSG_FTP_NETWORK_TIMEOUT,
				   PLACETEXT_RIGHT,
				   GAD_ENV_TIMEOUT,
				   ftp_integer_field_4},

				  // List Update
				  {OD_GADGET,
				   INTEGER_KIND,
				   {LEFT_EDGE_GAD, 5 + 1, 6, 1},
				   {4, 28 + 8, 4, STRHGT},
				   MSG_FTP_LIST_UPDATE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_LIST_UPDATE,
				   ftp_integer_field_4},

				  {OD_TEXT, TEXTPEN, {2, 6 + 1, 0, 1}, {4, 38 + 8, 0, 0}, MSG_FTP_LINKS, 0, 0, ftp_relative_taglist},

				  // Logical Parent Dir / Safe Links
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 7 + 1, 0, 1},
				   {4, 44 + 8, 26, CHECKBOX_HGT},
				   MSG_FTP_LOGICAL_LINKS,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SAFE_LINKS,
				   ftp_relative_taglist},

				  // Unk links -> file
				  {OD_GADGET,
				   CYCLE_KIND,
				   {LEFT_EDGE_GAD, 8 + 1, 15, 1},
				   {4, 52 + 8, 24, 6},
				   MSG_FTP_UNK_LINKS,
				   PLACETEXT_LEFT,
				   GAD_ENV_UNK_LINKS,
				   unklinks_taglist},

				  {OD_END}},

		  ftp_opt_display[] =
			  {
				  // Display
				  {OD_TEXT, TEXTPEN, {2, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_SUB_DISPLAY, 0, 0, ftp_relative_taglist},

				  // Show Startup Messages
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 2, 0, 1},
				   {4, 4, 26, CHECKBOX_HGT},
				   MSG_SHOW_STARTUP_MSG,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SHOW_STARTUP_MSG,
				   ftp_relative_taglist},

				  // Show Directory Messages
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 3, 0, 1},
				   {4, 12, 26, CHECKBOX_HGT},
				   MSG_SHOW_DIR_MSG,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SHOW_DIR_MSG,
				   ftp_relative_taglist},

				  // Progress Windows (CD or List)
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 4, 0, 1},
				   {4, 20, 26, CHECKBOX_HGT},
				   MSG_FTP_PROGRESS_WINDOW,
				   PLACETEXT_RIGHT,
				   GAD_ENV_PROGRESS_WINDOW,
				   ftp_relative_taglist},

				  {OD_TEXT, TEXTPEN, {2, 5, 0, 1}, {4, 32, 0, 0}, MSG_TRANSFER_PPOGRESS, 0, 0, ftp_relative_taglist},

				  // Transfer Options
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 6, 0, 1},
				   {4, 36, 26, CHECKBOX_HGT},
				   MSG_TRANSFER_DETAILS,
				   PLACETEXT_RIGHT,
				   GAD_ENV_TRANSFER_DETAILS,
				   ftp_relative_taglist},

				  {OD_END}},

		  ftp_opt_scripts[] =
			  {
				  // Scripts
				  {OD_TEXT, TEXTPEN, {2, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_TEXT_SCRIPTS, 0, 0, ftp_relative_taglist},

				  // Connect
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 2, 0, 1},
				   {4, 4, 26, CHECKBOX_HGT},
				   MSG_FTP_SCRIPT_CONNECT_SUCCESS,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SCRIPT_CONNECT_SUCCESS,
				   ftp_relative_taglist},

				  // Connect
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 3, 0, 1},
				   {4, 10, 26, CHECKBOX_HGT},
				   MSG_FTP_SCRIPT_CONNECT_FAIL,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SCRIPT_CONNECT_FAIL,
				   ftp_relative_taglist},

				  // Copy Success
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 4, 0, 1},
				   {4, 16, 26, CHECKBOX_HGT},
				   MSG_FTP_SCRIPT_COPY_SUCCESS,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SCRIPT_COPY_SUCCESS,
				   ftp_relative_taglist},

				  // Copy Fail
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 5, 0, 1},
				   {4, 22, 26, CHECKBOX_HGT},
				   MSG_FTP_SCRIPT_COPY_FAIL,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SCRIPT_COPY_FAIL,
				   ftp_relative_taglist},

				  // Error
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 6, 0, 1},
				   {4, 28, 26, CHECKBOX_HGT},
				   MSG_FTP_SCRIPT_ERROR,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SCRIPT_ERROR,
				   ftp_relative_taglist},

				  // Close
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 7, 0, 1},
				   {4, 34, 26, CHECKBOX_HGT},
				   MSG_FTP_SCRIPT_CLOSE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SCRIPT_CLOSE,
				   ftp_relative_taglist},

				  // Activation time
				  {OD_GADGET,
				   INTEGER_KIND,
				   {LEFT_EDGE_GAD, 8, 6, 1},
				   {4, 42, 4, STRHGT},
				   MSG_FTP_SCRIPT_TIME,
				   PLACETEXT_RIGHT,
				   GAD_ENV_SCRIPT_TIME,
				   ftp_integer_field_4},
				  {OD_END}},

		  ftp_opt_index[] =
			  {
				  // Index
				  {OD_TEXT, TEXTPEN, {2, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_SUB_INDEX, 0, 0, ftp_relative_taglist},

				  // Index
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 2, 0, 1},
				   {4, 4, 26, CHECKBOX_HGT},
				   MSG_FTP_INDEX,
				   PLACETEXT_RIGHT,
				   GAD_ENV_INDEX_ENABLE,
				   ftp_relative_taglist},

				  // Auto Download
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 3, 0, 1},
				   {4, 12, 26, CHECKBOX_HGT},
				   MSG_FTP_AUTO_DOWNLOAD,
				   PLACETEXT_RIGHT,
				   GAD_ENV_AUTO_DOWNLOAD,
				   ftp_relative_taglist},

				  // Download size
				  {OD_GADGET,
				   INTEGER_KIND,
				   {LEFT_EDGE_GAD, 4, 6, 1},
				   {4, 20, 4, STRHGT},
				   MSG_FTP_AUTO_DOWNLOAD_SIZE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_AUTO_DOWNLOAD_SIZE,
				   ftp_integer_field_4},
				  {OD_END}},

		  ftp_opt_global[] =
			  {
				  // Log
				  {OD_TEXT, TEXTPEN, {2, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_GLOBAL_SETTINGS, 0, 0, ftp_relative_taglist},

				  // Log Enable
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 2, 0, 1},
				   {4, 4, 26, CHECKBOX_HGT},
				   MSG_LOG_ENABLE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_LOG_ENABLE,
				   ftp_relative_taglist},

				  // Show Debug
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 3, 0, 1},
				   {4, 12, 26, CHECKBOX_HGT},
				   MSG_FTP_LOG_DEBUG,
				   PLACETEXT_RIGHT,
				   GAD_ENV_DEBUG,
				   ftp_relative_taglist},

				  // Disable Stayin' Alive pings (was DOpus/NoBeeGees env var)
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 4, 0, 1},
				   {4, 20, 26, CHECKBOX_HGT},
				   MSG_FTP_NO_KEEP_ALIVE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_NO_KEEP_ALIVE,
				   ftp_relative_taglist},

				  // Log file
				  {OD_GADGET,
				   STRING_KIND,
				   {LEFT_EDGE_GAD, 5, SIZE_MAX_LESS - 1, 1},
				   {4, 28, -4, STRHGT},
				   MSG_LOG_FILE,
				   0,
				   GAD_ENV_LOG_FILE,
				   ftp_options_log_tags},

				  // addr name
				  {OD_TEXT, TEXTPEN, {2, 7, 0, 1}, {4, 36, 0, 0}, MSG_FTP_ADDR_NAME, 0, 0, ftp_relative_taglist},

				  //  address book double-click edit/connect
				  {OD_GADGET,
				   CYCLE_KIND,
				   {LEFT_EDGE_GAD, 7, 10, 1},
				   {4, 36, 24, 6},
				   MSG_ADR_DC,
				   PLACETEXT_RIGHT,
				   GAD_ENV_ADDR_DC,
				   addr_dc_taglist},

				  // auto close
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 8, 0, 1},
				   {4, 42 + 4, 26, CHECKBOX_HGT},
				   MSG_AUTO_CLOSE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_ADDR_AUTO,
				   ftp_relative_taglist},

				  // Anonymous password
				  {OD_GADGET,
				   STRING_KIND,
				   {LEFT_EDGE_GAD, 10, SIZE_MAX_LESS - 1, 1},
				   {4, 44 + 4, -4, STRHGT},
				   MSG_ANON_PASS,
				   0,
				   GAD_ENV_ANON_PASSWORD,
				   ftp_options_anon_tags},
				  {OD_END}},

		  ftp_opt_lister[] =
			  {
				  // Lister Display
				  {OD_TEXT, TEXTPEN, {2, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_TEXT_LISTER, 0, 0, ftp_relative_taglist},

				  // Toolbar Glass
				  {OD_GADGET,
				   FILE_BUTTON_KIND,
				   {LEFT_EDGE_GAD, 3, 0, 1},
				   {4, 4, 28, STRHGT},
				   MSG_LISTER_TOOLBAR,
				   0,
				   GAD_ENV_TOOLBAR_GLASS,
				   ftp_relative_taglist},

				  // Toolbar string
				  {OD_GADGET,
				   STRING_KIND,
				   {LEFT_EDGE_GAD, 3, SIZE_MAX_LESS - 1, 1},
				   {32, 4, -4, STRHGT},
				   0,
				   0,
				   GAD_ENV_TOOLBAR,
				   ftp_text_field_256},

				  // Format text
				  {OD_TEXT, TEXTPEN, {4, 6, 0, 1}, {4, 13, 0, 0}, MSG_DEFAULT_FORMAT, 0, 0, ftp_relative_taglist},

				  // has default
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 7, 0, 1},
				   {4, 18, 24, CHECKBOX_HGT},
				   0,
				   0,
				   GAD_ENV_CUST_FORMAT,
				   ftp_custom_format_taglist},

				  // Set
				  {OD_GADGET,
				   BUTTON_KIND,
				   {LEFT_EDGE_GAD + 5, 7, SIZE_MAX_LESS - 2, 1},
				   {4, 18, 4, 6},
				   MSG_FORMAT_SET,
				   0,
				   GAD_ENV_SET_FORMAT,
				   ftp_relative_taglist},

				  {OD_END}},

		  ftp_opt_copyflags[] =
			  {

				  // Copy
				  {OD_TEXT, TEXTPEN, {1, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_TEXT_COPY, 0, 0, ftp_relative_taglist},

				  // Copy Type
				  {OD_GADGET,
				   CYCLE_KIND,
				   {LEFT_EDGE_GAD, 2, 11, 1},
				   {4, 0 + 4, 24, 6},
				   MSG_COPY_FLAGS,
				   PLACETEXT_RIGHT,
				   GAD_ENV_COPY_TYPE,
				   copy_type_taglist},

				  // Replace/Resume
				  {OD_GADGET,
				   CYCLE_KIND,
				   {LEFT_EDGE_GAD, 3, 11, 1},
				   {4, 8 + 4, 24, 6},
				   MSG_REPLACE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_REPLACE,
				   allow_resume_taglist},

				  // Set Source Archive
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 4, 0, 1},
				   {4, 16 + 4, 26, 4},
				   MSG_COPY_SET_ARCHIVE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_COPY_SET_ARCHIVE,
				   ftp_relative_taglist},

				  // Set Comment to URL
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 5, 0, 1},
				   {4, 22 + 4, 26, CHECKBOX_HGT},
				   MSG_COPY_URL_COMMENT,
				   PLACETEXT_RIGHT,
				   GAD_ENV_COPY_URL_COMMENT,
				   ftp_relative_taglist},

				  // Rescan
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 6, 0, 1},
				   {4, 28 + 4, 26, CHECKBOX_HGT},
				   MSG_COPY_RESCAN,
				   PLACETEXT_RIGHT,
				   GAD_ENV_COPY_RESCAN,
				   ftp_relative_taglist},

				  // special space
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {LEFT_EDGE_GAD, 7, 0, 1},
				   {4, 28 + 6 + 4, 26, CHECKBOX_HGT},
				   MSG_COPY_SPACE,
				   PLACETEXT_RIGHT,
				   GAD_ENV_COPY_RECURSIVE_SPECIAL,
				   ftp_relative_taglist},

				  {OD_END}},

		  ftp_opt_copyattr[] = {

			  // Copy
			  {OD_TEXT, TEXTPEN, {1, 1, 0, 1}, {4, 0, 0, 0}, MSG_FTP_TEXT_COPY_ATTRIBUTES, 0, 0, ftp_relative_taglist},

			  // Also copy source's...
			  {OD_TEXT, TEXTPEN, {2, 2, 0, 0}, {4, 8, 0, 0}, MSG_ALSO_COPY_SOURCE, 0, 0, ftp_relative_taglist},

			  // Use Opus Default
			  {OD_GADGET,
			   CHECKBOX_KIND,
			   {LEFT_EDGE_GAD, 3, 0, 1},
			   {4, 16, 26, CHECKBOX_HGT},
			   MSG_USE_OPUS_SETTINGS,
			   PLACETEXT_RIGHT,
			   GAD_ENV_COPY_OPUS_DEFAULT,
			   ftp_relative_taglist},

			  // Copy datestamp
			  {OD_GADGET,
			   CHECKBOX_KIND,
			   {LEFT_EDGE_GAD, 4, 0, 1},
			   {4, 24, 26, 4},
			   MSG_COPY_DATESTAMP,
			   PLACETEXT_RIGHT,
			   GAD_ENV_COPY_DATESTAMP,
			   ftp_relative_taglist},

			  // Copy protection bits
			  {OD_GADGET,
			   CHECKBOX_KIND,
			   {LEFT_EDGE_GAD, 5, 0, 1},
			   {4, 32, 26, 4},
			   MSG_COPY_PROTECTION,
			   PLACETEXT_RIGHT,
			   GAD_ENV_COPY_PROTECTION,
			   ftp_relative_taglist},

			  // Copy comment bits
			  {OD_GADGET,
			   CHECKBOX_KIND,
			   {LEFT_EDGE_GAD, 6, 0, 1},
			   {4, 40, 26, 4},
			   MSG_COPY_COMMENT,
			   PLACETEXT_RIGHT,
			   GAD_ENV_COPY_COMMENT,
			   ftp_relative_taglist},

			  {OD_END}};

// Sub options
SubOptionHandle ftp_custom_suboptions[] = {

	{ENV_SUB_COPYFLAGS, MSG_FTP_SUB_COPYFLAGS, ftp_opt_copyflags},
	{ENV_SUB_COPYATTR, MSG_FTP_SUB_COPYATTR, ftp_opt_copyattr},
	{ENV_SUB_LISTER, MSG_FTP_SUB_LISTER, ftp_opt_lister},
	{ENV_SUB_DISPLAY, MSG_FTP_SUB_DISPLAY, ftp_opt_display},
	{ENV_SUB_MISC, MSG_FTP_SUB_MISC, ftp_opt_misc},
	{ENV_SUB_SCRIPTS, MSG_FTP_SUB_SCRIPTS, ftp_opt_scripts},
	{ENV_SUB_INDEX, MSG_FTP_SUB_INDEX, ftp_opt_index},
	{ENV_SUB_CONNECTION, MSG_FTP_SUB_RECONNECTION, ftp_opt_connecton},
	{-1}};

SubOptionHandle ftp_default_suboptions[] = {{ENV_SUB_COPYFLAGS, MSG_FTP_SUB_COPYFLAGS, ftp_opt_copyflags},
											{ENV_SUB_COPYATTR, MSG_FTP_SUB_COPYATTR, ftp_opt_copyattr},
											{ENV_SUB_LISTER, MSG_FTP_SUB_LISTER, ftp_opt_lister},
											{ENV_SUB_DISPLAY, MSG_FTP_SUB_DISPLAY, ftp_opt_display},
											{ENV_SUB_MISC, MSG_FTP_SUB_MISC, ftp_opt_misc},
											{ENV_SUB_SCRIPTS, MSG_FTP_SUB_SCRIPTS, ftp_opt_scripts},
											{ENV_SUB_INDEX, MSG_FTP_SUB_INDEX, ftp_opt_index},
											{ENV_SUB_CONNECTION, MSG_FTP_SUB_RECONNECTION, ftp_opt_connecton},
											{ENV_SUB_GLOBAL, MSG_FTP_GLOBAL, ftp_opt_global},
											{-1}};

ConfigWindow

	ftp_connect_window = {{POS_CENTER, POS_CENTER, 45, 8}, {0, 0, 24, 84}};

static struct TagItem

	ftp_connect_layout_tags[] = {{GTCustom_LayoutRel, GAD_CONNECT_LAYOUT}, {GTCustom_CopyTags, TRUE}, {TAG_DONE}},

	ftp_connection_tags[] = {{GTCustom_LocaleLabels, (IPTR)connection_labels},
							 {GTCustom_CopyTags, TRUE},
							 {TAG_MORE, (IPTR)ftp_connect_layout_tags}},

	ftp_connect_name_tags[] = {{GTST_MaxChars, HOSTNAMELEN}, {TAG_MORE, (IPTR)ftp_connect_layout_tags}},

	ftp_connect_port_tags[] = {{GTIN_MaxChars, 5},
							   {GTCustom_Justify, JUSTIFY_CENTER},
							   {TAG_MORE, (IPTR)ftp_connect_layout_tags}},

	ftp_connect_user_tags[] = {{GTST_MaxChars, USERNAMELEN}, {TAG_MORE, (IPTR)ftp_connect_layout_tags}},

	ftp_connect_pass_tags[] = {{GTST_MaxChars, PASSWORDLEN},
							   {GTCustom_Secure, TRUE},
							   {TAG_MORE, (IPTR)ftp_connect_layout_tags}},

	ftp_connect_dir_tags[] = {{GTST_MaxChars, PATHLEN}, {TAG_MORE, (IPTR)ftp_connect_layout_tags}};

ObjectDef

	ftp_connect_objects[] = {

		// Background area
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {2, 2, -2, -10},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_CONNECT_LAYOUT,
		 0},

		// Name Glass
		{OD_GADGET,
		 FILE_BUTTON_KIND,
		 {13, 0, 0, 1},
		 {4, 4, 28, STRHGT},
		 MSG_EDIT_NAME,
		 0,
		 GAD_CONNECT_GLASS,
		 ftp_connect_layout_tags},

		// Name
		{OD_GADGET,
		 STRING_KIND,
		 {13, 0, SIZE_MAX_LESS - 1, 1},
		 {32, 4, -4, STRHGT},
		 0,
		 0,
		 GAD_CONNECT_NAME,
		 ftp_connect_name_tags},

		// Connection
		{OD_GADGET,
		 CYCLE_KIND,
		 {13, 1, SIZE_MAX_LESS - 1, 1},
		 {4, 12, -4, 6},
		 MSG_FTP_CONNECTION,
		 PLACETEXT_LEFT,
		 GAD_CONNECT_PROTOCOL,
		 ftp_connection_tags},

		// Host
		{OD_GADGET,
		 STRING_KIND,
		 {13, 2, SIZE_MAX_LESS - 1, 1},
		 {4, 20, -4, STRHGT},
		 MSG_EDIT_HOST,
		 0,
		 GAD_CONNECT_HOST,
		 ftp_connect_name_tags},

		// Anonymous
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {13, 3, 0, 1},
		 {4, 28, 24, CHECKBOX_HGT},
		 MSG_EDIT_ANON,
		 PLACETEXT_LEFT,
		 GAD_CONNECT_ANON,
		 ftp_connect_layout_tags},

		// Port
		{OD_GADGET,
		 INTEGER_KIND,
		 {POS_RIGHT_JUSTIFY - 2, 3, 7, 1},
		 {4, 28, 4, STRHGT},
		 MSG_EDIT_PORT,
		 0,
		 GAD_CONNECT_PORT,
		 ftp_connect_port_tags},

		// User
		{OD_GADGET,
		 STRING_KIND,
		 {13, 4, SIZE_MAX_LESS - 1, 1},
		 {4, 36, -4, STRHGT},
		 MSG_EDIT_USER,
		 0,
		 GAD_CONNECT_USER,
		 ftp_connect_user_tags},

		// Password
		{OD_GADGET,
		 STRING_KIND,
		 {13, 5, SIZE_MAX_LESS - 1, 1},
		 {4, 44, -4, STRHGT},
		 MSG_EDIT_PASSWORD,
		 0,
		 GAD_CONNECT_PASSWORD,
		 ftp_connect_pass_tags},

		// dir
		{OD_GADGET,
		 STRING_KIND,
		 {13, 7, SIZE_MAX_LESS - 1, 1},
		 {4, 60, -4, STRHGT},
		 MSG_EDIT_DIR,
		 0,
		 GAD_CONNECT_DIR,
		 ftp_connect_dir_tags},

		// Connect
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, BUTWID2, 1},
		 {2, -2, 4, 6},
		 MSG_FTP_CONNECT,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_CONNECT_CONNECT,
		 0},

		// Last Site
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_CENTER, POS_RIGHT_JUSTIFY, BUTWID2 + 6, 1},
		 {2, -2, 4, 6},
		 MSG_FTP_LAST_SITE,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_CONNECT_LAST,
		 0},

		// Cancel
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, BUTWID2, 1},
		 {-2, -2, 4, 6},
		 MSG_FTP_CANCEL_ULINE,
		 BUTTONFLAG_CANCEL_BUTTON,
		 GAD_CONNECT_CANCEL,
		 0},

		{OD_END}};

// Edit window
// uses similar settings as connect but some different gadgets and gadgetids

ConfigWindow

	ftp_edit_window = {{POS_CENTER, POS_CENTER, 45, 17}, {0, 0, 24, 72}};

// Labels
UWORD custom_options_labels[] = {MSG_DEFAULT_OPTIONS, MSG_CUSTOM_OPTIONS, 0};

static struct TagItem

	edit_custom_relative_tags[] = {{GTCustom_LayoutRel, GAD_EDIT_CUSTOM_LAYOUT}, {GTCustom_CopyTags, TRUE}, {TAG_DONE}},

	//
	edit_custom_options_taglist[] = {{GTCustom_LocaleLabels, (IPTR)custom_options_labels},
									 {GTCustom_CopyTags, TRUE},
									 {TAG_MORE, (IPTR)edit_custom_relative_tags}};

ObjectDef

	ftp_edit_objects[] = {

		// Background area
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {2, 2, -2, -10},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_CONNECT_LAYOUT, /* NOTE Uses layout tag*/
		 0},

		// Name
		{OD_GADGET,
		 STRING_KIND,
		 {13, 0, SIZE_MAX_LESS - 1, 1},
		 {4, 4, -4, STRHGT},
		 MSG_EDIT_NAME,
		 0,
		 GAD_EDIT_NAME,
		 ftp_connect_name_tags},

		// Connection
		{OD_GADGET,
		 CYCLE_KIND,
		 {13, 1, SIZE_MAX_LESS - 1, 1},
		 {4, 12, -4, 6},
		 MSG_FTP_CONNECTION,
		 PLACETEXT_LEFT,
		 GAD_EDIT_PROTOCOL,
		 ftp_connection_tags},

		// Host
		{OD_GADGET,
		 STRING_KIND,
		 {13, 2, SIZE_MAX_LESS - 1, 1},
		 {4, 20, -4, STRHGT},
		 MSG_EDIT_HOST,
		 0,
		 GAD_EDIT_HOST,
		 ftp_connect_name_tags},

		// Anonymous
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {13, 3, 0, 1},
		 {4, 28, 24, CHECKBOX_HGT},
		 MSG_EDIT_ANON,
		 PLACETEXT_LEFT,
		 GAD_EDIT_ANON,
		 ftp_connect_layout_tags},

		// Port
		{OD_GADGET,
		 INTEGER_KIND,
		 {POS_RIGHT_JUSTIFY - 2, 3, 7, 1},
		 {4, 28, 4, STRHGT},
		 MSG_EDIT_PORT,
		 0,
		 GAD_EDIT_PORT,
		 ftp_connect_port_tags},

		// User
		{OD_GADGET,
		 STRING_KIND,
		 {13, 4, SIZE_MAX_LESS - 1, 1},
		 {4, 36, -4, STRHGT},
		 MSG_EDIT_USER,
		 0,
		 GAD_EDIT_USER,
		 ftp_connect_user_tags},

		// Password
		{OD_GADGET,
		 STRING_KIND,
		 {13, 5, SIZE_MAX_LESS - 1, 1},
		 {4, 44, -4, STRHGT},
		 MSG_EDIT_PASSWORD,
		 0,
		 GAD_EDIT_PASSWORD,
		 ftp_connect_pass_tags},

		// dir
		{OD_GADGET,
		 STRING_KIND,
		 {13, 7, SIZE_MAX_LESS - 1, 1},
		 {4, 60, -4, STRHGT},
		 MSG_EDIT_DIR,
		 0,
		 GAD_EDIT_DIR,
		 ftp_connect_dir_tags},

		// Frame area custom rhs middle
		{OD_AREA,
		 TEXTPEN,
		 {1, 9, SIZE_MAX_LESS - 1, SIZE_MAX_LESS - 2},
		 {2, 72, -2, -10},
		 MSG_ADVANCED_SETTINGS,
		 AREAFLAG_TITLE | AREAFLAG_RECESSED,
		 GAD_EDIT_CUSTOM_LAYOUT,
		 0},

		// Custom Options
		{OD_GADGET,
		 MX_KIND,
		 {1, 0, 1, 1},
		 {4, 6, 7, 1},
		 0,
		 PLACETEXT_RIGHT,
		 GAD_EDIT_CUSTOM_OPTIONS,
		 edit_custom_options_taglist},

		// Set Custom
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY - 1, 1, 12, 1},
		 {2, 4, 0, 6},
		 MSG_SET_CUSTOM,
		 0,
		 GAD_EDIT_SET_CUSTOM_OPTIONS,
		 edit_custom_relative_tags},

		// OK
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, BUTWID2, 1},
		 {2, -2, 4, 6},
		 MSG_FTP_OK,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_EDIT_OK,
		 0},

		// Cancel
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, BUTWID2, 1},
		 {-2, -2, 4, 6},
		 MSG_FTP_CANCEL_ULINE,
		 BUTTONFLAG_CANCEL_BUTTON,
		 GAD_EDIT_CANCEL,
		 0},

		{OD_END}};

MenuData site_menus[] = {{NM_TITLE, 0, MSG_ENV_MENU_PROJECT, 0},
						 {NM_ITEM, MENU_FTP_OPEN, MSG_ENV_MENU_OPEN, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('O')},
						 {NM_ITEM, MENU_FTP_IMPORT, MSG_ENV_MENU_IMPORT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('I')},
						 {NM_ITEM, MENU_FTP_IMPORT_AMFTP, MSG_ENV_MENU_IMPORT_AMFTP, 0},
						 {NM_ITEM, 0, NM_BAR_LABEL, 0},
						 {NM_ITEM, MENU_FTP_SAVE, MSG_ENV_MENU_SAVE, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('S')},
						 {NM_ITEM, MENU_FTP_SAVEAS, MSG_ENV_MENU_SAVEAS, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('A')},
						 {NM_ITEM, 0, NM_BAR_LABEL, 0},
						 {NM_ITEM, MENU_FTP_EXPORT, MSG_ENV_MENU_EXPORT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('E')},
						 {NM_ITEM, 0, NM_BAR_LABEL, 0},
						 {NM_ITEM, MENU_FTP_CANCEL, MSG_ENV_MENU_QUIT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('Q')},
						 {NM_END}};

MenuData options_menus[] = {
	{NM_TITLE, 0, MSG_ENV_MENU_PROJECT, 0},
	{NM_ITEM, MENU_ENV_OPEN, MSG_ENV_MENU_OPEN, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('O')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	//{NM_ITEM,MENU_ENV_SAVE,MSG_ENV_MENU_SAVE,MENUFLAG_USE_SEQ|MENUFLAG_MAKE_SEQ('S')},
	{NM_ITEM, MENU_ENV_SAVEAS, MSG_ENV_MENU_SAVEAS, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('A')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_ENV_CANCEL, MSG_ENV_MENU_QUIT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('Q')},
	{NM_TITLE, 0, MSG_ENV_MENU_EDIT, 0},
	{NM_ITEM, MENU_ENV_RESET_DEFAULTS, MSG_ENV_MENU_RESET_DEFAULTS, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('D')},
	{NM_ITEM, MENU_ENV_LAST_SAVED, MSG_ENV_MENU_LAST_SAVED, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('L')},
	{NM_ITEM, MENU_ENV_RESTORE, MSG_ENV_MENU_RESTORE, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('R')},
	{NM_END}};

/********************************************************/

ConfigWindow ftp_list_window = {{POS_CENTER, POS_CENTER, 42, 10}, {0, 0, 30, 20}};

static struct TagItem ftp_lister_tags[] = {{GTLV_ShowSelected, 0}, {TAG_END}};

ObjectDef ftp_list_objects[] = {

	// Lister
	{OD_GADGET,
	 MY_LISTVIEW_KIND,
	 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {3, 3, -3, -12},
	 0,
	 LISTVIEWFLAG_CURSOR_KEYS,
	 GAD_LIST_SITES,
	 ftp_lister_tags},

	// Okay
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {3, -3, 4, 6},
	 MSG_FTP_OKAY,
	 BUTTONFLAG_OKAY_BUTTON,
	 GAD_LIST_OK,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-3, -3, 4, 6},
	 MSG_FTP_CANCEL,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_LIST_CANCEL,
	 0},

	{OD_END}};
