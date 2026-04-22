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

#if defined(__MORPHOS__) || defined(__AROS__)
	#include <intuition/extensions.h>
#endif

#include "dopuslib.h"
#include "boopsi.h"

// Initialise a BOOPSI class
Class *init_class(struct LibData *data, char *name, char *super, unsigned long (*dispatcher)(), long data_size)
{
	Class *cl;
	BoopsiLibs *libs;

	// Allocate library data
	if (!(libs = AllocVec(sizeof(BoopsiLibs), MEMF_CLEAR)))
		return 0;

	// Fill out library pointers
	/*libs->IntuitionBase=(struct Library *)IntuitionBase;
	libs->GfxBase=(struct GfxBase *)GfxBase;
	libs->UtilityBase=UtilityBase;
	libs->LayersBase=LayersBase;*/
	libs->data = data;

	// Create class
	if ((cl = MakeClass(name, super, 0, data_size, 0)))
	{
// Initialise
#if defined(__MORPHOS__)
		cl->cl_Dispatcher.h_Entry = (HOOKFUNC)HookEntry;
		cl->cl_Dispatcher.h_SubEntry = (HOOKFUNC)dispatcher;
#else
		cl->cl_Dispatcher.h_Entry = dispatcher;
#endif
		cl->cl_Dispatcher.h_Data = libs;

		// Make it available
		AddClass(cl);
	}
	else
		FreeVec(libs);

	return cl;
}

// Free a class
void class_free(Class *cl)
{
	APTR data;

	// Valid class?
	if (cl)
	{
		// Store data pointer
		data = cl->cl_Dispatcher.h_Data;

		// Try to free class
		if (FreeClass(cl))
		{
			// Free data
			FreeVec(data);
		}
	}
}

// Add a set of scroll bars to a window
struct Gadget *LIBFUNC L_AddScrollBars(REG(a0, struct Window *window),
									   REG(a1, struct List *list),
									   REG(a2, struct DrawInfo *draw_info),
									   REG(d0, short flags))
{
	struct Image *image[4] = {0, 0, 0, 0};
	struct Gadget *gadget = 0;
	short a;
	static struct TagItem map_tags[] = {{PGA_Top, ICSPECIAL_CODE}, {TAG_END}};

	// Create arrow images
	for (a = 0; a < 4; a++)
	{
		if (((a % 2) == 1 && flags & SCROLL_VERT) || ((a % 2) == 0 && flags & SCROLL_HORIZ))
		{
			if (!(image[a] = (struct Image *)NewObject(
					  0,
					  "sysiclass",
					  SYSIA_Size,
					  (window->WScreen->Flags & SCREENHIRES) ? SYSISIZE_MEDRES : SYSISIZE_LOWRES,
					  SYSIA_Which,
					  LEFTIMAGE + a,
					  SYSIA_DrawInfo,
					  (ULONG)draw_info,
					  TAG_END)))
				return 0;
			DoMethod((Object *)image[a], OM_ADDTAIL, list);
		}
	}

	// Want vertical scroller?
	if (flags & SCROLL_VERT)
	{
		// Create vertical slider
		if (!(gadget = (struct Gadget *)NewObject(
				  0,
				  "propgclass",
				  GA_ID,
				  GAD_VERT_SCROLLER,
				  GA_RelRight,
				  -(window->BorderRight - 5),
				  GA_Top,
				  window->BorderTop + 2,
				  GA_Width,
				  window->BorderRight - 8,
				  GA_RelHeight,
				  -(window->BorderTop + window->BorderBottom + image[1]->Height + image[3]->Height + 4),
				  GA_Immediate,
				  TRUE,
				  GA_FollowMouse,
				  TRUE,
				  GA_RelVerify,
				  TRUE,
				  GA_RightBorder,
				  TRUE,
				  PGA_Freedom,
				  FREEVERT,
				  PGA_NewLook,
				  TRUE,
				  PGA_Borderless,
				  TRUE,
				  PGA_Visible,
				  1,
				  PGA_Total,
				  1,
				  (flags & SCROLL_NOIDCMP) ? TAG_IGNORE : ICA_TARGET,
				  ICTARGET_IDCMP,
				  ICA_MAP,
				  map_tags,
				  TAG_END)))
			return 0;
		DoMethod((Object *)gadget, OM_ADDTAIL, list);

		// Up arrow
		if (!(gadget = (struct Gadget *)NewObject(0,
												  "buttongclass",
												  GA_ID,
												  GAD_VERT_ARROW_UP,
												  GA_Image,
												  image[1],
												  GA_Previous,
												  gadget,
												  GA_RelRight,
												  -(window->BorderRight - 1),
												  GA_RelBottom,
												  -(window->BorderBottom + image[1]->Height + image[3]->Height - 1),
												  GA_Width,
												  window->BorderRight,
												  GA_Height,
												  image[1]->Height,
												  GA_Immediate,
												  TRUE,
												  GA_RelVerify,
												  TRUE,
												  GA_RightBorder,
												  TRUE,
												  (flags & SCROLL_NOIDCMP) ? TAG_IGNORE : ICA_TARGET,
												  ICTARGET_IDCMP,
												  TAG_END)))
			return 0;
		DoMethod((Object *)gadget, OM_ADDTAIL, list);

		// Down arrow
		if (!(gadget = (struct Gadget *)NewObject(0,
												  "buttongclass",
												  GA_ID,
												  GAD_VERT_ARROW_DOWN,
												  GA_Image,
												  image[3],
												  GA_Previous,
												  gadget,
												  GA_RelRight,
												  -(window->BorderRight - 1),
												  GA_RelBottom,
												  -(window->BorderBottom + image[3]->Height - 1),
												  GA_Width,
												  window->BorderRight,
												  GA_Height,
												  image[0]->Height,
												  GA_Immediate,
												  TRUE,
												  GA_RelVerify,
												  TRUE,
												  GA_RightBorder,
												  TRUE,
												  (flags & SCROLL_NOIDCMP) ? TAG_IGNORE : ICA_TARGET,
												  ICTARGET_IDCMP,
												  TAG_END)))
			return 0;
		DoMethod((Object *)gadget, OM_ADDTAIL, list);
	}

	// Want horizontal scroller?
	if (flags & SCROLL_HORIZ)
	{
		// Create horizontal slider
		if (!(gadget = (struct Gadget *)NewObject(
				  0,
				  "propgclass",
				  GA_ID,
				  GAD_HORIZ_SCROLLER,
				  (gadget) ? GA_Previous : TAG_IGNORE,
				  gadget,
				  GA_Left,
				  window->BorderLeft,
				  GA_RelBottom,
				  -(window->BorderBottom - 3),
				  GA_RelWidth,
				  -(window->BorderLeft + window->BorderRight + image[0]->Width + image[2]->Width + 2),
				  GA_Height,
				  window->BorderBottom - 4,
				  GA_Immediate,
				  TRUE,
				  GA_FollowMouse,
				  TRUE,
				  GA_RelVerify,
				  TRUE,
				  GA_BottomBorder,
				  TRUE,
				  PGA_Freedom,
				  FREEHORIZ,
				  PGA_NewLook,
				  TRUE,
				  PGA_Borderless,
				  TRUE,
				  PGA_Visible,
				  1,
				  PGA_Total,
				  1,
				  (flags & SCROLL_NOIDCMP) ? TAG_IGNORE : ICA_TARGET,
				  ICTARGET_IDCMP,
				  ICA_MAP,
				  map_tags,
				  TAG_END)))
			return 0;
		DoMethod((Object *)gadget, OM_ADDTAIL, list);

		// Left arrow
		if (!(gadget = (struct Gadget *)NewObject(0,
												  "buttongclass",
												  GA_ID,
												  GAD_HORIZ_ARROW_LEFT,
												  GA_Image,
												  image[0],
												  GA_Previous,
												  gadget,
												  GA_RelRight,
												  -(window->BorderRight + image[0]->Width + image[2]->Width - 1),
												  GA_RelBottom,
												  -(window->BorderBottom - 1),
												  GA_Width,
												  image[0]->Width,
												  GA_Height,
												  window->BorderBottom,
												  GA_Immediate,
												  TRUE,
												  GA_RelVerify,
												  TRUE,
												  GA_BottomBorder,
												  TRUE,
												  (flags & SCROLL_NOIDCMP) ? TAG_IGNORE : ICA_TARGET,
												  ICTARGET_IDCMP,
												  TAG_END)))
			return 0;
		DoMethod((Object *)gadget, OM_ADDTAIL, list);

		// Right arrow
		if (!(gadget = (struct Gadget *)NewObject(0,
												  "buttongclass",
												  GA_ID,
												  GAD_HORIZ_ARROW_RIGHT,
												  GA_Image,
												  image[2],
												  GA_Previous,
												  gadget,
												  GA_RelRight,
												  -(window->BorderRight + image[0]->Width - 1),
												  GA_RelBottom,
												  -(window->BorderBottom - 1),
												  GA_Width,
												  image[2]->Width,
												  GA_Height,
												  window->BorderBottom,
												  GA_Immediate,
												  TRUE,
												  GA_RelVerify,
												  TRUE,
												  GA_BottomBorder,
												  TRUE,
												  (flags & SCROLL_NOIDCMP) ? TAG_IGNORE : ICA_TARGET,
												  ICTARGET_IDCMP,
												  TAG_END)))
			return 0;
		DoMethod((Object *)gadget, OM_ADDTAIL, list);
	}

	return gadget;
}

// Find a gadget in a BOOPSI list
struct Gadget *LIBFUNC L_FindBOOPSIGadget(REG(a0, struct List *list), REG(d0, UWORD id))
{
	APTR object_ptr;
	Object *object;
	Class *class;

	// Go through list
	object_ptr = list->lh_Head;
	while ((object = NextObject((APTR)&object_ptr)))
	{
		class = OCLASS(object);
		if (class->cl_Super->cl_ID && strcmp(class->cl_Super->cl_ID, "gadgetclass") == 0 &&
			((struct Gadget *)object)->GadgetID == id)
			return (struct Gadget *)object;
	}

	return 0;
}

// Free a boopsi list
void LIBFUNC L_BOOPSIFree(REG(a0, struct List *list))
{
	APTR object_ptr;
	Object *object;

	object_ptr = list->lh_Head;
	while ((object = NextObject((APTR)&object_ptr)))
		DisposeObject(object);
	NewList(list);
}

// Create a gadget in the window title bar
struct Gadget *LIBFUNC L_CreateTitleGadget(REG(a0, struct Screen *screen),
										   REG(a1, struct List *list),
										   REG(d0, BOOL cover_zoom),
										   REG(d1, short offset),
										   REG(d2, short type),
										   REG(d3, unsigned short id),
										   REG(a6, struct MyLibrary *libbase))
{
	short width, left, height = 0, top = 0;
	struct Gadget *gadget;
	struct Image *image = 0;
	struct Screen *pubscr = 0;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// No screen supplied?
	if (!screen && !(pubscr = LockPubScreen(0)))
		return 0;
	else if (pubscr)
		screen = pubscr;

	// Gauge?
	if (type == IM_GAUGE)
	{
		// Get right border size
		width = screen->WBorRight;

		// Get position of gadget
		left = 0;
		top = screen->WBorTop + screen->RastPort.Font->tf_YSize + 1;

		// Get relative height
		height = -top - screen->WBorBottom;
	}

	// Otherwise, get size from depth gadget
	else
	{
		struct DrawInfo *drinfo;
		struct Image *image;
		short depth_width = 0, zoom_width = 0;

		// Get DrawInfo
		drinfo = GetScreenDrawInfo(screen);

		// Get depth gadget image
		if ((image =
				 (struct Image *)NewObject(0, "sysiclass", SYSIA_DrawInfo, drinfo, SYSIA_Which, DEPTHIMAGE, TAG_END)))
		{
			// Get width from image
			depth_width = image->Width;

			// Free image
			DisposeObject((Object *)image);
		}

		// Need zoom size?
		if (!cover_zoom)
		{
			// Get zoom gadget image
			if ((image = (struct Image *)NewObject(
					 0, "sysiclass", SYSIA_DrawInfo, drinfo, SYSIA_Which, ZOOMIMAGE, TAG_END)))
			{
				// Get width from image
				zoom_width = image->Width;

				// Free image
				DisposeObject((Object *)image);
			}
		}

		// Free DrawInfo
		FreeScreenDrawInfo(screen, drinfo);

		// Need either size still?
		if (!depth_width || (!cover_zoom && !zoom_width))
		{
			struct Gadget *depth;

			// Find screen depth gadget
			if (!(depth = L_FindGadgetType(screen->FirstGadget, GTYP_SDEPTH)))
			{
				if (pubscr)
					UnlockPubScreen(0, pubscr);
				return 0;
			}

			// Use this as the width
			depth_width = depth->Width;
			zoom_width = depth->Width;
		}

		// Calculate position of gadget
		left = 2 - depth_width;
		if (!cover_zoom)
			left -= zoom_width - 1;

		// Additional position
		left += offset;

		// Use depth width for gadget
		width = depth_width;
	}

	// Create image (except for gauge)
	switch (type)
	{
	case IM_GAUGE:
		break;

#if defined(__MORPHOS__) || defined(__amigaos4__) || defined(__AROS__)
	#ifdef __amigaos4__
		#define LOCKIMAGE PADLOCKIMAGE
	#endif
	case IM_ICONIFY:
	case IM_LOCK: {
		APTR drawinfo = GetScreenDrawInfo(screen);

		image = (struct Image *)NewObject(0,
										  "sysiclass",
										  SYSIA_Which,
										  type == IM_ICONIFY ? ICONIFYIMAGE : LOCKIMAGE,
										  SYSIA_DrawInfo,
										  drawinfo,
										  TAG_END);

		FreeScreenDrawInfo(screen, drawinfo);
	}
	break;
#endif

	default:
		image = (struct Image *)NewObject(0,
										  "dopusiclass",
										  DIA_Type,
										  type,
										  IA_Width,
										  width,
										  IA_Height,
										  screen->WBorTop + screen->Font->ta_YSize + 1,
										  TAG_END);
	}

	if (type == IM_GAUGE || image != NULL)
	{
		// Create gadget
		if ((gadget = (struct Gadget *)NewObject(0,
												 (type == IM_GAUGE) ? "dopusgaugeclass" : "dopusbuttongclass",
												 GA_ID,
												 id,
												 (type == IM_GAUGE) ? GA_Left : GA_RelRight,
												 (type == IM_GAUGE) ? left : left - image->Width - 1,
												 GA_Top,
												 top,
												 GA_Width,
												 (type == IM_GAUGE) ? width : image->Width,
												 (type == IM_GAUGE) ? GA_RelHeight : GA_Height,
												 (type == IM_GAUGE) ? height : image->Height,
												 GA_RelVerify,
												 TRUE,
												 (type == IM_GAUGE) ? GA_LeftBorder : GA_TopBorder,
												 TRUE,
												 (image) ? GA_Image : TAG_IGNORE,
												 image,
												 TAG_END)))
		{
			// Add to boopsi list
			if (image)
				DoMethod((Object *)image, OM_ADDTAIL, list);
			DoMethod((Object *)gadget, OM_ADDTAIL, list);

			if (pubscr)
				UnlockPubScreen(0, pubscr);
			return gadget;
		}

		// Failed
		DisposeObject((Object *)image);
	}

	if (pubscr)
		UnlockPubScreen(0, pubscr);
	return 0;
}

// Look for a gadget of a certain type
struct Gadget *LIBFUNC L_FindGadgetType(REG(a0, struct Gadget *gadget), REG(d0, UWORD type))
{
	// Walk gadget list
	while (gadget)
	{
		// Is gadget the right type?
		if ((gadget->GadgetType & GTYP_SYSTYPEMASK) == type)
			return gadget;

		// Get next gadget
		gadget = gadget->NextGadget;
	}

	// Not found
	return 0;
}

// Fix title gadgets
void LIBFUNC L_FixTitleGadgets(REG(a0, struct Window *window))
{
	struct Gadget *gadget, *next;

	// Go through gadgets
	for (gadget = window->FirstGadget; gadget; gadget = next)
	{
		// Save next
		next = gadget->NextGadget;

		// Is gadget in the title bar?
		if (gadget->Activation & GACT_TOPBORDER)
		{
			// Not a system gadget?
			if (!(gadget->GadgetType & GTYP_SYSGADGET))
			{
				// Remove gadget from the list
				RemoveGList(window, gadget, 1);

				// Add to beginning of list
				AddGList(window, gadget, 0, 1, 0);
			}
		}
	}
}
