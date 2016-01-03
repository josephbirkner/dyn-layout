#include "StdAfx.h"
#pragma hdrstop

#include "../../GlobExport/geometry.h"

using namespace Layout;

/**
 * Returns a Boolean value that indicates whether 
 * two rectangles intersect
 */
bool Layout::RectIntersectsRect(CRect const& lh, CRect const& rh)
{
	return (lh.left < rh.right && lh.right > rh.left) &&
	       (lh.top < rh.bottom && lh.bottom > rh.top);
}

/**
 * Helper function that returns the intersection rect of two rects
 */
CRect Layout::RectGetIntersectionRect(CRect const& lh, CRect const& rh)
{
	CRect nullRect(0, 0, 0, 0);
	
	long maxL = std::max(lh.left, rh.left);
	long minR = std::min(lh.right, rh.right);
	
	long maxT = std::max(lh.top, rh.top);
	long minB = std::min(lh.bottom, rh.bottom);
	
	CRect intersection(maxL, maxT, minR, minB);
	
	if (minR - maxL > 0 && minB - maxT > 0)
		return intersection;
	
	return nullRect;
}