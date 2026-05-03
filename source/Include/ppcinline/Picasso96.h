/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_PICASSO96_H
#define _PPCINLINE_PICASSO96_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef PICASSO96_BASE_NAME
#define PICASSO96_BASE_NAME P96Base
#endif /* !PICASSO96_BASE_NAME */

#define p96OpenScreenTagList(__p0) \
	LP1(90, struct Screen *, p96OpenScreenTagList, \
		struct TagItem *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96CloseScreen(__p0) \
	LP1(96, BOOL , p96CloseScreen, \
		struct Screen *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96BestModeIDTagList(__p0) \
	LP1(60, ULONG , p96BestModeIDTagList, \
		struct TagItem *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96RequestModeIDTagList(__p0) \
	LP1(66, ULONG , p96RequestModeIDTagList, \
		struct TagItem *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96AllocModeListTagList(__p0) \
	LP1(72, struct List *, p96AllocModeListTagList, \
		struct TagItem *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96FreeModeList(__p0) \
	LP1NR(78, p96FreeModeList, \
		struct List *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96GetModeIDAttr(__p0, __p1) \
	LP2(84, ULONG , p96GetModeIDAttr, \
		ULONG , __p0, d0, \
		ULONG , __p1, d1, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96AllocBitMap(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(30, struct BitMap *, p96AllocBitMap, \
		ULONG , __p0, d0, \
		ULONG , __p1, d1, \
		ULONG , __p2, d2, \
		ULONG , __p3, d3, \
		struct BitMap *, __p4, a0, \
		RGBFTYPE , __p5, d7, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96FreeBitMap(__p0) \
	LP1NR(36, p96FreeBitMap, \
		struct BitMap *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96GetBitMapAttr(__p0, __p1) \
	LP2(42, ULONG , p96GetBitMapAttr, \
		struct BitMap *, __p0, a0, \
		ULONG , __p1, d0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96LockBitMap(__p0, __p1, __p2) \
	LP3(48, LONG , p96LockBitMap, \
		struct BitMap *, __p0, a0, \
		UBYTE *, __p1, a1, \
		ULONG , __p2, d0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96UnlockBitMap(__p0, __p1) \
	LP2NR(54, p96UnlockBitMap, \
		struct BitMap *, __p0, a0, \
		LONG , __p1, d0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96WritePixelArray(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8NR(102, p96WritePixelArray, \
		struct RenderInfo *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		struct RastPort *, __p3, a1, \
		UWORD , __p4, d2, \
		UWORD , __p5, d3, \
		UWORD , __p6, d4, \
		UWORD , __p7, d5, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96ReadPixelArray(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8NR(108, p96ReadPixelArray, \
		struct RenderInfo *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		struct RastPort *, __p3, a1, \
		UWORD , __p4, d2, \
		UWORD , __p5, d3, \
		UWORD , __p6, d4, \
		UWORD , __p7, d5, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96WritePixel(__p0, __p1, __p2, __p3) \
	LP4(114, ULONG , p96WritePixel, \
		struct RastPort *, __p0, a1, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		ULONG , __p3, d2, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96ReadPixel(__p0, __p1, __p2) \
	LP3(120, ULONG , p96ReadPixel, \
		struct RastPort *, __p0, a1, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96RectFill(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6NR(126, p96RectFill, \
		struct RastPort *, __p0, a1, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		UWORD , __p3, d2, \
		UWORD , __p4, d3, \
		ULONG , __p5, d4, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96WriteTrueColorData(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8NR(132, p96WriteTrueColorData, \
		struct TrueColorInfo *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		struct RastPort *, __p3, a1, \
		UWORD , __p4, d2, \
		UWORD , __p5, d3, \
		UWORD , __p6, d4, \
		UWORD , __p7, d5, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96ReadTrueColorData(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7) \
	LP8NR(138, p96ReadTrueColorData, \
		struct TrueColorInfo *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		struct RastPort *, __p3, a1, \
		UWORD , __p4, d2, \
		UWORD , __p5, d3, \
		UWORD , __p6, d4, \
		UWORD , __p7, d5, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96PIP_OpenTagList(__p0) \
	LP1(144, struct Window *, p96PIP_OpenTagList, \
		struct TagItem *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96PIP_Close(__p0) \
	LP1(150, BOOL , p96PIP_Close, \
		struct Window *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96PIP_SetTagList(__p0, __p1) \
	LP2(156, LONG , p96PIP_SetTagList, \
		struct Window *, __p0, a0, \
		struct TagItem *, __p1, a1, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96PIP_GetTagList(__p0, __p1) \
	LP2(162, LONG , p96PIP_GetTagList, \
		struct Window *, __p0, a0, \
		struct TagItem *, __p1, a1, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96GetRTGDataTagList(__p0) \
	LP1(180, LONG , p96GetRTGDataTagList, \
		struct TagItem *, __p0, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96GetBoardDataTagList(__p0, __p1) \
	LP2(186, LONG , p96GetBoardDataTagList, \
		ULONG , __p0, d0, \
		struct TagItem *, __p1, a0, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define p96EncodeColor(__p0, __p1) \
	LP2(192, ULONG , p96EncodeColor, \
		RGBFTYPE , __p0, d0, \
		ULONG , __p1, d1, \
		, PICASSO96_BASE_NAME, 0, 0, 0, 0, 0, 0)

#if defined(USE_INLINE_STDARG) && !defined(__STRICT_ANSI__)

#include <stdarg.h>

#define p96OpenScreenTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96OpenScreenTagList((struct TagItem *)_tags);})

#define p96BestModeIDTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96BestModeIDTagList((struct TagItem *)_tags);})

#define p96RequestModeIDTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96RequestModeIDTagList((struct TagItem *)_tags);})

#define p96AllocModeListTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96AllocModeListTagList((struct TagItem *)_tags);})

#define p96PIP_OpenTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96PIP_OpenTagList((struct TagItem *)_tags);})

#define p96PIP_SetTags(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96PIP_SetTagList(__p0, (struct TagItem *)_tags);})

#define p96PIP_GetTags(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96PIP_GetTagList(__p0, (struct TagItem *)_tags);})

#define p96GetRTGDataTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96GetRTGDataTagList((struct TagItem *)_tags);})

#define p96GetBoardDataTags(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	p96GetBoardDataTagList(__p0, (struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_PICASSO96_H */
