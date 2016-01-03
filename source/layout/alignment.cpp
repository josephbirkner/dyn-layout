#include "stdafx.h"

#pragma hdrstop

#include "../../GlobExport/alignment.h"
#include "../../GlobExport/area.h"
#include "../../GlobExport/manager.h"

using namespace Layout;

/**
 * The control will stick to the parents top/left edge, the width will be constant.
 */
void Align::TopLeft::update( Control* pCtrl, Align::Dimension nDim, CRect& rctResult )
{
	CRect const& rctAreaOrig = pCtrl->getArea()->getOrigClientRect();
	CRect const& rctArea = pCtrl->getArea()->getClientRect();
	
	if( nDim == Horizontal )
	{
		int dx = pCtrl->getOrigRect().left - rctAreaOrig.left;
		rctResult.MoveToX(rctArea.left + dx);
	}
	else
	{
		int dy = pCtrl->getOrigRect().top - rctAreaOrig.top;
		rctResult.MoveToY(rctArea.top + dy);
	}
}

void Align::TopLeft::getMinInsets(__in Control const* pCtrl, __in Dimension nDimension, CRect& bounds)
{
	CRect const& rctCtrlOrig = pCtrl->getOrigRect();
	CRect const& rctAreaOrig = pCtrl->getArea()->getOrigClientRect();
	
	if(nDimension == Horizontal)
		bounds.left = rctCtrlOrig.right - rctAreaOrig.left;
	else
		bounds.top = rctCtrlOrig.bottom - rctAreaOrig.top;
}

//Align::Resize::Resize(int iMinSize /*= 10*/)
//	: m_iMinSize(iMinSize)
//{}

/**
 * The control will resize, so that the distances to
 * the parents left/top and right/bottom edge are always constant.
 */
void Align::Resize::update( Control* pCtrl, Align::Dimension nDim, CRect& rctResult )
{
	int iDiff = 0;
	CRect const& rctAreaOrig = pCtrl->getArea()->getOrigClientRect();
	CRect const& rctArea = pCtrl->getArea()->getClientRect();
	
	if( nDim == Horizontal )
	{
		iDiff = rctAreaOrig.right - pCtrl->getOrigRect().right;
		rctResult.right = rctArea.right - iDiff;
		iDiff = rctAreaOrig.left - pCtrl->getOrigRect().left;
		rctResult.left = rctArea.left - iDiff;
	}
	else
	{
		iDiff = rctAreaOrig.bottom - pCtrl->getOrigRect().bottom;
		rctResult.bottom = rctArea.bottom - iDiff;
		iDiff = rctAreaOrig.top - pCtrl->getOrigRect().top;
		rctResult.top = rctArea.top - iDiff;
	}
}

void Align::Resize::getMinInsets(__in Control const* pCtrl, __in Dimension nDimension, CRect& bounds)
{
	CRect const& rctCtrlOrig = pCtrl->getOrigRect();
	CRect const& rctAreaOrig = pCtrl->getArea()->getOrigClientRect();
	
	if( nDimension == Horizontal ) {
		bounds.left = rctCtrlOrig.left - rctAreaOrig.left + m_iMinSize/2;
		bounds.right = rctAreaOrig.right - rctCtrlOrig.right + m_iMinSize/2;
	}
	else {
		bounds.top = rctCtrlOrig.top - rctAreaOrig.top + m_iMinSize/2;
		bounds.bottom = rctAreaOrig.bottom - rctCtrlOrig.bottom + m_iMinSize/2;
	}
}

/** 
 * The control will resize, so that the bottom/right distance
 * always equals the top/left distance to the corresponding parent edge
 */
void Align::Fit::update( Control* pCtrl, Align::Dimension nDim, CRect& rctResult )
{
	CRect const& rctAreaOrig = pCtrl->getArea()->getOrigClientRect();
	CRect const& rctArea = pCtrl->getArea()->getClientRect();
	
	if( nDim == Horizontal )
		rctResult.right = rctArea.right - (pCtrl->getOrigRect().left - rctAreaOrig.left);
	else
		rctResult.bottom = rctArea.bottom - (pCtrl->getOrigRect().top - rctAreaOrig.top);
}

/**
 * The control will stick to the parents right/bottom edge,
 * the width/height will be constant.
 */
void Align::BottomRight::update( Control* pCtrl, Align::Dimension nDim, CRect& rctResult )
{
	CRect const& rctAreaOrig = pCtrl->getArea()->getOrigClientRect();
	CRect const& rctArea = pCtrl->getArea()->getClientRect();
	
	if( nDim == Horizontal )
	{
		int dx = rctAreaOrig.right - pCtrl->getOrigRect().left;
		rctResult.MoveToX(rctArea.right - dx);
	}
	else
	{
		int dy = rctAreaOrig.bottom - pCtrl->getOrigRect().top;
		rctResult.MoveToY(rctArea.bottom - dy);
	}
}

void Align::BottomRight::getMinInsets(__in Control const* pCtrl, __in Dimension nDimension, CRect& bounds)
{
	CRect const& rctCtrlOrig = pCtrl->getOrigRect();
	CRect const& rctAreaOrig = pCtrl->getArea()->getOrigClientRect();
	
	if(nDimension == Horizontal)	
		bounds.right = rctAreaOrig.right - rctCtrlOrig.left;
	else
		bounds.bottom = rctAreaOrig.bottom - rctCtrlOrig.top;
}

/**
 * Create a new Align::Relative specifying a resize mode.
 */
Align::Relative::Relative(bool bResize)
	: Align::Mode()
	, m_bResize(bResize)
{
}

/**
 * The controls position will be scaled with the dialog.
 * If m_bResize is true, then the controls size will be
 * scaled as well.
 */
void Align::Relative::update( Control* pCtrl, Align::Dimension nDim, CRect& rctResult )
{
	CRect const& rctAreaOrig = pCtrl->getArea()->getOrigClientRect();
	CRect const& rctArea = pCtrl->getArea()->getClientRect();
	
	if( nDim == Horizontal )
	{
		double dFactorX = (double)(rctArea.Width()) / (double)(rctAreaOrig.Width());
		
		if( m_bResize )
		{
			rctResult.right = (LONG)((pCtrl->getOrigRect().right - rctAreaOrig.left) * dFactorX) + rctArea.left;
			rctResult.left = (LONG)((pCtrl->getOrigRect().left - rctAreaOrig.left) * dFactorX) + rctArea.left;
		}
		else
			rctResult.MoveToX((LONG)((pCtrl->getOrigRect().left - rctAreaOrig.left) * dFactorX) + rctArea.left);
	}
	else
	{
		double dFactorY = (double)(rctArea.Height()) / (double)(rctAreaOrig.Height());
		
		if( m_bResize )
		{
			rctResult.bottom = (LONG)((pCtrl->getOrigRect().bottom - rctAreaOrig.top) * dFactorY) + rctArea.top;
			rctResult.top = (LONG)((pCtrl->getOrigRect().top - rctAreaOrig.top) * dFactorY) + rctArea.top;
		}
		else
			rctResult.MoveToY((LONG)((pCtrl->getOrigRect().top - rctAreaOrig.top) * dFactorY) + rctArea.top);
	}
}
