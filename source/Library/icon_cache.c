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

/*	library/icon_cachce.c

	25 Oct 99	gjp Added changes to support loading V44 icons

	3  nov 99	I think problems with GetCachedDefDiskobject calling
			getCachedDiskObject were dues to icon not being remapped
			and thus with no image. Changed this to remap icon
			and it now seems to work better.

*/

#include "dopuslib.h"
#include <proto/newicon.h>

//#define NewIconBase (data->new_icon_base)

#define TESTGP

struct DiskObject *LIBFUNC L_GetCachedDefDiskObject(REG(d0, long type), REG(a6, struct MyLibrary *libbase))
{
	char *name = 0;
	struct DiskObject *icon = NULL;
	ULONG flags = 0;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = dopuslibbase_global;
#endif

	// No caching?
	if (type & GCDOF_NOCACHE)
	{
		flags = GCDOF_NOCACHE;
		type &= ~GCDOF_NOCACHE;
	}

	// Look at type to get filename
	switch (type)
	{
	case WBDISK:
		name = "env:sys/def_disk";
		break;

	case WBDRAWER:
		name = "env:sys/def_drawer";
		break;

	case WBTOOL:
		name = "env:sys/def_tool";
		break;

	case WBPROJECT:
		name = "env:sys/def_project";
		break;

	case WBGARBAGE:
		name = "env:sys/def_trashcan";
		break;

	case WBKICK:
		name = "env:sys/def_kick";
		break;
	}

	if (IconBase->lib_Version >= 44)
	{
		// using the new calls here results in a failure when image is written! ??
		// icon=(icon=GetIconTags(name,TAG_DONE) or GetIconTags(NULL,ICONGETA_GetDefaultType,type,TAG_DONE)
		// changed 3/11/99 to do rempapping  Seems to work. Maybe old 44.471 library!

		struct LibData *libdata;

		// Get data pointer
		libdata = (struct LibData *)libbase->ml_UserData;

		if (libdata->backfill_screen)
		{
			if (name)
				icon = GetIconTags(
					name, ICONGETA_FailIfUnavailable, TRUE, ICONGETA_Screen, (IPTR)*libdata->backfill_screen, TAG_DONE);

			if (!icon)
				icon = GetIconTags(
					NULL, ICONGETA_GetDefaultType, type, ICONGETA_Screen, (IPTR)*libdata->backfill_screen, TAG_DONE);
		}

#ifdef __AROS__
		if (icon->do_Type != type)
			icon->do_Type = type;
#endif

		return icon;
	}

	else  // <V44 old methods
	{
		// Valid name?
		if (name && (icon = L_GetCachedDiskObject(name, flags, libbase)))
			return icon;

		// Get icon the normal way

		if ((icon = GetDefDiskObject(type)))
		{
			// Clear this so we won't think it's cached
			icon->do_Gadget.SpecialInfo = 0;
		}
	}
	return icon;
}

void LIBFUNC L_FreeCachedDiskObject(REG(a0, struct DiskObject *icon), REG(a6, struct MyLibrary *libbase))
{
#if defined(__amigaos4__) || defined(__AROS__)
	libbase = dopuslibbase_global;
#endif

	// Valid icon?
	if (!icon)
		return;

	// Is this a copy?
	if (L_GetIconType(icon) == ICON_CACHED)
	{
		Image_Data *image;

		// Valid image pointer?
		if ((image = (Image_Data *)((DiskObjectCopy *)icon)->doc_Data))
		{
			Image_Data *find;

			// Lock image list
			L_GetSemaphore(&image_lock, SEMF_EXCLUSIVE, 0);

			// Check image is in list
			for (find = (Image_Data *)image_list.lh_Head; find->node.ln_Succ; find = (Image_Data *)find->node.ln_Succ)
			{
				// Match pointer
				if (find == image)
				{
					// Decrement usage count
					--image->count;

					// Flush image?
					if (image->count == 0)
					{
						// Remove from list
						Remove(&image->node);

						// Free original icon
						if (image->ilbm)
							L_FreeDiskObjectCopy((struct DiskObject *)image->ilbm, libbase);

						// Free entry
						free_image(image);
					}
					break;
				}
			}

			// Unlock image list
			L_FreeSemaphore(&image_lock);
		}
	}

	// Free as an icon copy
	L_FreeDiskObjectCopy(icon, libbase);
}

struct DiskObject *LIBFUNC L_GetCachedDiskObject(REG(a0, char *name),
												 REG(d0, ULONG flags),
												 REG(a6, struct MyLibrary *libbase))
{
	Image_Data *image;
	struct DiskObject *icon = 0, *reticon;
	unsigned long checksum[2];
	struct LibData *data;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = dopuslibbase_global;
#endif

	// Get data pointer
	data = (struct LibData *)libbase->ml_UserData;

	// Load icon
	// gjp If using OS 3.5

	if (IconBase->lib_Version >= 44)
	{
		icon = NULL;

		// Return only real icons here. Setting ICONGETA_FailIfUnavailable=FALSE
		// would make icon.library hand back a filename-pattern deficon (e.g.
		// env:sys/def_ascii) for any file without a real .info, which
		// short-circuits GetProperIcon() and prevents DOpus filetype icons from
		// ever being consulted in icon-view listers (issue #24).
		// Deficons fallback for files without .info still happens via
		// GetProperIcon() -> filetype_identify() -> GetCachedDiskObjectNew().
		if (data->backfill_screen)
			icon = GetIconTags(
				name, ICONGETA_FailIfUnavailable, TRUE, ICONGETA_Screen, (IPTR)*data->backfill_screen, TAG_DONE);

		return icon;
	}

#if defined(__MORPHOS__)
	#define ICONGETA_PNGBitMap (ICONA_Dummy + 256) /* struct BitMap ** */
	{
		struct BitMap *dummy;
		if ((icon = GetIconTags(name, ICONGETA_PNGBitMap, &dummy, TAG_DONE)))
		{
			if (!strcmp(name, "Disk") || !strcmp(name, "env:sys/def_disk"))
				icon->do_Type = WBDISK;

			return icon;
		}
	}
#endif

	// Got NewIcons, and NewIcons is enabled?

	if (NewIconBase && data->NewIconsFlags & ENVNIF_ENABLE)
	{
		struct NewDiskObject *ndo;
		struct NewIconDiskObject *nido;

		// Load as NewIcon
		if (!(ndo = GetNewDiskObject((UBYTE *)name)))
			return 0;

		// Allocate NIDO
		if ((nido = AllocVec(sizeof(struct NewIconDiskObject), MEMF_CLEAR)))
		{
			// Copy DiskObject structure
			CopyMem((char *)ndo->ndo_StdObject, (char *)&nido->nido_DiskObject, sizeof(struct DiskObject));

			// Set NewDiskObject pointer
			nido->nido_NewDiskObject = ndo;

			// Get pointer as normal icon
			icon = (struct DiskObject *)nido;

			// Set SpecialInfo to tell us it's a NewIcon
			nido->nido_DiskObject.do_Gadget.SpecialInfo = (APTR)(icon + 1);

			// Discourage NewIcons? See if size is less than 5x5
			if (!(data->NewIconsFlags & ENVNIF_DISCOURAGE) || (icon->do_Gadget.Width < 5 && icon->do_Gadget.Height < 5))
			{
				// New-style image? Use as is
				if (ndo->ndo_NormalImage || ndo->ndo_SelectedImage)
				{
					return icon;
				}
			}
		}

		// Failed, so can't use NewDiskObject
		else
		{
			FreeNewDiskObject(ndo);
		}
	}

	// Load icon from disk the old way
	if (!icon)
	{
		if (!(icon = GetDiskObject(name)))
			return 0;

		// Clear this so we won't think it's cached
		icon->do_Gadget.SpecialInfo = 0;
	}

	// Caching disabled? Return icon as is
	if (data->flags & LIBDF_NO_CACHING || flags & GCDOF_NOCACHE)
		return icon;

	// Get image checksums
	checksum[0] = L_IconCheckSum(icon, 0);
	checksum[1] = L_IconCheckSum(icon, 1);

	// Lock image list
	L_GetSemaphore(&image_lock, SEMF_EXCLUSIVE, 0);

	// Go through image list
	for (image = (Image_Data *)image_list.lh_Head; image->node.ln_Succ; image = (Image_Data *)image->node.ln_Succ)
	{
		// Cached icon?
		if (image->flags & IMAGEF_ICON)
		{
			// Match image checksums
			if (image->checksum[0] == checksum[0] && image->checksum[1] == checksum[1])
				break;
		}
	}

	// Don't have it in the list?
	if (!image->node.ln_Succ)
	{
		struct Image *im;

		// Get image pointer
		im = (struct Image *)icon->do_Gadget.GadgetRender;

		// Add entry to image list
		if (!(image = new_image(im->Width, im->Height, im->Depth)) ||
			!(image->ilbm = (ILBMHandle *)L_CopyDiskObject(icon, 0, libbase)))
		{
			// Failed
			free_image(image);
			L_FreeSemaphore(&image_lock);

			return icon;
		}

		// Fill out image structure
		image->flags |= IMAGEF_ICON;

		// Save checksums
		image->checksum[0] = checksum[0];
		image->checksum[1] = checksum[1];

		// Add to list
		AddTail(&image_list, &image->node);
	}

	// Get a copy of this icon
	if ((reticon = (struct DiskObject *)L_CopyDiskObject(icon, DOCF_NOIMAGE | DOCF_COPYALL, libbase)))
	{
		// Increment image count
		++image->count;

		// Get image pointers from cached copy
		reticon->do_Gadget.GadgetRender = ((struct DiskObject *)image->ilbm)->do_Gadget.GadgetRender;
		reticon->do_Gadget.SelectRender = ((struct DiskObject *)image->ilbm)->do_Gadget.SelectRender;

		// Set pointer back to image entry
		((DiskObjectCopy *)reticon)->doc_Data = image;

		// Free the original icon
		if (L_GetIconType(icon) == ICON_NEWICON)
		{
			// Free the NewDiskObject part
			FreeNewDiskObject(((struct NewIconDiskObject *)icon)->nido_NewDiskObject);

			// Free our part
			FreeVec(icon);
		}

		// Free icon normally
		else
			FreeDiskObject(icon);

		// Get new icon
		icon = reticon;
	}

	// Unlock image list
	L_FreeSemaphore(&image_lock);

	return icon;
}

struct DiskObject *LIBFUNC L_GetCachedDiskObjectNew(REG(a0, char *name),
													REG(d0, ULONG flags),
													REG(a6, struct MyLibrary *libbase))
{
	struct DiskObject *icon;
	BPTR lock, file;
	D_S(struct FileInfoBlock, fib)
	long type = WBPROJECT, val = 0;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = dopuslibbase_global;
#endif

	// Try for real icon
	if (flags & GCDOFN_REAL_ICON && (icon = L_GetCachedDiskObject(name, 0, libbase)))
		return icon;

	// Lock object
	if ((lock = Lock(name, ACCESS_READ)))
	{
		// Examine file
		Examine(lock, fib);

		// Drawer?
		if (fib->fib_DirEntryType > 0)
			type = WBDRAWER;

		// Otherwise, open file
		else if ((file = OpenFromLock(lock)))
		{
			// Read first longword
			Read(file, (char *)&val, sizeof(long));
#ifdef __AROS__
			val = AROS_BE2LONG(val);
#endif

			// Executable?
#ifdef __AROS__
			if (val == 0x7f454c46)
#else
			if ((val == 0x000003f3) || (val == 0x7f454c46))
#endif
				type = WBTOOL;

			// Close file
			Close(file);
			lock = 0;
		}

		// Unlock lock
		UnLock(lock);
	}

	// Couldn't lock; is it "Disk"?
	else if (stricmp(FilePart(name), "Disk") == 0)
		type = WBDISK;

	// Get default icon
	return L_GetCachedDefDiskObject(type, libbase);
}

// Calculate image checksum for an icon
unsigned long LIBFUNC L_IconCheckSum(REG(a0, struct DiskObject *icon), REG(d0, short which))
{
	unsigned long sum = 0;
	long size, a;
	struct Image *image;

	// Get image pointers
	image = (which) ? (struct Image *)icon->do_Gadget.SelectRender : (struct Image *)icon->do_Gadget.GadgetRender;

	// Valid image?
	if (!image || !image->ImageData)
		return 0;

	// Get size of image data
	size = ((image->Width + 15) >> 4) * image->Height * image->Depth;

	// Calculate image checksum
	for (a = 0; a < size; a++)
		sum += image->ImageData[a] * (a + 1);

	// Multiply by image size
	sum *= image->Width;
	sum *= image->Height;
	sum *= image->Depth;

	return sum;
}

// Remap an icon
BOOL LIBFUNC L_RemapIcon(REG(a0, struct DiskObject *icon), REG(a1, struct Screen *screen), REG(d0, short free_remap))
{
	struct NewIconDiskObject *nido;

	if (!icon)
		return 0;

	// gjp OS 3.5 stuff Remap
	if (IconBase->lib_Version >= 44)
	{
		LayoutIcon(icon, free_remap ? NULL : screen, TAG_DONE);
		return (BOOL)(free_remap ? 0 : TRUE);
	}

	// Not a NewIcon?
	if (L_GetIconType(icon) != ICON_NEWICON || !NewIconBase)
	{
		return 0;
	}

	// Get NIDO pointer
	nido = (struct NewIconDiskObject *)icon;

	// Remap?
	if (!free_remap)
	{
		struct Image *image;
		BOOL res = 0;

		// Fail if no screen
		if (!screen)
			return 0;

		// Already remapped?
		if (nido->nido_Flags & NIDOF_REMAPPED)
		{
#ifdef DEBUG_ICON
			D(bug("error! icon remap twice\n"));
#endif
			return 0;
		}

		// Get old image pointer and size
		icon->do_Gadget.GadgetRender = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.GadgetRender;
		icon->do_Gadget.Width = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.Width;
		icon->do_Gadget.Height = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.Height;

		// Remap image
		if (nido->nido_NewDiskObject->ndo_NormalImage &&
			(image = RemapChunkyImage(nido->nido_NewDiskObject->ndo_NormalImage, screen)))
		{
			// Set new image pointer
			icon->do_Gadget.GadgetRender = (APTR)image;

			// Get new size
			icon->do_Gadget.Width = image->Width;
			icon->do_Gadget.Height = image->Height + 1;
			res = TRUE;
		}

		// Get old image pointer
		icon->do_Gadget.SelectRender = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.SelectRender;
		icon->do_Gadget.Flags = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.Flags;

		// Remap image
		if (nido->nido_NewDiskObject->ndo_SelectedImage &&
			(image = RemapChunkyImage(nido->nido_NewDiskObject->ndo_SelectedImage, screen)))
		{
			// Set new image pointer
			icon->do_Gadget.SelectRender = (APTR)image;
			icon->do_Gadget.Flags &= ~GFLG_GADGHIGHBITS;
			icon->do_Gadget.Flags |= GFLG_GADGHIMAGE;
			res = TRUE;
		}

		// Set remapped flag
		nido->nido_Flags |= NIDOF_REMAPPED;
		return res;
	}

	// Not remapped?
	if (!(nido->nido_Flags & NIDOF_REMAPPED))
		return 0;

	// Get original size
	icon->do_Gadget.Width = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.Width;
	icon->do_Gadget.Height = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.Height;

	// Remapped image?
	if (icon->do_Gadget.GadgetRender != nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.GadgetRender)
	{
		// Free it
		FreeRemappedImage(icon->do_Gadget.GadgetRender, screen);

		// Get old image pointer
		icon->do_Gadget.GadgetRender = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.GadgetRender;
	}

	// Remapped image?
	if (icon->do_Gadget.SelectRender != nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.SelectRender)
	{
		// Free it
		FreeRemappedImage(icon->do_Gadget.SelectRender, screen);

		// Get old image pointer
		icon->do_Gadget.SelectRender = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.SelectRender;
	}

	// Restore flags
	icon->do_Gadget.Flags = nido->nido_NewDiskObject->ndo_StdObject->do_Gadget.Flags;

	// Clear remapped flag
	nido->nido_Flags &= ~NIDOF_REMAPPED;

	return 0;
}

// Get pointer to original icon from a NewIcon
struct DiskObject *LIBFUNC L_GetOriginalIcon(REG(a0, struct DiskObject *icon))
{
	struct NewIconDiskObject *nido;

	// Not a NewIcon?
	if (!icon || L_GetIconType(icon) != ICON_NEWICON || !NewIconBase)
		return icon;

	// Get NIDO pointer
	nido = (struct NewIconDiskObject *)icon;

	// Return original icon
	return nido->nido_NewDiskObject->ndo_StdObject;
}

// Set NewIcons flags
void LIBFUNC L_SetNewIconsFlags(REG(d0, ULONG flags), REG(d1, short precision), REG(a6, struct MyLibrary *libbase))
{
	struct LibData *data;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = dopuslibbase_global;
#endif

	// Get data pointer
	data = (struct LibData *)libbase->ml_UserData;

	// Set flags
	data->NewIconsFlags = flags;
	data->NewIconsPrecision = precision;

	// Got NewIcons?
	if (NewIconBase)
	{
		// Set precision
		((struct NewIconBase *)NewIconBase)->nib_Precision = precision;

		// Set flags
		if (((struct Library *)NewIconBase)->lib_Version >= 38)
		{
			if (flags & ENVNIF_DITHERING)
				((struct NewIconBase *)NewIconBase)->nib_Flags |= NIFLG_DO_DITHER;
			else
				((struct NewIconBase *)NewIconBase)->nib_Flags &= ~NIFLG_DO_DITHER;
		}
		if (((struct Library *)NewIconBase)->lib_Version >= 39)
		{
			if (flags & ENVNIF_RTG)
				((struct NewIconBase *)NewIconBase)->nib_Flags |= NIFLG_RTGMODE;
			else
				((struct NewIconBase *)NewIconBase)->nib_Flags &= ~NIFLG_RTGMODE;
		}
	}
}
