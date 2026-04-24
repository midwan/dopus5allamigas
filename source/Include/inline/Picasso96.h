/* Automatically generated header! Do not edit! */

#ifndef _INLINE_PICASSO96API_H
#define _INLINE_PICASSO96API_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef PICASSO96API_BASE_NAME
#define PICASSO96API_BASE_NAME P96Base
#endif /* !PICASSO96API_BASE_NAME */

#define p96AllocBitMap(SizeX, SizeY, Depth, Flags, Friend, RGBFormat) \
	LP6(0x1e, struct BitMap *, p96AllocBitMap, ULONG, SizeX, d0, ULONG, SizeY, d1, ULONG, Depth, d2, ULONG, Flags, d3, struct BitMap *, Friend, a0, RGBFTYPE, RGBFormat, d7, \
	, PICASSO96API_BASE_NAME)

#define p96AllocModeListTagList(Tags) \
	LP1(0x48, struct List *, p96AllocModeListTagList, struct TagItem *, Tags, a0, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96AllocModeListTags(tags...) \
	({ULONG _tags[] = { tags }; p96AllocModeListTagList((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96BestModeIDTagList(Tags) \
	LP1(0x3c, ULONG, p96BestModeIDTagList, struct TagItem *, Tags, a0, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96BestModeIDTags(tags...) \
	({ULONG _tags[] = { tags }; p96BestModeIDTagList((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96CloseScreen(Screen_) \
	LP1(0x60, BOOL, p96CloseScreen, struct Screen *, Screen_, a0, \
	, PICASSO96API_BASE_NAME)

#define p96EncodeColor(RGBFormat, Color) \
	LP2(0xc0, ULONG, p96EncodeColor, RGBFTYPE, RGBFormat, d0, ULONG, Color, d1, \
	, PICASSO96API_BASE_NAME)

#define p96FreeBitMap(BitMap_) \
	LP1NR(0x24, p96FreeBitMap, struct BitMap *, BitMap_, a0, \
	, PICASSO96API_BASE_NAME)

#define p96FreeModeList(List_) \
	LP1NR(0x4e, p96FreeModeList, struct List *, List_, a0, \
	, PICASSO96API_BASE_NAME)

#define p96GetBitMapAttr(BitMap_, Attribute) \
	LP2(0x2a, ULONG, p96GetBitMapAttr, struct BitMap *, BitMap_, a0, ULONG, Attribute, d0, \
	, PICASSO96API_BASE_NAME)

#define p96GetBoardDataTagList(Board, Tags) \
	LP2(0xba, LONG, p96GetBoardDataTagList, ULONG, Board, d0, struct TagItem *, Tags, a0, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96GetBoardDataTags(a0, tags...) \
	({ULONG _tags[] = { tags }; p96GetBoardDataTagList((a0), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96GetModeIDAttr(Mode, Attribute) \
	LP2(0x54, ULONG, p96GetModeIDAttr, ULONG, Mode, d0, ULONG, Attribute, d1, \
	, PICASSO96API_BASE_NAME)

#define p96GetRTGDataTagList(Tags) \
	LP1(0xb4, LONG, p96GetRTGDataTagList, struct TagItem *, Tags, a0, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96GetRTGDataTags(tags...) \
	({ULONG _tags[] = { tags }; p96GetRTGDataTagList((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96LockBitMap(BitMap_, Buffer, Size) \
	LP3(0x30, LONG, p96LockBitMap, struct BitMap *, BitMap_, a0, UBYTE *, Buffer, a1, ULONG, Size, d0, \
	, PICASSO96API_BASE_NAME)

#define p96OpenScreenTagList(Tags) \
	LP1(0x5a, struct Screen *, p96OpenScreenTagList, struct TagItem *, Tags, a0, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96OpenScreenTags(tags...) \
	({ULONG _tags[] = { tags }; p96OpenScreenTagList((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96PIP_Close(Window_) \
	LP1(0x96, BOOL, p96PIP_Close, struct Window *, Window_, a0, \
	, PICASSO96API_BASE_NAME)

#define p96PIP_GetIMsg(Port) \
	LP1(0xa8, struct IntuiMessage *, p96PIP_GetIMsg, struct MsgPort *, Port, a0, \
	, PICASSO96API_BASE_NAME)

#define p96PIP_GetTagList(Window_, Tags) \
	LP2(0xa2, LONG, p96PIP_GetTagList, struct Window *, Window_, a0, struct TagItem *, Tags, a1, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96PIP_GetTags(a0, tags...) \
	({ULONG _tags[] = { tags }; p96PIP_GetTagList((a0), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96PIP_OpenTagList(Tags) \
	LP1(0x90, struct Window *, p96PIP_OpenTagList, struct TagItem *, Tags, a0, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96PIP_OpenTags(tags...) \
	({ULONG _tags[] = { tags }; p96PIP_OpenTagList((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96PIP_ReplyIMsg(IntuiMessage_) \
	LP1NR(0xae, p96PIP_ReplyIMsg, struct IntuiMessage *, IntuiMessage_, a1, \
	, PICASSO96API_BASE_NAME)

#define p96PIP_SetTagList(Window_, Tags) \
	LP2(0x9c, LONG, p96PIP_SetTagList, struct Window *, Window_, a0, struct TagItem *, Tags, a1, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96PIP_SetTags(a0, tags...) \
	({ULONG _tags[] = { tags }; p96PIP_SetTagList((a0), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96ReadPixel(rp, x, y) \
	LP3(0x78, ULONG, p96ReadPixel, struct RastPort *, rp, a1, UWORD, x, d0, UWORD, y, d1, \
	, PICASSO96API_BASE_NAME)

#define p96ReadPixelArray(ri, DestX, DestY, rp, SrcX, SrcY, SizeX, SizeY) \
	LP8NR(0x6c, p96ReadPixelArray, struct RenderInfo *, ri, a0, UWORD, DestX, d0, UWORD, DestY, d1, struct RastPort *, rp, a1, UWORD, SrcX, d2, UWORD, SrcY, d3, UWORD, SizeX, d4, UWORD, SizeY, d5, \
	, PICASSO96API_BASE_NAME)

#define p96ReadTrueColorData(tci, DestX, DestY, rp, SrcX, SrcY, SizeX, SizeY) \
	LP8NR(0x8a, p96ReadTrueColorData, struct TrueColorInfo *, tci, a0, UWORD, DestX, d0, UWORD, DestY, d1, struct RastPort *, rp, a1, UWORD, SrcX, d2, UWORD, SrcY, d3, UWORD, SizeX, d4, UWORD, SizeY, d5, \
	, PICASSO96API_BASE_NAME)

#define p96RectFill(rp, MinX, MinY, MaxX, MaxY, color) \
	LP6NR(0x7e, p96RectFill, struct RastPort *, rp, a1, UWORD, MinX, d0, UWORD, MinY, d1, UWORD, MaxX, d2, UWORD, MaxY, d3, ULONG, color, d4, \
	, PICASSO96API_BASE_NAME)

#define p96RequestModeIDTagList(Tags) \
	LP1(0x42, ULONG, p96RequestModeIDTagList, struct TagItem *, Tags, a0, \
	, PICASSO96API_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define p96RequestModeIDTags(tags...) \
	({ULONG _tags[] = { tags }; p96RequestModeIDTagList((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define p96UnlockBitMap(BitMap_, Lock) \
	LP2NR(0x36, p96UnlockBitMap, struct BitMap *, BitMap_, a0, LONG, Lock, d0, \
	, PICASSO96API_BASE_NAME)

#define p96WritePixel(rp, x, y, color) \
	LP4(0x72, ULONG, p96WritePixel, struct RastPort *, rp, a1, UWORD, x, d0, UWORD, y, d1, ULONG, color, d2, \
	, PICASSO96API_BASE_NAME)

#define p96WritePixelArray(ri, SrcX, SrcY, rp, DestX, DestY, SizeX, SizeY) \
	LP8NR(0x66, p96WritePixelArray, struct RenderInfo *, ri, a0, UWORD, SrcX, d0, UWORD, SrcY, d1, struct RastPort *, rp, a1, UWORD, DestX, d2, UWORD, DestY, d3, UWORD, SizeX, d4, UWORD, SizeY, d5, \
	, PICASSO96API_BASE_NAME)

#define p96WriteTrueColorData(tci, SrcX, SrcY, rp, DestX, DestY, SizeX, SizeY) \
	LP8NR(0x84, p96WriteTrueColorData, struct TrueColorInfo *, tci, a0, UWORD, SrcX, d0, UWORD, SrcY, d1, struct RastPort *, rp, a1, UWORD, DestX, d2, UWORD, DestY, d3, UWORD, SizeX, d4, UWORD, SizeY, d5, \
	, PICASSO96API_BASE_NAME)

#endif /* !_INLINE_PICASSO96API_H */
