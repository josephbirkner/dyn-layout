#include "StdAfx.h"

#pragma hdrstop

#include "../../GlobExport/gdiplusutil.h"

using namespace Gdiplus;

namespace Layout
{
	GdiPlusUtil::GdiPlusUtil()
	{
		GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
	}
	
	GdiPlusUtil::~GdiPlusUtil()
	{
		GdiplusShutdown(m_gdiplusToken);
	}
	
	void GdiPlusUtil::drawString(HDC hDC, RECT rctShape, std::string sText, COLORREF hColor, INT iSize, std::string sFont)
	{
			hColor |= (255 << 24);
			Graphics    graphics(hDC);
			graphics.SetClip(Rect(rctShape.left, rctShape.top, rctShape.right - rctShape.left, rctShape.bottom - rctShape.top));
			Color       crColor;
			crColor.SetFromCOLORREF(hColor);
			SolidBrush  brush(crColor);
			FontFamily  fontFamily(cstrToWstr(sFont).c_str());
			Font        font(&fontFamily, iSize, FontStyleRegular, UnitPixel);
			PointF      pointF((REAL) rctShape.left, (REAL) rctShape.top);
			graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
			graphics.DrawString(cstrToWstr(sText).c_str(), -1, &font, pointF, &brush);
	}

	void GdiPlusUtil::drawEllipse( HDC hDC, int iPosX, int iPosY, int iWidth, int iHeight, COLORREF hColor )
	{
			hColor |= (255 << 24);
			Graphics    graphics(hDC);
			Color       crColor;
			crColor.SetFromCOLORREF(hColor);
			SolidBrush  brush(crColor);
			graphics.SetSmoothingMode(SmoothingModeAntiAlias);
			graphics.FillEllipse(&brush, iPosX, iPosY, iWidth, iHeight);
	}
	
	std::wstring GdiPlusUtil::cstrToWstr( std::string const& str )
	{
		UINT nLen = str.length() + 1;
		std::wstring wstrText(nLen, '#');
		mbstowcs(&wstrText[0], str.c_str(), nLen);
		return wstrText;
	}

	void GdiPlusUtil::drawRect( HDC hDC, int iPosX, int iPosY, int iWidth, int iHeight, COLORREF hColor )
	{
			hColor |= (255 << 24);
			Graphics    graphics(hDC);
			Color       crColor;
			crColor.SetFromCOLORREF(hColor);
			SolidBrush  brush(crColor);
			graphics.FillRectangle(&brush, iPosX, iPosY, iWidth, iHeight);
	}
}
