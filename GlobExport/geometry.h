#ifndef _LAYOUT_GEOMETRY_
#define _LAYOUT_GEOMETRY_

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

class CRect;

namespace Layout
{
	/**
	 * Returns a Boolean value that indicates whether 
	 * two rectangles intersect.
	 */
	bool LAYOUT_API RectIntersectsRect(CRect const&, CRect const&);

	/**
	 * Returns the intersection of two rects or an empty rect.
	 */
	CRect LAYOUT_API RectGetIntersectionRect(CRect const&, CRect const&);
}

#endif