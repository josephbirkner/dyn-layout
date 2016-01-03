#include "StdAfx.h"
#pragma hdrstop

#include "../../GlobExport/area.h"
#include "../../GlobExport/manager.h"
#include "../../GlobExport/gdiplusutil.h"

#include <boost/icl/interval_map.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <limits>
#include <sstream>
#include <list>

using namespace Layout;

IMPLEMENT_DYNAMIC(Area, CStatic)

BEGIN_MESSAGE_MAP(Area, CStatic)
ON_WM_ERASEBKGND()
ON_WM_MOUSELEAVE()
ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

static SIZE const FOLDEDSIZE = {_LAYOUT_AREA_FOLDEDSIZE, _LAYOUT_AREA_FOLDEDSIZE};

namespace
{
struct MaxDistance
{
	MaxDistance() : _v(0)
	{}
	
	MaxDistance(int v) : _v(v)
	{}
	
	MaxDistance& operator +=(const MaxDistance& v)
	{
		_v = std::max(_v, v._v);
		return *this;
	}
	
	operator int() const
	{
		return _v;
	}
	
	int _v;
};

bool operator ==(const MaxDistance& lhs, const MaxDistance& rhs)
{
	return lhs._v == rhs._v;
}

int CalcMinSize(const boost::icl::interval_map<int, MaxDistance>& low, const boost::icl::interval_map<int, MaxDistance>& high)
{
	using namespace boost::icl;
	
	int minSize = 0;
	for (interval_map<int, MaxDistance>::const_iterator lowIter = low.begin();
	     lowIter != low.end(); ++lowIter)
	{
		discrete_interval<int> range = (*lowIter).first;
		interval_map<int, MaxDistance>::const_iterator highIter = high.find(range);
		if (highIter != high.end())
			minSize = std::max(minSize, (*lowIter).second + (*highIter).second);
	}
	
	return minSize;
}
}

POINT Layout::Area::addToClientRectVisibleTopLeftPoint( int iOffX, int iOffY )
{
	POINT ptTopLeft = m_rctCurrentVisibleClientShape.TopLeft();
	ptTopLeft.x += iOffX;
	ptTopLeft.y += iOffY;
	return ptTopLeft;
}

CRect Layout::Area::getVisibleClientRect()
{
	CRect rctVisibleRegion = m_rctCurrentClientShape;
	CRect rctWindowClientRect;
	getManager()->getWnd()->GetClientRect(&rctWindowClientRect);

	if(rctVisibleRegion.top < rctWindowClientRect.top)
		rctVisibleRegion.top = rctWindowClientRect.top;
	if(rctVisibleRegion.left < rctWindowClientRect.left)
		rctVisibleRegion.left = rctWindowClientRect.left;
	if(rctVisibleRegion.bottom > rctWindowClientRect.bottom)
		rctVisibleRegion.bottom = rctWindowClientRect.bottom;
	if(rctVisibleRegion.right > rctWindowClientRect.right)
		rctVisibleRegion.right = rctWindowClientRect.right;

	bool bTop(true), bBottom(true), bLeft(true), bRight(true);
	getTouchedParentWindowEdges(bTop, bBottom, bLeft, bRight);

	if(bTop)
		rctVisibleRegion.top += _LAYOUT_AREA_BORDERSIZE;
	if(bBottom)
		rctVisibleRegion.bottom -= _LAYOUT_AREA_BORDERSIZE;
	if(bLeft)
		rctVisibleRegion.left += _LAYOUT_AREA_BORDERSIZE;
	if(bRight)
		rctVisibleRegion.right -= _LAYOUT_AREA_BORDERSIZE;

	return rctVisibleRegion;
}

void Area::getTouchedParentWindowEdges( __inout bool& bTop, __inout bool& bBottom, __inout bool& bLeft, __inout bool& bRight ) const
{
	if(getParent())
	{
		getParent()->getTouchedParentWindowEdges(bTop, bBottom, bLeft, bRight);

		if(
			bTop &&
			getParent()->getSplitter()->getOrientation() == Splitter::Horizontal &&
			isLoArea()
		)
			bTop = false;

		if(
			bBottom &&
			getParent()->getSplitter()->getOrientation() == Splitter::Horizontal &&
			isHiArea()
		)
			bBottom = false;

		if(
			bLeft &&
			getParent()->getSplitter()->getOrientation() == Splitter::Vertical &&
			isLoArea()
		)
			bLeft = false;

		if(
			bRight &&
			getParent()->getSplitter()->getOrientation() == Splitter::Vertical &&
			isHiArea()
		)
			bRight = false;
	}
}

void Area::CreateLayoutAreaWindow()
{
	// Create the Static Window
	DWORD dwStyle = WS_CHILD|WS_VISIBLE|SS_NOTIFY|SS_OWNERDRAW;
	HWND hWndParent = getManager()->getWnd()->GetSafeHwnd();
	UINT iControlId = getManager()->getNewControlID();

	HWND hWnd = ::CreateWindow(
		"STATIC",
		"",
		dwStyle,
		m_rctCurrentClientShape.left,
		m_rctCurrentClientShape.top,
		m_rctCurrentClientShape.Width(),
		m_rctCurrentClientShape.Height(),
		hWndParent,
		(HMENU) getManager()->getNewControlID(),
		AfxGetInstanceHandle(),
		NULL
	);
	::SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
	::EnableWindow(hWnd, TRUE);

	SubclassWindow(hWnd);

	// The topmost area usually shouldn't be visible unless it has any such styles.
	if(m_pParent == NULL && !hasStyle((Layout::AreaStyles) (AreaStyleDrawBk|AreaStyleHover|AreaStyleDrawBk|AreaStyleDrawTitle|AreaStyleHoverTitle)))
		ShowWindow(SW_HIDE);
}

///////////////////////////////////
// Layout Area Message Handlers
///////////////////////////////////

/* BOOL Layout::Area::PreTranslateMessage( MSG* pMsg )
{
	if(pMsg == NULL)
		return FALSE;

	if(pMsg->hwnd == this->GetSafeHwnd() && !this->isParentArea())
	{
		switch(pMsg->message)
		{
			case WM_MOUSEMOVE:
			case WM_MOUSELEAVE:
			case WM_ERASEBKGND:
				return FALSE;
		}
	}
	getManager()->getWnd()->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
	return TRUE;
} */

BOOL Area::OnEraseBkgnd( CDC *pDC )
{
	return TRUE;
}

void Area::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	return;
}

void Area::draw( HDC hDC )
{
	if(m_bVisible == false)
		return;

	if(isParentArea())
	{
		m_pHiChild->draw(hDC);
		m_pLoChild->draw(hDC);
	}
	else
	{
		// Draw Background
		if(getCurrentBkColorStyle() != FALSE)
		{
			gdiPlusUtil.drawRect(hDC,
				m_rctCurrentVisibleClientShape.left,
				m_rctCurrentVisibleClientShape.top,
				m_rctCurrentVisibleClientShape.Width(),
				m_rctCurrentVisibleClientShape.Height(),
				getColor(getCurrentBkColorStyle()) );
		}

		// Draw Title
		if(hasStyle(AreaStyleDrawTitle) && !getName().empty())
		{
			COLORREF crText = 0;
			if(m_bHovered && hasStyle(AreaStyleHoverTitle))
				crText = getColor(AreaStyleHoverTitle);
			else
				crText = getColor(AreaStyleDrawTitle);

			CRect rctVisibleClientRect = m_rctCurrentVisibleClientShape;
			rctVisibleClientRect.DeflateRect(_LAYOUT_AREA_TITLEOFFSET, _LAYOUT_AREA_TITLEOFFSET, _LAYOUT_AREA_TITLEOFFSET, _LAYOUT_AREA_TITLEOFFSET);

			gdiPlusUtil.drawString(
				hDC,
				rctVisibleClientRect,
				getName(), crText
			);
		}

		// Draw Left Style Line
		if(hasStyle(AreaStyleDrawLeftLine))
		{
			COLORREF crLine = 0;
			if(m_bHovered && hasStyle(AreaStyleHoverTitle))
				crLine = getColor(AreaStyleHoverTitle);
			else
				crLine = getColor(AreaStyleDrawTitle);

			CRect rctVisibleClientRect = m_rctCurrentVisibleClientShape;
			//rctVisibleClientRect.DeflateRect(0, _LAYOUT_AREA_TITLEOFFSET, 0, _LAYOUT_AREA_TITLEOFFSET);

			gdiPlusUtil.drawRect(
				hDC,
				rctVisibleClientRect.left,
				rctVisibleClientRect.top,
				1,
				rctVisibleClientRect.Height(),
				crLine
			);
		}
	}
}

void Layout::Area::OnMouseLeave()
{
	if(hasStyle(AreaStyleHover))
	{
		POINT ptCursor;
		::GetCursorPos(&ptCursor);
		getManager()->getWnd()->ScreenToClient(&ptCursor);
		// Only register area state as unhovered and redraw if the cursor is
		// really not above the area anymore (Not just over a top control).
		if(m_rctCurrentVisibleClientShape.PtInRect(ptCursor) == FALSE)
		{
			m_bHovered = false;
			getManager()->getWnd()->InvalidateRect(&m_rctCurrentVisibleClientShape, TRUE);
			getManager()->setHoveredArea(NULL);
		}
	}
}

AreaStyles Area::getCurrentBkColorStyle() const
{
	if (m_bHovered && hasStyle(AreaStyleHover))
		return AreaStyleHover;
	else if (hasStyle(AreaStyleDrawBk))
		return AreaStyleDrawBk;
	else
		return (AreaStyles) FALSE;
}

void Area::OnMouseMove( UINT nFlags, CPoint point )
{
	if(hasStyle(AreaStyleHover))
	{
		// The rect will only be redrawn if the hover status was previously false.
		if(m_bHovered == false)
		{
			m_bHovered = true;
			getManager()->getWnd()->InvalidateRect(&m_rctCurrentVisibleClientShape, TRUE);
			getManager()->setHoveredArea(this);
		}

		// The TrackMouseEvent might have been used up by a call to OnMouseLeave
		// when the user hovered above a top level control above the area. That means,
		// the mouse is still in the area, but windows assumes "leave" as not directly above the control.
		// So we might have to generate a new TRACKMOUSEEVENT. We can determine that by querying
		// if there's already one there.
		TRACKMOUSEEVENT tme = {0};
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_QUERY;
		::TrackMouseEvent(&tme);

		if(tme.hwndTrack != m_hWnd || (tme.dwFlags & TME_LEAVE) == 0)
		{
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_hWnd;
			::TrackMouseEvent(&tme);
		}
	}
}

///////////////////////////////////////////
// The rest of the class implementation
///////////////////////////////////////////

Area::Area( Manager const* pMgr, CRect const& rctShape, SIZE const& hMinSize, SIZE const& hMaxSize ) :
	m_hMinSize(hMinSize),
	m_hMaxSize(hMaxSize),
	m_hProcessedMinSize(NULLSIZE),
	m_hFoldedMinSize(FOLDEDSIZE),
	m_hProcessedFoldedMinSize(NULLSIZE),
	m_rctOrigClientShape(0, 0, 0, 0),
	m_rctCurrentShape(0, 0, 0, 0),
	m_rctCurrentClientShape(0, 0, 0, 0),
	m_pAlignmentManager(pMgr),
	m_pParent(NULL),
	m_pHiChild(NULL),
	m_pLoChild(NULL),
	m_pSplitter(NULL),
	m_bHovered(false),
	m_bVisible(true)

{
	setCurrentRect(rctShape);
	updateProcessedFoldedMinSize();
	m_rctOrigClientShape = getClientRect();
	CreateLayoutAreaWindow();
}

Area::Area( Area const* pParent, CRect const& rctShape, SIZE const& hMinSize, SIZE const& hMaxSize ) :
	m_hMinSize(hMinSize),
	m_hMaxSize(hMaxSize),
	m_hProcessedMinSize(NULLSIZE),
	m_hFoldedMinSize(FOLDEDSIZE),
	m_hProcessedFoldedMinSize(NULLSIZE),
	m_rctOrigClientShape(0,0,0,0),
	m_rctCurrentShape(0,0,0,0),
	m_rctCurrentClientShape(0,0,0,0),
	m_pAlignmentManager(pParent->getManager()),
	m_pParent(pParent),
	m_pHiChild(NULL),
	m_pLoChild(NULL),
	m_pSplitter(NULL),
	m_bHovered(false),
	m_bVisible(true)
{
	setCurrentRect(rctShape);
	updateProcessedFoldedMinSize();
	m_rctOrigClientShape = getClientRect();
	CreateLayoutAreaWindow();
}

Area::~Area()
{
	if(isParentArea())
	{
		delete m_pLoChild;
		delete m_pHiChild;
		delete m_pSplitter;
	}
}

bool Area::insertIfOwned( Control* pControl )
{
	bool bReturn = false;

	if(isControlInRect(pControl))
	{
		// Insert the control into this alignment areas control list.
		bReturn = true;
		m_vControls.push_back(pControl);

		if(isParentArea())
		{
			if(!m_pHiChild->insertIfOwned(pControl))
				m_pLoChild->insertIfOwned(pControl);
		}
		else
			pControl->setAlignmentArea(this);
	}

	return bReturn;
}

bool Area::clampRect(UINT nSide, CRect& rctNewrect)
{
	SIZE const& minSize = getMinSize();
	bool bResult = false;

	if(CSize(minSize) != CSize(m_hMaxSize))
	{
		SIZE newSize = rctNewrect.Size();

		if( rctNewrect.Width() < minSize.cx )
		{
			newSize.cx = minSize.cx;
			bResult = true;
		}
		else if( rctNewrect.Width() > m_hMaxSize.cx && minSize.cx <= m_hMaxSize.cx )
		{
			newSize.cx = m_hMaxSize.cx;
			bResult = true;
		}

		if( rctNewrect.Height() < minSize.cy )
		{
			newSize.cy = minSize.cy;
			bResult = true;
		}
		else if( rctNewrect.Height() > m_hMaxSize.cy && minSize.cy <= m_hMaxSize.cy )
		{
			newSize.cy = m_hMaxSize.cy;
			bResult = true;
		}

		if(bResult)
		{
			switch( nSide )
			{
				case WMSZ_LEFT:
				case WMSZ_TOP:
				case WMSZ_TOPLEFT:
				{
					rctNewrect.left = rctNewrect.right - newSize.cx;
					rctNewrect.top = rctNewrect.bottom - newSize.cy;
					break;
				}
				case WMSZ_RIGHT:
				case WMSZ_BOTTOM:
				case WMSZ_BOTTOMRIGHT:
				{
					rctNewrect.right = rctNewrect.left + newSize.cx;
					rctNewrect.bottom = rctNewrect.top + newSize.cy;
					break;
				}
				case WMSZ_TOPRIGHT:
				{
					rctNewrect.right = rctNewrect.left + newSize.cx;
					rctNewrect.top = rctNewrect.bottom - newSize.cy;
					break;
				}
				case WMSZ_BOTTOMLEFT:
				{
					rctNewrect.left = rctNewrect.right - newSize.cx;
					rctNewrect.bottom = rctNewrect.top + newSize.cy;
					break;
				}
			}
		}
	}
	
	return bResult;
}

void Area::update( CRect rctNewrect )
{
	if(rctNewrect == m_rctCurrentShape)
		return;
	
	if (m_hProcessedMinSize.cx == 0 && m_hProcessedMinSize.cy == 0)
		updateMinSize();
	
	// Apply the new size
	resizeAndAutoFoldIfNecessary(rctNewrect, true);
}

bool Area::isParentArea() const
{
	// This is a fundemental assertion. A binary tree can't be randomly linear.
	AFXASSUME((m_pHiChild != NULL) == (m_pLoChild != NULL));
	return m_pHiChild && m_pLoChild;
}

SIZE const& Area::getMinSize() const
{
	// If the area is foldable, its "official" minimum size
	// is the folded size. The processed min size is just for the
	// area to know when it should 'collapse'.
	if(hasStyle(AreaStyleFoldable))
		return m_hProcessedFoldedMinSize;
	else
		return m_hProcessedMinSize;
}

SIZE const& Area::getMaxSize() const
{
	return m_hMaxSize;
}

void Area::resizeAndAutoFoldIfNecessary( __inout CRect& newShape, __in bool bShapeIsScreenCoords )
{
	if(hasStyle(AreaStyleFoldable))
	{
		if(wouldFold(Splitter::Vertical, newShape.Size()))
			fold(Splitter::Vertical);
		else if(wouldFold(Splitter::Horizontal, newShape.Size()))
			fold(Splitter::Horizontal);
		else if(isFolded(Splitter::Horizontal) || isFolded(Splitter::Vertical))
			unfold(newShape);
	}

	if(isFolded(Splitter::Vertical))
		getFoldedShape(Splitter::Vertical, newShape);

	if(isFolded(Splitter::Horizontal))
		getFoldedShape(Splitter::Horizontal, newShape);

	setCurrentRect(newShape, bShapeIsScreenCoords);

	if(isParentArea())
		updateSplitter();
	else
		// To prevent flickering, we only invalidate the background region for the 'bottommost' areas
		::InvalidateRect(getManager()->getWnd()->GetSafeHwnd(), &m_rctCurrentClientShape, TRUE);

	updateControls();
}

void Area::updateSplitter()
{
	// Ensure that this is a parent area with an updatable splitter
	ASSERT(isParentArea());

	// Update the areas current rect to match the dialogs current position
	updateCurrentRect();

	// Update the splitter as an aligned control
	m_pSplitter->update();

	// Update the child areas according to the splitters new position
	updateChildAreas();
}

void Area::updateMinSize() const
{
	using namespace boost::icl;
	
	// Get the min size of the children
	if(isParentArea())
	{
		getChildHi()->updateMinSize();
		getChildLo()->updateMinSize();

		long* minSizeHi = (long*) &(getChildHi()->getMinSize());
		long* minSizeLo = (long*) &(getChildLo()->getMinSize());

		const int iFixedDim = m_pSplitter->getOrientation();
		const int iSplitDim = 1 - iFixedDim;

		assert(iFixedDim >= 0 && iFixedDim <= 1);

		long* plResult = (long*) &m_hProcessedMinSize;
		plResult[iFixedDim] = std::max(minSizeHi[iFixedDim], minSizeLo[iFixedDim]);
		plResult[iSplitDim] = minSizeHi[iSplitDim] + minSizeLo[iSplitDim];
	}
	if (m_hMinSize.cy == 0 || m_hMinSize.cx == 0)
	{
		interval_map<int, MaxDistance> leftOffset, rightInset, topOffset, bottomInset;
		
		// Dialog box margins 7dlu / 11px on all sides
		// Source: https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486%28v=vs.85%29.aspx
		const int defaultPadding = 11;
		
		const CRect& area = getOrigClientRect();
		leftOffset += std::make_pair(interval<int>::right_open(
			area.top, area.bottom), MaxDistance(defaultPadding));
		rightInset += std::make_pair(interval<int>::right_open(
			area.top, area.bottom), MaxDistance(defaultPadding));
		topOffset += std::make_pair(interval<int>::right_open(
			area.left, area.right), MaxDistance(defaultPadding));
		bottomInset += std::make_pair(interval<int>::right_open(
			area.left, area.right), MaxDistance(defaultPadding));
		
		// Calculate the min size of the fixed-size controls. (Controls with high or low alignment)
		// Since Parent areas can also hold controls, it would be more correct to calculate
		// the minimum required size also for those. #MaybeLater
		for each(Control const* pAlignedControl in m_vControls)
		{
			CRect inset(0, 0, 0, 0);
			pAlignedControl->getMinInsets(inset);
			
			CRect controlFrame = pAlignedControl->getOrigRect();
			
			leftOffset += std::make_pair(interval<int>::right_open(
				controlFrame.top, controlFrame.bottom), MaxDistance(inset.left));
			rightInset += std::make_pair(interval<int>::right_open(
				controlFrame.top, controlFrame.bottom), MaxDistance(inset.right));
			topOffset += std::make_pair(interval<int>::right_open(
				controlFrame.left, controlFrame.right), MaxDistance(inset.top));
			bottomInset += std::make_pair(interval<int>::right_open(
				controlFrame.left, controlFrame.right), MaxDistance(inset.bottom));
		}
		
		m_hProcessedMinSize.cx = CalcMinSize(leftOffset, rightInset);
		m_hProcessedMinSize.cy = CalcMinSize(topOffset, bottomInset);
	}

	// Check if a min size is set for this area,
	// and if it is smaller than the child min size.
	if (CSize(m_hMinSize) != CSize(m_hMaxSize))
	{
		if(m_hProcessedMinSize.cx < m_hMinSize.cx)
			m_hProcessedMinSize.cx = m_hMinSize.cx;

		if(m_hProcessedMinSize.cy < m_hMinSize.cy)
			m_hProcessedMinSize.cy = m_hMinSize.cy;
	}
}

void Area::getChildAreaShapes( __out CRect& rctHi, __out CRect& rctLo ) const
{
	if(!m_pSplitter)
		return;

	getChildAreaShapes(rctHi, rctLo, m_pSplitter->getRect());
}

void Area::getChildAreaShapes( __out CRect& rctHi, __out CRect& rctLo, __in CRect const& rctSplitter, __in bool bSplitterRectIsScreenCoords ) const
{
	if(!m_pSplitter)
		return;

	CRect rctSplitterScreenRect = rctSplitter;
	if(bSplitterRectIsScreenCoords == false)
		getManager()->getWnd()->ClientToScreen(rctSplitterScreenRect);

	switch(m_pSplitter->getOrientation())
	{
		case 0: // horizontal splitter
		{
			rctHi.bottom = rctSplitterScreenRect.top;
			rctHi.right = m_rctCurrentShape.right;
			rctLo.top = rctSplitterScreenRect.bottom;
			rctLo.left = m_rctCurrentShape.left;
			break;
		}
		case 1: // vertical splitter
		{
			rctHi.bottom = m_rctCurrentShape.bottom;
			rctHi.right = rctSplitterScreenRect.left;
			rctLo.top = m_rctCurrentShape.top;
			rctLo.left = rctSplitterScreenRect.right;
		}
	}

	rctHi.top = m_rctCurrentShape.top;
	rctHi.left = m_rctCurrentShape.left;
	rctLo.bottom = m_rctCurrentShape.bottom;
	rctLo.right = m_rctCurrentShape.right;
}

void Area::updateControls()
{
	HDWP windowPosHandle = BeginDeferWindowPos(m_vControls.size());
	for each(Control* pControl in m_vControls)
		if (!(windowPosHandle = pControl->update(windowPosHandle)))
			break;
	if (windowPosHandle)
		EndDeferWindowPos(windowPosHandle);
}

bool Area::removeControl( Control* pCtrl )
{
	for( std::vector<Control*>::iterator itCtrl = m_vControls.begin(); itCtrl != m_vControls.end(); ++itCtrl )
		if(*itCtrl == pCtrl)
		{
			m_vControls.erase(itCtrl);
			return true;
		}

	return false;
}

Splitter const* Area::putSplitter( Control const* pHigh, Control const* pLow, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment )
{
	ASSERT(pHigh && pLow && pHigh->getArea() == this && pLow->getArea() == this && !isParentArea());

	if(!pHigh || !pLow || isParentArea() || pHigh->getArea() != this || pLow->getArea() != this)
		return NULL;

	// Create the Splitter
	CRect rctSplitter(0, 0, 0, 0);
	getSplitterRect(nOrientation, rctSplitter, pHigh, pLow);
	m_pSplitter = Splitter::Create(this, rctSplitter, nOrientation, nAlignment);

	// Create the child areas
	CRect rctHi, rctLo;
	getChildAreaShapes(rctHi, rctLo, rctSplitter);
	m_pHiChild = new Area(this, rctHi, NULLSIZE, NULLSIZE);
	m_pLoChild = new Area(this, rctLo, NULLSIZE, NULLSIZE);

	std::list<Control*> lstMovedToChildList;

	// Insert the controls into the child areas
	for each(Control* pCtrl in m_vControls)
	{
		if(!m_pHiChild->insertIfOwned(pCtrl))
			m_pLoChild->insertIfOwned(pCtrl);

		if(pCtrl->getArea() == m_pHiChild || pCtrl->getArea() == m_pLoChild)
			lstMovedToChildList.push_back(pCtrl);
	}

	// Remove all controls from this areas list that have been moved to child lists
	for each(Control* pCtrl in lstMovedToChildList)
		removeControl(pCtrl);

	// Hide this control. Only the bottom level area controls need to be "visible"
	ShowWindow(SW_HIDE);

	return m_pSplitter;
}

void Area::getSplitterRect( Splitter::Orientation nOrientation, __out CRect& rctSplitter, Control const* pHigh, Control const* pLow )
{
	CRect const& rctClientShape = getVisibleClientRect();

	switch(nOrientation)
	{
		case Splitter::Horizontal:
		{
			rctSplitter.left = rctClientShape.left;
			rctSplitter.right = rctClientShape.right;
			rctSplitter.top = pHigh->getRect().bottom;
			rctSplitter.bottom = pLow->getRect().top;

			if (rctSplitter.Height() > 6)
			{
				long offset((rctSplitter.Height() + rctSplitter.Height()%2 - 6)/2);

				rctSplitter.top += offset;
				rctSplitter.bottom -= offset;
			}

			break;
		}
		case Splitter::Vertical:
		{
			rctSplitter.top = rctClientShape.top;
			rctSplitter.bottom = rctClientShape.bottom;
			rctSplitter.left = pHigh->getRect().right;
			rctSplitter.right = pLow->getRect().left;

			if (rctSplitter.Width() > 6)
			{
				long offset((rctSplitter.Width() + rctSplitter.Width()%2 - 6)/2);

				rctSplitter.left += offset;
				rctSplitter.right -= offset;
			}
		}
	}
}

bool Area::isControlInRect( Control const* pCtrl )
{
	CRect const& rctClient = getClientRect();
	return rctClient.PtInRect(pCtrl->getRect().TopLeft()) && rctClient.PtInRect(pCtrl->getRect().BottomRight());
}

bool Area::hasMinMaxSize() const
{
	return m_hProcessedMinSize.cx > 0 || m_hProcessedMinSize.cy > 0;
}

void Area::setCurrentRect( CRect const& rctShape, bool bShapeIsScreenCoords )
{
	AFXASSUME(("No manager assigned (NULL)!", getManager()));
	if (!getManager())
		return;

	m_rctCurrentShape = rctShape;
	m_rctCurrentClientShape = rctShape;
	if(bShapeIsScreenCoords)
		getManager()->getWnd()->ScreenToClient(m_rctCurrentClientShape);
	else
		getManager()->getWnd()->ClientToScreen(m_rctCurrentShape);

	m_rctCurrentVisibleClientShape = getVisibleClientRect();
	m_rctCurrentVisibleShape = m_rctCurrentVisibleClientShape;
	getManager()->getWnd()->ClientToScreen(&m_rctCurrentVisibleShape);

	if(GetSafeHwnd() != NULL)
	{
		SetWindowPos(
			&wndBottom,
			m_rctCurrentClientShape.left,
			m_rctCurrentClientShape.top,
			m_rctCurrentClientShape.Width(),
			m_rctCurrentClientShape.Height(),
			SWP_NOACTIVATE
		);
	}
}

void Area::updateChildAreas()
{
	// Retreive the new child area shapes
	CRect rctHi, rctLo;

	getChildAreaShapes(rctHi, rctLo);

	// Apply the new shapes
	m_pHiChild->resizeAndAutoFoldIfNecessary(rctHi, true);
	m_pLoChild->resizeAndAutoFoldIfNecessary(rctLo, true);
}

void Area::updateCurrentRect()
{
	CRect rctArea = getClientRect();
	setCurrentRect(rctArea, false);
}

void Area::updateOrigRect()
{
	updateCurrentRect();
	m_rctOrigClientShape = m_rctCurrentClientShape;

	if(isParentArea())
	{
		m_pHiChild->updateOrigRect();
		m_pLoChild->updateOrigRect();
		m_pSplitter->updateOrigRect();
	}

	for each(Control* pCtrl in m_vControls)
		pCtrl->updateOrigRect();
}

bool Area::checkSplitterRectWithChildAreaMinSizesAndAutoFold( __inout CRect& rctSplitter ) const
{
	LONG* plSplitterRect = (LONG*) &rctSplitter; // {x1, y1, x2, y2}

	// Check if the given splitter rect would violate the child areas min sizes
	CRect rctHi, rctLo;
	getChildAreaShapes(rctHi, rctLo, rctSplitter, false);

	int iFixedDim = (int) m_pSplitter->getOrientation();
	int iSplitDim = 1 - m_pSplitter->getOrientation();

	SIZE currentSizeLo(rctLo.Size()), currentSizeHi(rctHi.Size());
	LONG const* plLoCurrentSize = (LONG*) &currentSizeLo;
	LONG const* plHiCurrentSize = (LONG*) &currentSizeHi;
	LONG const* plLoMinSize = (LONG*) &(getChildLo()->getMinSize());
	LONG const* plHiMinSize = (LONG*) &(getChildHi()->getMinSize());

	bool bFoldingCheckNotNeededSinceSplitterAlreadyAtLimit = false;

	if(plLoCurrentSize[iSplitDim] < plLoMinSize[iSplitDim])
	{
		int iDiff = plLoMinSize[iSplitDim] - plLoCurrentSize[iSplitDim];
		plSplitterRect[iSplitDim] -= iDiff;
		plSplitterRect[iSplitDim + 2] -= iDiff;

		// The lower area modifying the higher area to satisfy its own min size
		// might lead to the higher area collapsing. Thats why we will only dis-
		// able the folding check if the splitter is already folded for this area.
		// A splitter can not fold towards the high and the low area at the same time.
		bFoldingCheckNotNeededSinceSplitterAlreadyAtLimit = getChildLo()->hasStyle(AreaStyleFoldable);
	}
	else if(plHiCurrentSize[iSplitDim] < plHiMinSize[iSplitDim])
	{
		int iDiff = plHiMinSize[iSplitDim] - plHiCurrentSize[iSplitDim];
		plSplitterRect[iSplitDim] += iDiff;
		plSplitterRect[iSplitDim + 2] += iDiff;

		// The higher area modifying the lower area to satisfy its own min size
		// might lead to the lower area collapsing. Thats why we will only dis-
		// able the folding check if the splitter is already folded for this area.
		// A splitter can not fold towards the high and the low area at the same time.
		bFoldingCheckNotNeededSinceSplitterAlreadyAtLimit = getChildHi()->hasStyle(AreaStyleFoldable);
	}

	bool bResult = bFoldingCheckNotNeededSinceSplitterAlreadyAtLimit;

	// If the user let go of the splitter, and one of the areas collapsed or would collapse,
	// then we will pull the splitter to the edge of the collapsed area
	if(getSplitter()->isDraggedByUser() == false && bFoldingCheckNotNeededSinceSplitterAlreadyAtLimit == false)
	{
		if(
			getChildHi()->isFolded(getSplitter()->getOrientation()) ||
			getChildHi()->wouldFold(getSplitter()->getOrientation(), currentSizeHi)
		)
		{
			int iDiff = plHiCurrentSize[iSplitDim] - plHiMinSize[iSplitDim];
			plSplitterRect[iSplitDim] -= iDiff;
			plSplitterRect[iSplitDim + 2] -= iDiff;
			bResult = true;
		}
		else if(
			getChildLo()->isFolded(getSplitter()->getOrientation()) ||
			getChildLo()->wouldFold(getSplitter()->getOrientation(), currentSizeLo)
		)
		{
			int iDiff = plLoCurrentSize[iSplitDim] - plLoMinSize[iSplitDim];
			plSplitterRect[iSplitDim] += iDiff;
			plSplitterRect[iSplitDim + 2] += iDiff;
			bResult = true;
		}
	}
	return bResult;
}

void Area::fold(Splitter::Orientation nOrientation)
{
	if(!m_mapDimensionFolded[nOrientation])
	{
		m_mapDimensionFolded[nOrientation] = true;

		getFoldedShape(nOrientation, m_rctCurrentShape);

		// Hide the controls
		for each(Control* pControl in m_vControls)
			pControl->temporaryHide();

		if(isParentArea())
			m_pSplitter->temporaryHide();

		resizeAndAutoFoldIfNecessary(m_rctCurrentShape);
	}
}

void Area::unfold(__in CRect const& rctDesiredUnfoldedShape)
{
	// Mark as unfolded
	m_mapDimensionFolded[Splitter::Horizontal] = false;
	m_mapDimensionFolded[Splitter::Vertical] = false;

	// Show the controls
	for each(Control* pControl in m_vControls)
		pControl->temporaryShow();

	if(isParentArea())
		m_pSplitter->temporaryShow();

	resizeAndAutoFoldIfNecessary(CRect(rctDesiredUnfoldedShape));
}

bool Area::wouldFold( Splitter::Orientation nDim, SIZE szTestSize ) const
{
	if(!hasStyle(AreaStyleFoldable))
		return false;

	if(nDim == Splitter::Vertical && szTestSize.cx < m_hProcessedMinSize.cx)
		return true;

	if(nDim == Splitter::Horizontal && szTestSize.cy < m_hProcessedMinSize.cy)
		return true;

	return false;
}

void Area::updateProcessedFoldedMinSize()
{
	m_hProcessedFoldedMinSize = m_hFoldedMinSize;

	int iDiff = m_rctCurrentClientShape.Width() - m_rctCurrentVisibleClientShape.Width();
	if(iDiff > 0)
		m_hProcessedFoldedMinSize.cx += iDiff;

	iDiff = m_rctCurrentClientShape.Height() - m_rctCurrentVisibleClientShape.Height();
	if(iDiff > 0)
		m_hProcessedFoldedMinSize.cy += iDiff;
}

bool Area::isFolded( Splitter::Orientation nDim ) const
{
	return m_mapDimensionFolded[nDim];
}

void Area::setVisible( bool bShow )
{
	if(bShow)
	{
		// Hide Controls if not collapsed already
		if(!isFolded(Splitter::Horizontal) && !isFolded(Splitter::Vertical))
			for each(Control* pControl in m_vControls)
				pControl->temporaryHide();
	}
	else
	{
		// Show controls if not hidden due to collapse
		if(!isFolded(Splitter::Horizontal) && !isFolded(Splitter::Vertical))
			for each(Control* pControl in m_vControls)
				pControl->temporaryShow();
	}

	m_bVisible = bShow;

	// Propagate visiblity to subareas
	if(isParentArea())
	{
		m_pHiChild->setVisible(bShow);
		m_pLoChild->setVisible(bShow);
		m_pSplitter->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
}

bool Area::isLoArea() const
{
	return !isHiArea();
}

bool Area::isLoArea(Splitter::Orientation nOrientation) const
{
	return !isHiArea(nOrientation);
}

bool Area::isHiArea() const
{
	if(!getParent())
		return true;

	return getParent()->getChildHi() == this;
}

bool Area::isHiArea(Splitter::Orientation nOrientation) const
{
	if(!getParent())
		return true;

	if(getParent()->getSplitter()->getOrientation() == nOrientation)
		return isHiArea();
	else
		return getParent()->isHiArea(nOrientation);
}

void Area::getFoldedShape( __in Splitter::Orientation nOrientation, __inout CRect& rctShape )
{
	if(nOrientation == Splitter::Horizontal)
	{
		if(isHiArea(nOrientation))
			rctShape.bottom = rctShape.top + m_hProcessedFoldedMinSize.cy;
		else
			rctShape.top = rctShape.bottom - m_hProcessedFoldedMinSize.cy;
	}
	else
	{
		if(isHiArea(nOrientation))
			rctShape.right = rctShape.left + m_hProcessedFoldedMinSize.cx;
		else
			rctShape.left = rctShape.right - m_hProcessedFoldedMinSize.cx;
	}
}

Area* Layout::Area::isBackgroundHwnd(HWND hCtrl)
{
	if(hCtrl == GetSafeHwnd())
		return this;

	if(isParentArea())
	{
		Area* pResult = m_pHiChild->isBackgroundHwnd(hCtrl);
		if(pResult != NULL)
			return pResult;
		else if(pResult = m_pLoChild->isBackgroundHwnd(hCtrl))
			return pResult;
	}

	return NULL;
}
