#include "font.h"

const char USED_VAR version[] = "\0$VER: ViewFont " CMD_STRING;

const ConfigWindow font_window = {{POS_CENTER, POS_CENTER, 40, 9}, {0, 0, 44, 67}};

const static char *dummy_labels[] = {"", 0};

const struct TagItem

	font_name_tags[] = {{GTST_MaxChars, 32}, {TAG_END, 0}},

	font_size_tags[] = {{GTIN_MaxChars, 3},
						{GTCustom_LayoutPos, GAD_FONT_FONT_POPUP},
						{GTCustom_CopyTags, 0},
						{TAG_END, 0}},

	font_bold_tags[] = {{GTCustom_Style, FSF_BOLD}, {TAG_END, 0}},

	font_italic_tags[] = {{GTCustom_Style, FSF_ITALIC}, {TAG_END, 0}},

	font_display_tags[] = {{GTCustom_ThinBorders, TRUE}, {GTCustom_FrameFlags, AREAFLAG_RECESSED}, {TAG_END, 0}},

	font_cycle_tags[] = {{GTCY_Labels, (IPTR)dummy_labels}, {TAG_END, 0}};

const ObjectDef

	font_objects[] = {

		// Font popup
		{OD_GADGET,
		 FONT_BUTTON_KIND,
		 {0, 0, 0, 1},
		 {4, 6, 28, 6},
		 MSG_FONT_FONT,
		 PLACETEXT_LEFT | TEXTFLAG_ADJUST_TEXT,
		 GAD_FONT_FONT_POPUP,
		 0},

		// Font name
		{OD_GADGET,
		 STRING_KIND,
		 {POS_REL_RIGHT, 0, SIZE_MAX_LESS - 7, 1},
		 {0, 6, -16, 6},
		 0,
		 0,
		 GAD_FONT_FONT,
		 font_name_tags},

		// Font size
		{OD_GADGET,
		 INTEGER_KIND,
		 {0, 0, 6, 1},
		 {0, 2, 8, 6},
		 MSG_FONT_SIZE,
		 POSFLAG_ADJUST_POS_Y | POSFLAG_ALIGN_POS_X,
		 GAD_FONT_SIZE,
		 font_size_tags},

		// Increase size
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_REL_RIGHT, 1, 2, 1},
		 {0, 14, 4, 6},
		 (IPTR) "_+",
		 TEXTFLAG_TEXT_STRING,
		 GAD_FONT_UP,
		 0},

		// Decrease size
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_REL_RIGHT, 1, 2, 1},
		 {0, 14, 4, 6},
		 (IPTR) "_-",
		 TEXTFLAG_TEXT_STRING,
		 GAD_FONT_DOWN,
		 0},

		// Size cycle
		{OD_GADGET, CYCLE_KIND, {POS_REL_RIGHT, 1, 6, 1}, {0, 14, 28, 6}, 0, 0, GAD_FONT_CYCLE, font_cycle_tags},

		// Bold
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY - 5, 0, 2, 1},
		 {-12, 6, 4, 6},
		 MSG_FONT_BOLD,
		 BUTTONFLAG_TOGGLE_SELECT,
		 GAD_FONT_BOLD,
		 font_bold_tags},

		// Italics
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY - 3, 0, 2, 1},
		 {-8, 6, 4, 6},
		 MSG_FONT_ITALIC,
		 BUTTONFLAG_TOGGLE_SELECT,
		 GAD_FONT_ITALIC,
		 font_italic_tags},

		// Underline
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY - 1, 0, 2, 1},
		 {-4, 6, 4, 6},
		 MSG_FONT_ULINE,
		 BUTTONFLAG_TOGGLE_SELECT,
		 GAD_FONT_ULINE,
		 0},

		// Display area
		{OD_GADGET,
		 FRAME_KIND,
		 {0, 2, SIZE_MAXIMUM, SIZE_MAXIMUM},
		 {6, 28, -6, -6},
		 0,
		 0,
		 GAD_FONT_DISPLAY,
		 font_display_tags},

		{OD_END}};

struct Library *DOpusBase = NULL;
#ifdef __amigaos4__
struct DOpusIFace *IDOpus = NULL;
struct Library *LocaleBase = NULL;
#elif 0	 // defined(__amigaos3__) || defined(__MORPHOS__)
struct Library *LayersBase = NULL;
struct Library *AslBase = NULL;
struct Library *DiskfontBase = NULL;
	#ifdef __MORPHOS__
struct Library *LocaleBase = NULL;
	#else
struct LocaleBase *LocaleBase = NULL;
	#endif
struct Library *WorkbenchBase = NULL;
#endif

const MenuData font_menus[] = {
	{NM_TITLE, 0, MSG_MENU_PROJECT, 0},
	{NM_ITEM, MENU_OPEN_FONT, MSG_MENU_OPEN_FONT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('O')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_SAVE_SETTINGS, MSG_MENU_SAVE_SETTINGS, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('V')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_ABOUT, MSG_MENU_ABOUT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('?')},
	{NM_ITEM, MENU_QUIT, MSG_MENU_QUIT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('Q')},

	{NM_END}};
