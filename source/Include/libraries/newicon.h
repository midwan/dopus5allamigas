#ifndef LIBRARIES_NEWICON_H
#define LIBRARIES_NEWICON_H

#ifndef EXEC_LIBRARIES_H
	#include <exec/libraries.h>
#endif

#ifndef WORKBENCH_WORKBENCH_H
	#include "workbench/workbench.h"
#endif

#ifndef __amigaos3__
	#pragma pack(2)
#endif

/* Programs should NOT change anything in the librarybase!
 * Let the user do it through the preferences
 */

struct NewIconBase
{
	struct Library nib_Lib;
	APTR nib_SegList;
	LONG nib_Precision; /* precision when remapping images via */
						/* RemapChunkyImage(). Programs are not supposed */
						/* to modify this, it is a value chosen by the user */
						/* in the preferences. */
	LONG nib_Flags;		/* library base flags - see NIFLG_#? defs below */
	LONG nib_DPMode;	/* Depth Promotion mode value. (V39) */
	LONG nib_FOMode;	/* Font Online mode (V40) */
	LONG nib_Reserved;
};

/* This mask can be used to isolate the user preferences part of the flags */
#define NIFLG_PUBMASK 0x0000FFFF

/*** Library base flags ***/

#define NIFLG_DO_DITHER (1 << 0) /* use dithering when exact pen not available */

#define NIFLG_RESERVED                             \
	(1 << 1) /* Keep your hands away from that one \
				(V39) */

#define NIFLG_RTGMODE                                     \
	(1 << 2) /* Use FastRAM when possible instead of CHIP \
				Needs Cgfx or something similar that      \
				properly patches graphics.library to      \
				allow graphic/blitter work from FastRAM.  \
				(V39) */

#define NIFLG_NOBORDER                                 \
	(1 << 3) /* Won't display any border around icons. \
				(V39) */

#define NIFLG_TRANSPARENT                         \
	(1 << 4) /*  Icons will become transparent.   \
				 Requires NoBorder to be enabled. \
				 (V40) */

#define NIFLG_TRANSDRAG   (1<<5> /* Icons will become transparent & 
                                    borderless while dragging.
                                     (V40) */

/*** System flags (0xFFFF0000) - These are READ-ONLY! ***/

/* Set by the NewIcons exe if it's disabled through Exchange */

#define NIFLG_IS_DISABLED              \
	(1 << 16) /* Commodity is disabled \
			   **READ-ONLY** (V40) */

/* These flags are set by the library depending on the features it can
   support. */

#define NIFLG_LIBRTG                            \
	(1 << 17) /* This library supports RTG Mode \
			   **READ-ONLY** (V40) */

#define NIFLG_LIBDIT                             \
	(1 << 18) /* This library supports dithering \
			   **READ-ONLY** (V40) */

/* Values for nib_DPMode (V39) */

#define NIDPM_NEVER 0  /* Do not depth promote old icons */
#define NIDPM_WBONLY 1 /* Only promote icons opened by Workbench */
#define NIDPM_ALL 2	   /* Depth promote all old icons opened */

/* Values for nib_FOMode (V40) */

#define NIFOM_NONE 0	/* Normal */
#define NIFOM_OUTLINE 1 /* Draw an outline border */
#define NIFOM_SHADOW 2	/* Only cast a shadow */

/*
 * This structure contains an image in a format unusual to the Amiga: chunky
 * pixel. We use this format because it is faster to remap to a given palette.
 */
struct ChunkyImage
{
	UWORD Width;	   /* width of the image. Must be <= 93 */
	UWORD Height;	   /* height of the image. Must be <= 93 */
	UWORD NumColors;   /* number of pens defined in the palette. Must be <= 256 */
	UWORD Flags;	   /* see defines below */
	UBYTE *Palette;	   /* array of RGB components of the pens (one byte per gun) */
	UBYTE *ChunkyData; /* Width*Height bytes storing the colors of the pixels */
};

#define CIF_COLOR_0_TRANSP (1 << 0) /* color 0 is transparent, not the one */
									/* stored in palette */

#define CIF_DEFAULT (1 << 1) /* when this bit is set, the image was */
							 /* obtained via the DEFAULTIMAGE ToolType, */
							 /* therefore it is not actually part of the */
							 /* icon and will not be stored in the */
							 /* ToolTypes on a PutNewDiskObject(). */

struct NewDiskObject
{
	struct DiskObject *ndo_StdObject;	   /* The old-style icon */
	struct ChunkyImage *ndo_NormalImage;   /* May be NULL, meaning no new style icon */
	struct ChunkyImage *ndo_SelectedImage; /* May be NULL, meaning no secondary image */
};

/*** The prefs file format (V40)

	 Prefs file must be named NewIcons.prefs and is located in the
	 usual ENVARC: (and ENV:) path.
 ***/

struct NewIconsPrefs
{
	UWORD version;	/* Prefs version, currently 0x0002 */
	ULONG flags;	/* Flags (see NIFLG_#? definitions) */
	LONG precision; /* Precision setting (-1 = best, 16=worse) */
	ULONG dpmode;	/* Depth Promotion mode (see NIDPM_#? definitions) */
	ULONG fomode;	/* Font Outline mode (see NIFOM_#? definitions) */
};

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif /* LIBRARIES_NEWICON_H */
