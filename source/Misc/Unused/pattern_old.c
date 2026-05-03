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

/*
// Load a backfill pattern
void GetPattern(PatternData *pattern,struct Screen *screen)
{
	// Initially make invalid
	pattern->valid=FALSE;

	// Set screen pointer
	pattern->screen=screen;

	// Check for invalid data
	if (!pattern->data) return;

	// Bitmap pattern?
	if (pattern->prefs.wbp_Flags&WBPF_PATTERN)
	{
		UWORD *ptr,*planes[MAXDEPTH];
		short num,row,col,pen;

		// Get plane pointers
		for (num=0;num<pattern->prefs.wbp_Depth;num++)
		{
			planes[num]=((UWORD *)pattern->data)+(PAT_HEIGHT*num);
		}
		pattern->fill_plane_key=0;

		// Under 37 use fill pattern as supplied
		if (((struct Library *)GfxBase)->lib_Version<39)
		{
			for (num=0;num<pattern->prefs.wbp_Depth;num++)
			{
				CopyMem(
					(char *)planes[num],
					(char *)pattern->fill[num],
					PAT_HEIGHT*sizeof(UWORD));
				pattern->fill_plane_key|=1<<num;
			}
		}

		// Otherwise, we need to remap
		else
		{
			// Clear fill initially
			for (num=0,ptr=pattern->fill[0];num<128;num++,ptr++)
				*ptr=0;

			// Go through pattern rows
			for (row=0;row<PAT_HEIGHT;row++)
			{
				// Go through bits
				for (col=0;col<PAT_WIDTH;col++)
				{
					// Clear pen
					pen=0;

					// Calculate pixel colour
					for (num=0;num<pattern->prefs.wbp_Depth;num++)
					{
						// Is bit set in this bitplane?
						if (planes[num][row]&(1<<col)) pen|=(1<<num);
					}

					// Pixel set?
					if (pen)
					{
						// Map pens 4-7 to top four colours
						if (pen>=4)
						{
							pen=(1<<screen->RastPort.BitMap->Depth)-(8-pen);
						}

						// Remap
						for (num=0;num<8;num++)
						{
							// Want this bit?
							if (pen&(1<<num))
							{
								// Set bit
								pattern->fill[num][row]|=1<<col;
								pattern->fill_plane_key|=(1<<num);
							}
						}
					}
				}
			}
		}
	}

	// Picture; must have datatypes
	else
	if (DataTypesBase)
	{
		// Get picture object
		if (pattern->object=
			NewDTObject(pattern->data,
				DTA_GroupID,GID_PICTURE,
				PDTA_Screen,screen,
				PDTA_FreeSourceBitMap,TRUE,
				OBP_Precision,PRECISION_IMAGE,
				TAG_END))
		{
			struct BitMapHeader *header;
			struct BitMap *bitmap;
			long which=PDTA_DestBitMap;

			// Layout picture (should check for success)
			DoMethod(pattern->object,DTM_PROCLAYOUT,0,1);

			// Get bitmap pointer and header
			GetDTAttrs(pattern->object,
				PDTA_BitMap,&bitmap,
				PDTA_DestBitMap,&pattern->bitmap,
				PDTA_BitMapHeader,&header,
				TAG_END);

			// No dest bitmap?
			if (!pattern->bitmap)
			{
				pattern->bitmap=bitmap;
				which=PDTA_BitMap;
			}

			// Valid bitmap?
			if (pattern->bitmap)
			{
				short depth;

				// Get width and height
				pattern->width=header->bmh_Width;
				pattern->height=header->bmh_Height;

				// Get depth
				depth=GetBitMapAttr(pattern->bitmap,BMA_DEPTH);

				// No point allocating something deeper than our screen
				if (depth>screen->RastPort.BitMap->Depth)
					depth=screen->RastPort.BitMap->Depth;

				// Try and allocate friend bitmap
				if (pattern->friend_bitmap=
					AllocBitMap(
						pattern->width,
						pattern->height,
						depth,
						0,
						screen->RastPort.BitMap))
				{
					// Copy image to friend bitmap
					BltBitMap(
						pattern->bitmap,0,0,
						pattern->friend_bitmap,0,0,
						pattern->width,pattern->height,
						0xc0,0xff,0);

					// Free class-allocated bitmap
					FreeBitMap(pattern->bitmap);

					// Set new bitmap pointer
					SetDTAttrs(pattern->object,0,0,
						which,AllocBitMap(1,1,1,0,0),
						TAG_DONE);

					// Store pointer to friend bitmap
					pattern->bitmap=pattern->friend_bitmap;
				}
			}
		}
	}

	// Pattern ok now
	pattern->valid=TRUE;
}
*/

/*
// Load a backfill pattern
void GetPattern(PatternData *pattern,struct Screen *screen)
{
	// Initially make invalid
	pattern->valid=FALSE;

	// Set screen pointer
	pattern->screen=screen;

	// Check for invalid data
	if (!pattern->data) return;

	// Bitmap pattern?
	if (pattern->prefs.wbp_Flags&WBPF_PATTERN)
	{
		UWORD *ptr,*planes[MAXDEPTH];
		short num,row,col,pen;

		// Get plane pointers
		for (num=0;num<pattern->prefs.wbp_Depth;num++)
		{
			planes[num]=((UWORD *)pattern->data)+(PAT_HEIGHT*num);
		}
		pattern->fill_plane_key=0;

		// Under 37 use fill pattern as supplied
		if (((struct Library *)GfxBase)->lib_Version<39)
		{
			for (num=0;num<pattern->prefs.wbp_Depth;num++)
			{
				CopyMem(
					(char *)planes[num],
					(char *)pattern->fill[num],
					PAT_HEIGHT*sizeof(UWORD));
				pattern->fill_plane_key|=1<<num;
			}
		}

		// Otherwise, we need to remap
		else
		{
			// Clear fill initially
			for (num=0,ptr=pattern->fill[0];num<128;num++,ptr++)
				*ptr=0;

			// Go through pattern rows
			for (row=0;row<PAT_HEIGHT;row++)
			{
				// Go through bits
				for (col=0;col<PAT_WIDTH;col++)
				{
					// Clear pen
					pen=0;

					// Calculate pixel colour
					for (num=0;num<pattern->prefs.wbp_Depth;num++)
					{
						// Is bit set in this bitplane?
						if (planes[num][row]&(1<<col)) pen|=(1<<num);
					}

					// Pixel set?
					if (pen)
					{
						// Map pens 4-7 to top four colours
						if (pen>=4)
						{
							pen=(1<<screen->RastPort.BitMap->Depth)-(8-pen);
						}

						// Remap
						for (num=0;num<8;num++)
						{
							// Want this bit?
							if (pen&(1<<num))
							{
								// Set bit
								pattern->fill[num][row]|=1<<col;
								pattern->fill_plane_key|=(1<<num);
							}
						}
					}
				}
			}
		}
	}

	// Picture; must have datatypes
	else
	if (DataTypesBase)
	{
		// Get picture object
		if (pattern->object=
			NewDTObject(pattern->data,
				DTA_GroupID,GID_PICTURE,
				PDTA_Screen,screen,
				PDTA_FreeSourceBitMap,TRUE,
				OBP_Precision,PRECISION_IMAGE,
				TAG_END))
		{
			struct BitMapHeader *header;
			struct BitMap *bitmap;

			// Layout picture (should check for success)
			DoMethod(pattern->object,DTM_PROCLAYOUT,0,1);

			// Get bitmap pointer and header
			GetDTAttrs(pattern->object,
				PDTA_BitMap,&bitmap,
				PDTA_DestBitMap,&pattern->bitmap,
				PDTA_BitMapHeader,&header,
				TAG_END);

			// No dest bitmap?
			if (!pattern->bitmap) pattern->bitmap=bitmap;

			// Valid bitmap?
			if (pattern->bitmap)
			{
				short depth;

				// Get width and height
				pattern->width=header->bmh_Width;
				pattern->height=header->bmh_Height;

				// Get depth
				depth=GetBitMapAttr(pattern->bitmap,BMA_DEPTH);

				// No point allocating something deeper than our screen
				if (depth>screen->RastPort.BitMap->Depth)
					depth=screen->RastPort.BitMap->Depth;

				// Try and allocate friend bitmap
				if (pattern->friend_bitmap=
					AllocBitMap(
						pattern->width,
						pattern->height,
						depth,
						0,
						screen->RastPort.BitMap))
				{
					UBYTE *table;
					long count=0;

					// Copy image to friend bitmap
					BltBitMap(
						pattern->bitmap,0,0,
						pattern->friend_bitmap,0,0,
						pattern->width,pattern->height,
						0xc0,0xff,0);

					// Store pointer to friend bitmap
					pattern->bitmap=pattern->friend_bitmap;

					// Get pen information
					GetDTAttrs(pattern->object,
						PDTA_Allocated,&table,
						PDTA_NumAlloc,&count,
						TAG_END);

					// Were any pens allocated?
					if (count>0 && table)
					{
						// Allocate pen array
						if (pattern->pen_array=AllocVec(count*sizeof(short),MEMF_CLEAR))
						{
							short a;

							// Copy the pen table
							for (a=0;a<count;a++)
								pattern->pen_array[a]=table[a];

							// Store pen count
							pattern->pen_count=count;

							// Forbid here so we don't lose the pens
							Forbid();
						}
					}

					// Free datatypes object
					DisposeDTObject(pattern->object);
					pattern->object=0;

					// Allocate the pens so they won't be lost
					if (pattern->pen_array)
					{
						short a;

						for (a=0;a<count;a++)
						{
							pattern->pen_array[a]=
								ObtainPen(
									screen->ViewPort.ColorMap,
									pattern->pen_array[a],
									0,0,0,
									PENF_NO_SETCOLOR);
						}

						// Permit now the pens are safe
						Permit();
					}
				}
			}
		}
	}

	// Pattern ok now
	pattern->valid=TRUE;
}
*/

/*
		long start_offset_x,start_offset_y;
		long pos_x,pos_y,end_x,end_y;

		// Calculate starting y offset
		start_offset_y=info->offsety%pattern->height;

		// Get starting and ending positions
		pos_y=info->bounds.MinY;
		end_x=info->bounds.MaxX;
		end_y=info->bounds.MaxY;

		// Draw backfill picture
		while (pos_y<=end_y)
		{
			short height;

			// Get height we can draw
			height=pattern->height-start_offset_y;
			if (height>(end_y-pos_y)+1) height=(end_y-pos_y)+1;

			// Get starting x position
			pos_x=info->bounds.MinX;
			start_offset_x=info->offsetx%pattern->width;

			// Draw this row
			while (pos_x<=end_x)
			{
				short width;

				// Get width we can draw
				width=pattern->width-start_offset_x;
				if (width>(end_x-pos_x)+1) width=(end_x-pos_x)+1;

				// Draw it!
				BltBitMap(
					pattern->bitmap,
					start_offset_x,start_offset_y,
					rp.BitMap,
					pos_x,pos_y,
					width,height,
					0xc0,0xff,0);

				// Increment x position
				pos_x+=width;

				// Reset x offset
				start_offset_x=0;
			}

			// Increment y position
			pos_y+=height;

			// Reset y offset
			start_offset_y=0;
		}

*/
