#ifndef _GDIPLUSUTIL_HPP_
#define _GDIPLUSUTIL_HPP_

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#include <gdiplus.h>
#undef max
#undef min

namespace Layout
{
	#define _LAYOUT_AREA_BORDERSIZE   6
	#define _LAYOUT_AREA_TITLEOFFSET  5
	#define _LAYOUT_AREA_TEXTSIZE     13
	#define _LAYOUT_SPLITTER_SIZE     6
	#define _LAYOUT_AREA_FOLDEDSIZE   _LAYOUT_AREA_TITLEOFFSET * 2 + _LAYOUT_AREA_TEXTSIZE + 3
	
	class GdiPlusUtil
	{
	public:
		GdiPlusUtil();
		~GdiPlusUtil();
		LAYOUT_API bool isInitialized() {return m_bInitialized;}
		LAYOUT_API void drawString(HDC hDC, RECT rctShape, std::string sText, COLORREF hColor, INT iSize = _LAYOUT_AREA_TEXTSIZE, std::string sFont = "Segoe UI");
		LAYOUT_API void drawEllipse( HDC hDC, int iPosX, int iPosY, int iWidth, int iHeight, COLORREF hColor );
		LAYOUT_API void drawRect( HDC hDC, int iPosX, int iPosY, int iWidth, int iHeight, COLORREF hColor );
	
	private:
		bool m_bInitialized;
		Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
		ULONG_PTR m_gdiplusToken;
		
		std::wstring cstrToWstr(std::string const& str);
	};

	static GdiPlusUtil gdiPlusUtil;
}

#endif
