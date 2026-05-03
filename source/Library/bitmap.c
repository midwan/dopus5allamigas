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

#include "dopuslib.h"

struct BitMap *LIBFUNC L_NewBitMap(REG(d0, ULONG sizex),
								   REG(d1, ULONG sizey),
								   REG(d2, ULONG depth),
								   REG(d3, ULONG flags),
								   REG(a0, struct BitMap *friend))
{
	struct BitMap *bm;
	short plane;

	// Under 39, allocate directly
	if (((struct Library *)GfxBase)->lib_Version >= 39)
	{
		// Friend bitmap, minplanes not set?
		if (friend && !(flags & BMF_MINPLANES))
		{
			// Non-standard friend bitmap?
			if (!(GetBitMapAttr(friend, BMA_FLAGS) & BMF_STANDARD))
			{
				// Set min-planes in flags
				flags |= BMF_MINPLANES;
			}
		}

		return AllocBitMap(sizex, sizey, depth, flags, friend);
	}

	// Round up width, check depth
	sizex = (((sizex + 15) >> 3) & 0xfffe) << 3;
	if (depth < 1)
		depth = 1;
	else if (depth > 8)
		depth = 8;

	// Allocate and initialise bitmap
	if (!(bm = AllocVec(sizeof(struct BitMap), MEMF_CLEAR)))
		return 0;
	InitBitMap(bm, depth, sizex, sizey);

	// Allocate planes
	for (plane = 0; plane < depth; plane++)
	{
		// Allocate plane
		if (!(bm->Planes[plane] = AllocRaster(sizex, sizey)))
			break;

		// Clear?
		if (flags & BMF_CLEAR)
			BltClear(bm->Planes[plane], RASSIZE(sizex, sizey), 0);
	}

	// Failed?
	if (plane < depth)
	{
		// Free what we got
		while ((--plane) >= 0)
			FreeRaster(bm->Planes[plane], sizex, sizey);
		FreeVec(bm);
		return 0;
	}

	return bm;
}

void LIBFUNC L_DisposeBitMap(REG(a0, struct BitMap *bm))
{
	short plane;

	// Invalid pointer?
	if (!bm)
		return;

	// Under 39, free directly
	if (((struct Library *)GfxBase)->lib_Version >= 39)
	{
		FreeBitMap(bm);
		return;
	}

	// Free planes
	for (plane = 0; plane < bm->Depth; plane++)
		if (bm->Planes[plane])
			FreeRaster(bm->Planes[plane], bm->BytesPerRow << 3, bm->Rows);

	// Free bitmap
	FreeVec(bm);
}
