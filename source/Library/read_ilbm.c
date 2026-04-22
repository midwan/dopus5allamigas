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

// Read an ILBM image
ILBMHandle *LIBFUNC L_ReadILBM(REG(a0, char *name), REG(d0, ULONG flags))
{
	struct _IFFHandle *iff;
	ILBMHandle *ilbm;
	unsigned char *body = 0;
	unsigned long body_size = 0;
	ULONG chunk, chunksize;
	BOOL fail = 0;

	// Allocate new ILBM handle
	if (!(ilbm = AllocVec(sizeof(ILBMHandle), MEMF_CLEAR)) ||
		!(ilbm->memory = L_NewMemHandle(1024, sizeof(ANIMHandle) + sizeof(ANIMFrame), MEMF_CLEAR)))
	{
		FreeVec(ilbm);
		return 0;
	}

	// Try to open file
	if (!(iff = L_IFFOpen(name, IFF_READ, ID_ILBM)))
	{
		// Not ILBM; is it ANIM?
		if ((iff = L_IFFOpen(name, IFF_READ, ID_ANIM)))
		{
			// Allocate anim data
			if (!(flags & ILBMF_NO_ANIM))
			{
				if ((ilbm->anim = L_AllocMemH(ilbm->memory, sizeof(ANIMHandle))))
					NewList((struct List *)&ilbm->anim->frames);
				ilbm->flags |= ILBMF_IS_ANIM;
			}
		}

		// Not ILBM or ANIM
		else
		{
			L_FreeILBM(ilbm);
			return 0;
		}
	}

	// Parse IFF file
	while (!fail && (chunk = L_IFFNextChunk(iff, 0)))
	{
		// Get chunk size
		chunksize = L_IFFChunkSize(iff);

		// Look at chunk type
		switch (chunk)
		{
		// Bitmap header
		case ID_BMHD:

			// Read header
			if ((L_IFFReadChunkBytes(iff, &ilbm->header, sizeof(BitMapHeader))) == sizeof(BitMapHeader))
			{
#ifdef __AROS__
				ilbm->header.w = AROS_BE2WORD(ilbm->header.w);
				ilbm->header.h = AROS_BE2WORD(ilbm->header.h);
				ilbm->header.x = AROS_BE2WORD(ilbm->header.x);
				ilbm->header.y = AROS_BE2WORD(ilbm->header.y);
				ilbm->header.transparentColor = AROS_BE2WORD(ilbm->header.transparentColor);
				ilbm->header.pageWidth = AROS_BE2WORD(ilbm->header.pageWidth);
				ilbm->header.pageHeight = AROS_BE2WORD(ilbm->header.pageHeight);
#endif

				ilbm->flags |= ILBMF_GOT_HEADER;
				if ((flags & ILBMF_NO_24BIT) && ilbm->header.nPlanes > 8)
				{
					ilbm->flags &= ~ILBMF_GOT_HEADER;
					fail = 1;
				}
			}
			break;

		// Mode ID
		case ID_CAMG:

			// Read mode
			L_IFFReadChunkBytes(iff, &ilbm->mode_id, sizeof(ULONG));
#ifdef __AROS__
			ilbm->mode_id = AROS_BE2LONG(ilbm->mode_id);
#endif
			ilbm->flags |= ILBMF_GOT_MODE;
			break;

		// DPAnimChunk
		case ID_DPAN:

			// Do we have animation data?
			if (ilbm->flags & ILBMF_IS_ANIM)
			{
				// Read header
				if ((L_IFFReadChunkBytes(iff, &ilbm->anim->header, sizeof(DPAnimChunk)) != sizeof(DPAnimChunk)))
					ilbm->flags &= ~ILBMF_IS_ANIM;
#ifdef __AROS__
				else
				{
					ilbm->anim->header.version = AROS_BE2WORD(ilbm->anim->header.version);
					ilbm->anim->header.nframes = AROS_BE2WORD(ilbm->anim->header.nframes);
					ilbm->anim->header.flags = AROS_BE2WORD(ilbm->anim->header.flags);
				}
#endif
			}
			break;

		// Anim frame
		case ID_ANHD:

			// Do we have animation data?
			if (ilbm->flags & ILBMF_IS_ANIM)
			{
				// If we haven't had a body yet, ignore this chunk (DPaintV animbrush)
				if (!(ilbm->flags & ILBMF_GOT_BODY))
					break;

				// See if we're allowed this one
				if (!(flags & ILBMF_ANIM_ONE_ONLY) || ilbm->anim->frame_count < 1)
				{
					// Create a new frame
					if ((ilbm->anim->current = L_AllocMemH(ilbm->memory, sizeof(ANIMFrame))))
					{
						// Read frame header
						L_IFFReadChunkBytes(iff, &ilbm->anim->current->header, sizeof(AnimHdr));

#ifdef __AROS__
						ilbm->anim->current->header.w = AROS_BE2WORD(ilbm->anim->current->header.w);
						ilbm->anim->current->header.h = AROS_BE2WORD(ilbm->anim->current->header.h);
						ilbm->anim->current->header.x = AROS_BE2WORD(ilbm->anim->current->header.x);
						ilbm->anim->current->header.y = AROS_BE2WORD(ilbm->anim->current->header.y);
						ilbm->anim->current->header.abstime = AROS_BE2LONG(ilbm->anim->current->header.abstime);
						ilbm->anim->current->header.reltime = AROS_BE2LONG(ilbm->anim->current->header.reltime);
						ilbm->anim->current->header.bits = AROS_BE2LONG(ilbm->anim->current->header.bits);
#endif

						// Add to list
						AddTail((struct List *)&ilbm->anim->frames, (struct Node *)ilbm->anim->current);
					}
				}
			}
			break;

		// Delta
		case ID_DLTA:

			// Do we have animation data and a valid frame?
			if (ilbm->flags & ILBMF_IS_ANIM && ilbm->anim->current)
			{
				// Allocate delta memory
				if ((ilbm->anim->current->delta = L_AllocMemH(ilbm->memory, chunksize)))
				{
					// Read delta
					L_IFFReadChunkBytes(iff, ilbm->anim->current->delta, chunksize);

					// Increment frame count
					++ilbm->anim->frame_count;
				}

				// Abandon frame
				else
				{
					Remove((struct Node *)ilbm->anim->current);
					L_FreeMemH(ilbm->anim->current);
				}

				// Clear current frame pointer
				ilbm->anim->current = 0;
			}
			break;

		// Body
		case ID_BODY:

			// Allocate body data
			if ((body = L_AllocMemH(ilbm->memory, chunksize)))
			{
				// Read body
				if ((body_size = L_IFFReadChunkBytes(iff, body, chunksize)) == chunksize)
					ilbm->flags |= ILBMF_GOT_BODY;
			}
			break;

		// Palette
		case ID_CMAP:

			// Do we want a palette?
			if (!(flags & ILBMF_NO_PALETTE))
			{
				// Allocate palette data
				if ((ilbm->palette = L_AllocMemH(ilbm->memory, (chunksize + 2) * sizeof(ULONG))))
				{
					char *temp;

					// Get temporary buffer
					if ((temp = L_AllocMemH(ilbm->memory, chunksize)))
					{
						short entry;
						unsigned char *ptr;

						// Read palette
						L_IFFReadChunkBytes(iff, temp, chunksize);

						// Initialise palette
						ilbm->palette[0] = (chunksize / 3) << 16;

						// Fill out palette
						ptr = (unsigned char *)temp;
						for (entry = 1; entry <= chunksize; entry++, ptr++)
						{
							unsigned char val;

							// 8 bit palette?
							if (ilbm->header.flags & BMHDF_CMAPOK || ilbm->header.nPlanes > 6)
							{
								val = *ptr;
							}

							// 4 bit left justified
							else
							{
								val = ((*ptr) >> 4) & 0xf;
								val |= (val << 4);
							}

							// Convert to 32 bit left justified
							ilbm->palette[entry] =
								(((ULONG)val) << 24) | (((ULONG)val) << 16) | (((ULONG)val) << 8) | (((ULONG)val));
						}

						// Free temporary buffer
						L_FreeMemH(temp);
						ilbm->flags |= ILBMF_GOT_PALETTE;
					}

					// No palette
					else
					{
						L_FreeMemH(ilbm->palette);
						ilbm->palette = 0;
					}
				}
			}
			break;
		}
	}

	// Finished parsing file
	L_IFFClose(iff);

	// See what we've got
	if (!(ilbm->flags & ILBMF_GOT_HEADER) || !(ilbm->flags & ILBMF_GOT_BODY))
	{
		// Without a header or a body we must fail
		L_FreeILBM(ilbm);
		return 0;
	}

	// See what the customer wants
	if (flags & ILBMF_GET_BODY)
	{
		// Only wants a body
		ilbm->image.body.data = body;
		ilbm->image.body.size = body_size;
	}

	// Don't want body
	else
	{
		// Clear body flag
		ilbm->flags &= ~ILBMF_GOT_BODY;

		// Wants bitmap
		if (flags & ILBMF_GET_BITMAP)
		{
			// Create a bitmap
			if ((ilbm->image.bitmap.bitmap =
					 L_NewBitMap(ilbm->header.w, ilbm->header.h, ilbm->header.nPlanes, BMF_CLEAR | BMF_DISPLAYABLE, 0)))
			{
				// Set flag to say we got bitmap
				ilbm->flags |= ILBMF_GOT_BITMAP;

				// Decode body
				L_DecodeILBM((char *)body,
							 ilbm->header.w,
							 ilbm->header.h,
							 ilbm->header.nPlanes,
							 ilbm->image.bitmap.bitmap,
							 (ilbm->header.masking == 1) ? DIF_MASK : 0,
							 ilbm->header.compression);
			}
		}

		// Wants planes
		else if (flags & ILBMF_GET_PLANES)
		{
			// Allocate space for plane pointers
			if ((ilbm->image.planes.planes =
					 L_AllocMemH(ilbm->memory, ilbm->header.nPlanes * sizeof(unsigned short *))))
			{
				short plane;
				struct BitMap fake;
				unsigned long planesize;

				// Initialise fake bitmap
				InitBitMap(&fake, ilbm->header.nPlanes, ilbm->header.w, ilbm->header.h);
				planesize = RASSIZE(ilbm->header.w, ilbm->header.h);

				// Small enough for chip?
				if (planesize * ilbm->header.nPlanes <= 16384)
					ilbm->flags |= ILBMF_CHIP_PLANES;

				// Allocate planes
				for (plane = 0; plane < ilbm->header.nPlanes; plane++)
				{
					// Allocate chip memory if we're allowed to
					if (!(ilbm->flags & ILBMF_CHIP_PLANES) ||
						!(ilbm->image.planes.planes[plane] = AllocVec(planesize, MEMF_CHIP)))
					{
						// Allocate from normal memory
						if ((ilbm->image.planes.planes[plane] = AllocVec(planesize, 0)))
						{
							// Did we end up with chip memory anyway?
							if (plane == 0 && (TypeOfMem(ilbm->image.planes.planes[plane]) & MEMF_CHIP))
							{
								// Allocate all chip memory from now on
								ilbm->flags |= ILBMF_CHIP_PLANES;
							}

							// Otherwise, clear chip flag
							else
								ilbm->flags &= ~ILBMF_CHIP_PLANES;
						}
						else
							break;
					}
				}

				// Did we fail to get all planes?
				if (plane < ilbm->header.nPlanes)
				{
					// Free what we did get
					for (; plane >= 0; plane--)
					{
						FreeVec(ilbm->image.planes.planes[plane]);
					}

					// Free plane pointers
					L_FreeMemH(ilbm->image.planes.planes);
					ilbm->image.planes.planes = 0;
				}

				// Successful
				else
				{
					// Set plane pointers in fake bitmap
					for (plane = 0; plane < ilbm->header.nPlanes; plane++)
						fake.Planes[plane] = (PLANEPTR)ilbm->image.planes.planes[plane];

					// Decode body
					L_DecodeILBM((char *)body,
								 ilbm->header.w,
								 ilbm->header.h,
								 ilbm->header.nPlanes,
								 &fake,
								 (ilbm->header.masking == 1) ? DIF_MASK : 0,
								 ilbm->header.compression);

					// Set flag to say we've got planes
					ilbm->flags |= ILBMF_GOT_PLANES;
				}
			}
		}
	}

	// Do we need to free the body?
	if (!(flags & ILBMF_GET_BODY))
	{
		L_FreeMemH(body);
	}

	// Are we an animation?
	if (ilbm->flags & ILBMF_IS_ANIM)
	{
		// Initialise current frame
		ilbm->anim->current = (ANIMFrame *)ilbm->anim->frames.mlh_Head;

		// Are we an anim brush?
		if (ilbm->anim->current->header.interleave == 1)
			ilbm->flags |= ILBMF_IS_ANIM_BRUSH;
	}

	// Return ILBM handle
	return ilbm;
}

// Free an ILBM image
void LIBFUNC L_FreeILBM(REG(a0, ILBMHandle *ilbm))
{
	// Valid?
	if (ilbm)
	{
		// Do we have a bitmap?
		if (ilbm->flags & ILBMF_GOT_BITMAP)
		{
			// Free bitmap
			L_DisposeBitMap(ilbm->image.bitmap.bitmap);
		}

		// Or do we got planes?
		else if (ilbm->flags & ILBMF_GOT_PLANES)
		{
			short plane;

			// Free planes
			for (plane = 0; plane < ilbm->header.nPlanes; plane++)
				FreeVec(ilbm->image.planes.planes[plane]);
		}

		// Free handle memory
		L_FreeMemHandle(ilbm->memory);
		FreeVec(ilbm);
	}
}

// Decode an IFF body
void LIBFUNC L_DecodeILBM(REG(a0, char *source),
						  REG(d0, unsigned short width),
						  REG(d1, unsigned short height),
						  REG(d2, unsigned short planes),
						  REG(a1, struct BitMap *dest),
						  REG(d3, unsigned long flags),
						  REG(d4, char comp))
{
	register short bpr;
	unsigned short plane, row;
	unsigned short dest_rows, dest_depth, bufsize = 0;
	unsigned long bmoffset = 0;
	struct RastPort rp, temprp;
	struct BitMap *tempbm = 0;
	UBYTE *buffer = 0;
	signed char *src = (signed char *)source;  // needed for signedness issue
	BOOL dest_is_p96, dest_is_cgx;

	// Check valid source and destination
	if (!source || !dest)
		return;

	// Masking?
	if (flags & DIF_MASK)
		++planes;

	// Get bytes per row for source
	bpr = ((width + 15) >> 4) << 1;

	// Under 39?
	if (((struct Library *)GfxBase)->lib_Version >= 39)
	{
		// Get bitmap info
		dest_rows = GetBitMapAttr(dest, BMA_HEIGHT);
		dest_depth = GetBitMapAttr(dest, BMA_DEPTH);
	}

	// Normal
	else
	{
		dest_rows = dest->Rows;
		dest_depth = dest->Depth;
	}

	// Check height doesn't exceed destination bitmap
	if (height > dest_rows)
		height = dest_rows;

	// Allocate temporary bitmap if WritePixel needed.
	// Prefer P96's WritePixelArray path on a P96 destination; if the
	// destination isn't a P96 bitmap but cybergraphics claims it, use
	// CGX's WritePixelArray; otherwise use a planar temp bitmap +
	// WritePixelLine8 (cannot carry 24-bit data).
	//
	// Probing the destination bitmap itself (not just "is any library
	// resident?") matters on mixed setups where Picasso96API is loaded
	// but a given BitMap was allocated through CyberGraphX or is plain
	// planar - routing the row through the wrong API corrupts output.
	dest_is_p96 = FALSE;
	dest_is_cgx = FALSE;
#if !defined(__AROS__)
	if (P96Base && p96GetBitMapAttr(dest, P96BMA_ISP96))
		dest_is_p96 = TRUE;
#endif
	if (!dest_is_p96 && CyberGfxBase && GetCyberMapAttr(dest, CYBRMATTR_ISCYBERGFX))
		dest_is_cgx = TRUE;

	// Note: 24-plane ILBMs decoded into a non-RTG destination fall through
	// to the planar copy path below. That path correctly populates a true
	// 24-plane planar destination (the L_ReadILBM ILBMF_GET_BITMAP /
	// ILBMF_GET_PLANES callers create one); it is lossy only when
	// dest_depth < planes, which is the same behaviour as master. The
	// 24-bit DIF_WRITEPIX-into-8-bit-buffer overflow is prevented separately
	// by the buffer-allocation guard right below: 24-bit only allocates a
	// chunky buffer when an RTG dest is available, otherwise leaves
	// `buffer` NULL so DIF_WRITEPIX is cleared and the planar copy runs.

	if (flags & DIF_WRITEPIX)
	{
		if (planes == 24)
		{
			// 24-bit ILBM rows are RGB triples - that needs a P96 or
			// CGX destination able to take 3 bytes per pixel. The planar
			// tempbm + WritePixelLine8 path can only hold 1 byte per
			// pixel; trying to feed it the 24-bit decode would overrun
			// an 8-bit buffer (the decoder always advances ptr += 3 in
			// the 24-bit branch). If no RTG dest is available, leave
			// `buffer` NULL so DIF_WRITEPIX gets cleared below and the
			// decoder falls back to the legacy planar copy path.
			if (dest_is_p96 || dest_is_cgx)
				buffer = AllocVec((bufsize = bpr * 24), 0);
		}
		// 8bpp path: RTG write or planar tempbm fallback
		else if (dest_is_p96 || dest_is_cgx || (tempbm = L_NewBitMap(width, 1, dest_depth, 0, 0)))
		{
			buffer = AllocVec((bufsize = bpr << 3), 0);
		}

		// Got buffer?
		if (buffer)
		{
			// Set up dummy rastport
			InitRastPort(&rp);
			rp.BitMap = dest;

			// Set up temporary rastport
			if (tempbm)
			{
				InitRastPort(&temprp);
				temprp.BitMap = tempbm;
			}
		}
		else
			flags &= ~DIF_WRITEPIX;
	}

	// Run-length encoding?
	if (comp == 1)
	{
		// Go through image rows
		for (row = 0; row < height; row++)
		{
			// Write pixel?
			if (flags & DIF_WRITEPIX)
			{
				short col;

				// Clear buffer
				for (col = 0; col < bufsize; col++)
					buffer[col] = 0;
			}

			// Go through planes
			for (plane = 0; plane < planes; plane++)
			{
				// Check this plane is ok to decode into
				if (plane < dest_depth && (flags & DIF_WRITEPIX || dest->Planes[plane]))
				{
					register char *ptr;
					register short copy, col, count;
					short pnum = 0;

					// Get destination pointer
					if (flags & DIF_WRITEPIX)
					{
						// 24 bit?
						if (planes == 24)
						{
							// Get start
							if (plane < 8)
							{
								ptr = (char *)buffer;
								pnum = plane;
							}
							else if (plane < 16)
							{
								ptr = (char *)buffer + 1;
								pnum = plane - 8;
							}
							else
							{
								ptr = (char *)buffer + 2;
								pnum = plane - 16;
							}
						}
						else
							ptr = (char *)buffer;
					}
					else
						ptr = (char *)(dest->Planes[plane] + bmoffset);

					// Go through columns
					for (col = 0; col < bpr;)
					{
						// >=0 bytes copied
						if ((count = *src++) >= 0)
						{
							copy = count + 1;
							col += copy;
							if (flags & DIF_WRITEPIX)
							{
								while (copy--)
								{
									register unsigned char val;
									register short a;

									// Get value
									val = *src++;

									// 24 bit?
									if (planes == 24)
									{
										// 8 pixels in value
										for (a = 0; a < 8; a++, ptr += 3)
											if (val & (1 << (7 - a)))
												*ptr |= 1 << pnum;
									}

									// 8 pixels in value
									else
										for (a = 0; a < 8; a++, ptr++)
											if (val & (1 << (7 - a)))
												*ptr |= 1 << plane;
								}
							}
							else
								while (copy--)
									*ptr++ = *src++;
						}

						// -128<0 byte run
						else if (count != -128)
						{
							copy = 1 - count;
							col += copy;
							if (flags & DIF_WRITEPIX)
							{
								register unsigned char val;

								// Get value
								val = *src;

								while (copy--)
								{
									register short a;

									// 24 bit?
									if (planes == 24)
									{
										// 8 pixels in value
										for (a = 0; a < 8; a++, ptr += 3)
											if (val & (1 << (7 - a)))
												*ptr |= 1 << pnum;
									}

									// 8 pixels in value
									else
										for (a = 0; a < 8; a++, ++ptr)
											if (val & (1 << (7 - a)))
												*ptr |= 1 << plane;
								}
							}
							else
								while (copy--)
									*ptr++ = *src;
							++src;
						}
					}
				}

				// Destination invalid
				else
				{
					register short col, copy, count;

					for (col = 0; col < bpr;)
					{
						if ((count = *src++) >= 0)
						{
							copy = count + 1;
							col += copy;
							src += copy;
						}
						else if (count != -128)
						{
							copy = 1 - count;
							col += copy;
							++src;
						}
					}
				}
			}

			// Writepixel?
			if (flags & DIF_WRITEPIX)
			{
				// Dispatch by destination capability (decided once above).
				// Using the dest-specific probe instead of "is the library
				// resident?" avoids misrouting rows on mixed setups where
				// P96 is loaded but this particular BitMap came from CGX.
#if !defined(__AROS__)
				if (dest_is_p96)
				{
					struct RenderInfo ri;
					ri.Memory = buffer;
					ri.BytesPerRow = (planes == 24) ? width * 3 : width;
					ri.pad = 0;
					ri.RGBFormat = (planes == 24) ? RGBFB_R8G8B8 : RGBFB_CLUT;
					p96WritePixelArray(&ri, 0, 0, &rp, 0, row, width, 1);
				}
				else
#endif

				// CyberGraphX destination (CGX-only installs, or P96 resident
				// but this BitMap is a CGX one)
				if (dest_is_cgx)
				{
					WritePixelArray(buffer,
									0,
									0,
									width,
									&rp,
									0,
									row,
									width,
									1,
									(planes == 24) ? RECTFMT_RGB : RECTFMT_LUT8);
				}

				// Planar tempbm path (destination isn't RTG-capable)
				else
					WritePixelLine8(&rp, 0, row, width, buffer, &temprp);
			}

			// Increment bitmap offset
			else
				bmoffset += dest->BytesPerRow;
		}
	}

	// No encoding
	else if (comp == 0)
	{
		// The uncompressed branch only knows how to write to traditional
		// planar plane pointers. RTG destinations (chunky P96/CGX bitmaps)
		// don't have valid Planes[plane] pointers, and there is no
		// DIF_WRITEPIX-style chunky write path here yet. Guarding on
		// dest->Planes[plane] keeps a CopyMem from corrupting RTG bitmap
		// internals. The corresponding RLE branch above already has this
		// guard (it is wrapped in `(flags & DIF_WRITEPIX || dest->Planes[plane])`).
		for (row = 0; row < height; row++)
		{
			for (plane = 0; plane < planes; plane++)
			{
				if (plane < dest_depth && dest->Planes[plane])
					CopyMem(src, (char *)dest->Planes[plane] + bmoffset, bpr);
				src += bpr;
			}
			bmoffset += dest->BytesPerRow;
		}
	}

	// Free buffers, etc
	if (tempbm)
		L_DisposeBitMap(tempbm);
	if (buffer)
		FreeVec(buffer);
}

/*
// Decode an IFF body
void LIBFUNC L_DecodeILBM(
	REG(a0, unsigned char *source),
	REG(d0, unsigned short width),
	REG(d1, unsigned short height),
	REG(d2, unsigned short depth),
	REG(a1, struct BitMap *dest),
	REG(d3, char mask),
	REG(d4, char comp))
{
	RLEinfo picinfo;

	// Check valid source and destination
	if (!source || !dest) return;

	// Fill out RLEinfo structure
	picinfo.sourceptr=source;
	picinfo.destplanes=dest->Planes;
	picinfo.imagebpr=((width+15)/16)*2;
	picinfo.imageheight=height;
	picinfo.imagedepth=depth;
	picinfo.destbpr=dest->BytesPerRow;
	picinfo.destheight=dest->Rows;
	picinfo.destdepth=dest->Depth;
	picinfo.masking=mask;
	picinfo.compression=comp;
	picinfo.offset=0;

	// Decode it
	L_DecodeRLE(&picinfo);
}
*/

// Do RLE decoding
void LIBFUNC L_DecodeRLE(REG(a0, RLEinfo *rle))
{
	register char *source;
	register short bpr;
	unsigned short plane, row, planes, depth, height, destbpr;
	unsigned long bmoffset;
	PLANEPTR *planeptrs;

	planes = rle->imagedepth;
	if (rle->masking == 1)
		++planes;
	source = (char *)rle->sourceptr;
	bpr = rle->imagebpr;
	destbpr = rle->destbpr;

	// Get height to do
	if (rle->imageheight > rle->destheight)
		height = rle->destheight;
	else
		height = rle->imageheight;

	// Get depth to do
	depth = rle->destdepth;

	// Get plane pointers
	planeptrs = rle->destplanes;

	// Look at compression type
	switch (rle->compression)
	{
	// Run-length encoding
	case 1:

		// Start offset in this bitmap
		bmoffset = rle->offset;

		// Go through image rows
		for (row = 0; row < height; row++)
		{
			// Go through planes
			for (plane = 0; plane < planes; plane++)
			{
				// Check this plane is ok to decode into
				if (plane < depth && planeptrs[plane])
				{
					register char *dest;
					register short copy, col, count;

					// Get destination pointer
					dest = (char *)(planeptrs[plane] + bmoffset);

					// Go through columns
					for (col = 0; col < bpr;)
					{
						// >=0 bytes copied
						if ((count = *source++) >= 0)
						{
							copy = count + 1;
							col += copy;
							while (copy--)
								*dest++ = *source++;
						}

						// -128<0 byte run
						else if (count != -128)
						{
							copy = 1 - count;
							col += copy;
							while (copy--)
								*dest++ = *source;
							++source;
						}
					}
				}

				// Destination invalid
				else
				{
					register short col, copy, count;

					for (col = 0; col < bpr;)
					{
						if ((count = *source++) >= 0)
						{
							copy = count + 1;
							col += copy;
							source += copy;
						}
						else if (count != -128)
						{
							copy = 1 - count;
							col += copy;
							++source;
						}
					}
				}
			}
			bmoffset += destbpr;
		}
		break;

	// No encoding
	case 0:
		bmoffset = rle->offset;
		for (row = 0; row < rle->imageheight; row++)
		{
			if (row >= height)
				break;
			for (plane = 0; plane < planes; plane++)
			{
				if (plane < depth)
					CopyMem(source, (char *)planeptrs[plane] + bmoffset, bpr);
				source += bpr;
			}
			bmoffset += destbpr;
		}
		break;
	}
}

// Fake an ILBM structure from data
ILBMHandle *LIBFUNC L_FakeILBM(REG(a0, UWORD *imagedata),
							   REG(a1, ULONG *palette),
							   REG(d0, short width),
							   REG(d1, short height),
							   REG(d2, short depth),
							   REG(d3, ULONG flags))
{
	ILBMHandle *ilbm;

	// Allocate new ILBM handle
	if (!(ilbm = AllocVec(sizeof(ILBMHandle), MEMF_CLEAR)) ||
		!(ilbm->memory = L_NewMemHandle(1024, sizeof(ANIMHandle) + sizeof(ANIMFrame), MEMF_CLEAR)))
	{
		FreeVec(ilbm);
		return 0;
	}

	// Bitmap header
	ilbm->header.w = width;
	ilbm->header.h = height;
	ilbm->header.nPlanes = depth;
	ilbm->flags |= ILBMF_GOT_HEADER;

	// Do we want a palette?
	if (!(flags & ILBMF_NO_PALETTE))
	{
		// Palette?
		if (palette)
		{
			long chunksize;

			// Get size
			chunksize = (palette[0] >> 16) * 3;

			// Allocate palette data
			if ((ilbm->palette = L_AllocMemH(ilbm->memory, (chunksize + 2) * sizeof(ULONG))))
			{
				// Copy palette
				CopyMem((char *)palette, (char *)ilbm->palette, (chunksize + 2) * sizeof(ULONG));
				ilbm->flags |= ILBMF_GOT_PALETTE;
			}
		}
	}

	// Allocate space for plane pointers
	if ((ilbm->image.planes.planes = L_AllocMemH(ilbm->memory, depth * sizeof(unsigned short *))))
	{
		short plane;
		unsigned long planesize;

		// Get size of bitplane
		planesize = RASSIZE(width, height);

		// Set flag for chip memory
		ilbm->flags |= ILBMF_CHIP_PLANES;

		// Allocate planes
		for (plane = 0; plane < depth; plane++)
		{
			// Allocate plane
			if (!(ilbm->image.planes.planes[plane] = AllocVec(planesize, MEMF_CHIP)))
				break;
		}

		// Did we fail to get all planes?
		if (plane < depth)
		{
			// Free what we did get
			for (; plane >= 0; plane--)
			{
				FreeVec(ilbm->image.planes.planes[plane]);
			}

			// Free plane pointers
			L_FreeMemH(ilbm->image.planes.planes);
			ilbm->image.planes.planes = 0;
		}

		// Successful
		else
		{
			// Fill in bitmap
			for (plane = 0; plane < ilbm->header.nPlanes; plane++)
			{
				// Copy to bitplane
				CopyMem(((char *)imagedata) + (planesize * plane), (char *)ilbm->image.planes.planes[plane], planesize);
			}

			// Set flag to say we've got planes
			ilbm->flags |= ILBMF_GOT_PLANES;
		}
	}

	// Return ILBM handle
	return ilbm;
}
