#include "StdAfx.h"
#pragma hdrstop

#include "../../GlobExport/splitter.h"
#include "../../GlobExport/area.h"
#include "../../GlobExport/manager.h"
#include "../../GlobExport/gdiplusutil.h"

using namespace Layout;

IMPLEMENT_DYNAMIC(Splitter, CStatic)

BEGIN_MESSAGE_MAP(Splitter, CStatic)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

Splitter* Splitter::Create( Area const* pArea, CRect rctSplitter, Orientation nOrientation, SplitterAlignment nAlignment )
{
	DWORD dwStyle = WS_CHILD|WS_VISIBLE|SS_NOTIFY|SS_OWNERDRAW;
	
	HWND hWndParent = pArea->getManager()->getWnd()->GetSafeHwnd();
	UINT iSplitterId = pArea->getManager()->getNewControlID();
	
	HWND hWnd = ::CreateWindow(
		"STATIC",
		"",
		dwStyle,
		rctSplitter.left,
		rctSplitter.top,
		rctSplitter.Width(),
		rctSplitter.Height(),
		hWndParent,
		(HMENU) iSplitterId,
		AfxGetApp()->m_hInstance,
		NULL
	);
		
	Splitter* pResult = new Splitter(pArea, hWnd, nOrientation, nAlignment);
	pResult->SubclassWindow(hWnd);
	
	return pResult;
}

Splitter::Splitter( Area const* pArea, HWND hWnd, Orientation nOrientation, SplitterAlignment nAlignment )
	: Control(
		pArea->getManager(),
		hWnd,
		nOrientation == Horizontal ? (Align::Mode&) Align::Resize() : (nAlignment == AlignHigh ? (Align::Mode&) Align::TopLeft() : (nAlignment == AlignLow ? (Align::Mode&) Align::BottomRight() : (Align::Mode&) Align::Relative(false))),
		nOrientation == Vertical   ? (Align::Mode&) Align::Resize() : (nAlignment == AlignHigh ? (Align::Mode&) Align::TopLeft() : (nAlignment == AlignLow ? (Align::Mode&) Align::BottomRight() : (Align::Mode&) Align::Relative(false)))
	),
	m_nAlignment(nAlignment),
	m_nOrientation(nOrientation),
	m_bDragging(false),
	m_hHandleColor(0)
{
	setAlignmentArea(pArea);
	setHandleColor(RGBA(200, 200, 200, 255));
}

Splitter::~Splitter()
{
}

bool Splitter::move( int iHorzPos, int iVertPos )
{
	CRect rctOrig(m_rctCurrent);

	bool bResult = false;
	if(m_nOrientation == Horizontal && m_rctCurrent.top != iVertPos)
	{
		bResult = true;
		m_rctCurrent.MoveToY(iVertPos);
	}
	else if(m_nOrientation == Vertical && m_rctCurrent.left != iHorzPos)
	{
		bResult = true;
		m_rctCurrent.MoveToX(iHorzPos);
	}
	
	if(bResult)
	{
		getArea()->checkSplitterRectWithChildAreaMinSizesAndAutoFold(m_rctCurrent);
		if(rctOrig == m_rctCurrent)
			bResult = false;
			
		if(bResult)
		{
			getControl()->MoveWindow(&m_rctCurrent);
			updateOrigRect();
		}
	}
	return bResult;
}

afx_msg BOOL Splitter::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
	// Set the right cursor
	if(getOrientation() == Horizontal)
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
	else
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
		
	return TRUE;
}

void Splitter::OnMouseMove( UINT nFlags, CPoint point )
{
	if(m_bDragging)
	{
		this->ClientToScreen(&point);
		this->GetParent()->ScreenToClient(&point);
		if(move(point.x, point.y))
		{
			const_cast<Area*>(getArea())->updateChildAreas();
			getArea()->getManager()->updateAllOrigRect();
		}
	}
}

void Splitter::OnLButtonDown( UINT nFlags, CPoint point )
{
	m_bDragging = true;
	SetCapture();
}

void Splitter::OnLButtonUp( UINT nFlags, CPoint point )
{
	m_bDragging = false;
	ReleaseCapture();
	
	// On Mouse Release, the User returns the control
	// of the splitter to the layout engine. This is the
	// moment, when the splitter has to ask its splitted
	// areas, if one of them collapsed automatically because
	// it was pushed below its minimum size.
	update();
	const_cast<Area*>(getArea())->updateChildAreas();
}

void Splitter::update()
{
	CRect rctOld(getRect());
	
	// update horizontal and vertical alignment
	m_pHorzAlign->update(this, Align::Horizontal, m_rctCurrent);
	m_pVertAlign->update(this, Align::Vertical, m_rctCurrent); 
	
	// incorporate min/max size
	bool bNewOrigSize = getArea()->checkSplitterRectWithChildAreaMinSizesAndAutoFold(m_rctCurrent);
		
	// enforce the new rect if necessary
	if( rctOld != m_rctCurrent )
	{
		MoveWindow(m_rctCurrent, true);
		getManager()->getWnd()->InvalidateRect(&m_rctCurrent, TRUE);
		if(bNewOrigSize)
			getManager()->updateAllOrigRect();
	}
}

void Layout::Splitter::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if(m_hHandleColor == NULL)
		return;
	
	const int iDotSpacing = 4;
	const int iDotSize = 5;
	
	HDC hDC = lpDrawItemStruct->hDC;
	CRect rctSplitter = lpDrawItemStruct->rcItem;
	int iSize = iDotSpacing * 2 + iDotSize * 3;
	POINT ptHandle;
	
	// Reduce rect to size
	if(getOrientation() == Horizontal)
	{
		ptHandle.x = (rctSplitter.Width() - iSize)/2;
		ptHandle.y = rctSplitter.top;
	}
	else
	{
		ptHandle.y = (rctSplitter.Height() - iSize)/2;
		ptHandle.x = rctSplitter.left;
	}
	
	for( int i = 0; i < 3; ++i)
	{
		gdiPlusUtil.drawEllipse(hDC, ptHandle.x, ptHandle.y, iDotSize, iDotSize, m_hHandleColor);
		if(getOrientation() == Horizontal)
			ptHandle.x += iDotSpacing + iDotSize;
		else
			ptHandle.y += iDotSpacing + iDotSize;
	}
	
	return;
}

void Layout::Splitter::setHandleColor( COLORREF hColor )
{
	m_hHandleColor = hColor;
}

