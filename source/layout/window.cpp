#include "StdAfx.h"

#pragma hdrstop

#include "../../GlobExport/window.h"
#include "../../GlobExport/manager.h"
#include "../../GlobExport/owner.h"
#include "../../GlobExport/gdiplusutil.h"

#include <algorithm>
#include <sstream>
#include <list>

using namespace Layout;

IMPLEMENT_DYNAMIC(Window, Area)

BEGIN_MESSAGE_MAP(Window, Area)
//ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

Window::Window( Owner const& windowedLayout, Area const* pParent, CRect const& rctShape, SIZE const& hMinSize, SIZE const& hMaxSize )
	: Area(pParent, rctShape, hMinSize, hMaxSize), m_aChildLayout(windowedLayout)
{
	initWindow();
}

Window::Window( Owner const& windowedLayout, Manager const* pMgr, CRect const& rctShape, SIZE const& hMinSize, SIZE const& hMaxSize )
	: Area(pMgr, rctShape, hMinSize, hMaxSize), m_aChildLayout(windowedLayout)
{
	initWindow();
}

Window::~Window()
{
	
}

void Layout::Window::initWindow()
{
	AFXASSUME(m_aChildLayout.getHwnd());
	m_aChildLayout.setParent(this->GetSafeHwnd());
	::MoveWindow(m_aChildLayout.getHwnd(),
		m_rctCurrentVisibleShape.left,
		m_rctCurrentVisibleShape.top,
		m_rctCurrentVisibleShape.Width(),
		m_rctCurrentVisibleShape.Height(),
		TRUE
	);
}

void Layout::Window::setCurrentRect( CRect const& rctShape, bool bShapeIsScreenCoords /*= true*/ )
{
	__super::setCurrentRect(rctShape, bShapeIsScreenCoords);
	
	if(GetSafeHwnd() != NULL)
	{
		::MoveWindow(m_aChildLayout.getHwnd(),
			m_rctCurrentVisibleShape.left,
			m_rctCurrentVisibleShape.top,
			m_rctCurrentVisibleShape.Width(),
			m_rctCurrentVisibleShape.Height(),
			TRUE
		);
	}
}
