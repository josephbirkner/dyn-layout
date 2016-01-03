#ifndef _LAYOUT_AREACREATEPARAMS_
#define _LAYOUT_AREACREATEPARAMS_

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

#include <map>

namespace Layout
{
	struct _AreaCreateParamsImpl;
	
	#define RGBA(r,g,b,a) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16))|(((DWORD)(BYTE)(a))<<24))

	enum AreaStyles
	{
		AreaStyleFoldable = 1,
		AreaStyleDrawBk = 2,
		AreaStyleDrawTitle = 4,
		AreaStyleHover = 8,
		AreaStyleHoverTitle = 16,
		AreaStyleDrawLeftLine = 32
	};
	
	class AreaColorScheme
	{
		friend class AreaColorScheme;
	
	public:
		LAYOUT_API AreaColorScheme();
		LAYOUT_API AreaColorScheme(COLORREF aBk, COLORREF aHover = 0, COLORREF aText = 0, COLORREF aTextHoverColor = 0);
		LAYOUT_API AreaColorScheme(AreaColorScheme const& aOther);
		LAYOUT_API virtual ~AreaColorScheme();
		
		/** Set the areas background color */
		LAYOUT_API virtual void setColor(AreaStyles nColorForThisStyleElement, COLORREF aColor);
		LAYOUT_API virtual COLORREF getColor(AreaStyles nColorForThisStyleElement) const;
		LAYOUT_API virtual HBRUSH getBrush(AreaStyles nBrushForThisStyleElement) const;
		
		/** Set the background, hover and text color with one AreaColorScheme */
		LAYOUT_API virtual void setColorScheme(AreaColorScheme const& aScheme);
		
	private:
		typedef std::pair<COLORREF, HBRUSH> ColorBrushPair;
		typedef std::map<AreaStyles, ColorBrushPair> StyleColorBrushMap;
		typedef std::pair<AreaStyles, ColorBrushPair> StyleColorBrushPair;
		
		mutable StyleColorBrushMap m_mapStyleColorBrush;
	};
	
	class AreaProperties : public AreaColorScheme
	{
	public:
		LAYOUT_API AreaProperties();
		LAYOUT_API AreaProperties(UINT nID, long lStyle, AreaColorScheme aColors, std::string sCaption);
		LAYOUT_API virtual ~AreaProperties();
		
		/** Set the name of the area */
		LAYOUT_API virtual void setName(std::string sName);
		LAYOUT_API virtual std::string getName() const;
		
		/** Set the control limiting the area towards its splitter */
		LAYOUT_API virtual void setControl(HWND hWnd);
		LAYOUT_API virtual void setControl(UINT nID);
		LAYOUT_API virtual HWND getCtrlWnd() const;
		LAYOUT_API virtual UINT getCtrlId() const;
		
		/** Set the areas style. This is a combination if the AreaStyles enum. */
		LAYOUT_API virtual void setStyle(long iStyle);
		LAYOUT_API virtual long getStyle() const;
		LAYOUT_API virtual bool hasStyle(AreaStyles nStyle) const;
		
		/** Adapt property values from another property object */
		LAYOUT_API virtual void adaptValues(AreaProperties const& aOtherProps);
		
	private:
		_AreaCreateParamsImpl* m_pImpl;
	};
}

#endif
