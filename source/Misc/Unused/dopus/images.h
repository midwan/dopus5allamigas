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

#ifndef _DOPUS_IMAGES
#define _DOPUS_IMAGES

/*****************************************************************************

 Images

 *****************************************************************************/

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// RenderImage() tags
#define IM_Width TAG_USER + 0		   // Width of image
#define IM_Height TAG_USER + 1		   // Height of image
#define IM_State TAG_USER + 2		   // 1 = selected, 0 = normal (default)
#define IM_Rectangle TAG_USER + 3	   // Rectangle to center within
#define IM_Mask TAG_USER + 4		   // 1 = mask image
#define IM_Depth TAG_USER + 5		   // Depth of image
#define IM_ClipBoundary TAG_USER + 6   // Clip boundary size
#define IM_Erase TAG_USER + 7		   // Erase background
#define IM_NoDrawInvalid TAG_USER + 8  // Don't draw if image is invalid
#define IM_NoIconRemap TAG_USER + 9	   // Don't remap icons

// Images remapping
typedef struct
{
	struct Screen *ir_Screen;
	unsigned short *ir_PenArray;
	short ir_PenCount;
	unsigned long ir_Flags;
} ImageRemap;

#define IRF_REMAP_COL0 (1 << 0)		  // Remap colour 0
#define IRF_PRECISION_EXACT (1 << 1)  // Remap precision
#define IRF_PRECISION_ICON (1 << 2)
#define IRF_PRECISION_GUI (1 << 3)

// Open an image in memory (for remapping)
typedef struct
{
	UWORD *oi_ImageData;
	ULONG *oi_Palette;
	short oi_Width;
	short oi_Height;
	short oi_Depth;
} OpenImageInfo;

void CloseImage(APTR image);
APTR CopyImage(APTR image);
void FreeImageRemap(ImageRemap *);
void FreeRemapImage(APTR, ImageRemap *);
void GetImageAttrs(APTR image, struct TagItem *tags);
ULONG *GetImagePalette(APTR image);
APTR OpenImage(char *name, OpenImageInfo *);
BOOL RemapImage(APTR, struct Screen *, ImageRemap *);
short RenderImage(struct RastPort *rp, APTR image, unsigned short left, unsigned short top, struct TagItem *tags);

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
