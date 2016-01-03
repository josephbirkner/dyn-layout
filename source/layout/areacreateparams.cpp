#include "StdAfx.h"

#pragma hdrstop

#include "../../GlobExport/areacreateparams.h"

namespace Layout
{
	struct _AreaCreateParamsImpl
	{
		_AreaCreateParamsImpl() : m_hControl(0), m_nControl(0), m_lStyle(0) {;}

		HWND m_hControl;
		UINT m_nControl;
		std::string m_sName;
		long m_lStyle;
	};

	void AreaColorScheme::setColorScheme( AreaColorScheme const& aScheme )
	{
		for each(StyleColorBrushPair const& aStyleColorBrush in aScheme.m_mapStyleColorBrush)
		{
			// Only insert the color from the other scheme if it actually exists
			if(aStyleColorBrush.second.second != NULL)
				setColor(aStyleColorBrush.first, aStyleColorBrush.second.first);
		}
	}
	
	void AreaColorScheme::setColor(AreaStyles nColorForThisStyleElement, COLORREF aColor)
	{
		StyleColorBrushPair pairNewStyleColorBrush(nColorForThisStyleElement, ColorBrushPair(aColor, NULL));
		std::pair<StyleColorBrushMap::iterator, bool> insertResult = m_mapStyleColorBrush.insert(pairNewStyleColorBrush);
		ColorBrushPair& aColorBrushPair = insertResult.first->second;
		
		// If the entry existed, we need to delete the previous brush first
		if(aColorBrushPair.second != NULL)
			DeleteObject(aColorBrushPair.second);
		
		// Create the new brush and and set the color in the map
		aColorBrushPair.first = aColor;
		aColorBrushPair.second = CreateSolidBrush(aColor);
	}
		
	COLORREF AreaColorScheme::getColor(AreaStyles nColorForThisStyleElement) const
	{
		return m_mapStyleColorBrush[nColorForThisStyleElement].first;
	}
	
	HBRUSH AreaColorScheme::getBrush(AreaStyles nColorForThisStyleElement) const
	{
		return m_mapStyleColorBrush[nColorForThisStyleElement].second;
	}
	
	AreaColorScheme::AreaColorScheme( AreaColorScheme const& aOther )
	{
		setColorScheme(aOther);
	}

	AreaColorScheme::AreaColorScheme( COLORREF aBk, COLORREF aHover /*= 0*/, COLORREF aText /*= 0*/, COLORREF aTextHover /*= 0*/ )
	{
		setColor(AreaStyleDrawBk, aBk);
		setColor(AreaStyleDrawTitle, aText);
		setColor(AreaStyleHover, aHover);
		setColor(AreaStyleHoverTitle, aTextHover);
	}

	AreaColorScheme::AreaColorScheme()
	{
	}

	AreaColorScheme::~AreaColorScheme()
	{
		for each(std::pair<AreaStyles, std::pair<COLORREF, HBRUSH>> const& aStyleColorBrush in m_mapStyleColorBrush)
		{
			// Only delete the brush if it actually has a brush
			if(aStyleColorBrush.second.second != NULL)
				DeleteObject(aStyleColorBrush.second.second);
		}
	}

	AreaProperties::AreaProperties()
	{
		m_pImpl = new _AreaCreateParamsImpl();
	}

	AreaProperties::AreaProperties( UINT nID, long lStyle, AreaColorScheme aColors, std::string sCaption )
	{
		m_pImpl = new _AreaCreateParamsImpl();
		setControl(nID);
		setName(sCaption);
		setColorScheme(aColors);
		setStyle(lStyle);
	}

	AreaProperties::~AreaProperties()
	{
		delete m_pImpl;
	}
	
	void AreaProperties::setControl(HWND hControl)
	{
		m_pImpl->m_hControl = hControl;
	}
	
	HWND AreaProperties::getCtrlWnd() const
	{
		return m_pImpl->m_hControl;
	}
	
	void AreaProperties::setControl(UINT nControl)
	{
		m_pImpl->m_nControl = nControl;
	}
	
	UINT AreaProperties::getCtrlId() const
	{
		return m_pImpl->m_nControl;
	}
	
	void AreaProperties::setName(std::string sName)
	{
		m_pImpl->m_sName = sName;
	}
	
	std::string AreaProperties::getName() const
	{
		return m_pImpl->m_sName;
	}
	
	void AreaProperties::setStyle(long lStyle)
	{
		m_pImpl->m_lStyle = lStyle;
	}
	
	long AreaProperties::getStyle() const
	{
		return m_pImpl->m_lStyle;
	}

	void AreaProperties::adaptValues( AreaProperties const& aOtherProps )
	{
		setName(aOtherProps.getName());
		setControl(aOtherProps.getCtrlId());
		setControl(aOtherProps.getCtrlWnd());
		setColorScheme(aOtherProps);
		setStyle(aOtherProps.getStyle());
	}

	bool AreaProperties::hasStyle( AreaStyles nStyle ) const
	{
		return (getStyle() & (long) nStyle) != 0;
	}
}

