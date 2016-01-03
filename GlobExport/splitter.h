#ifndef _LAYOUT_SPLITTER_
#define _LAYOUT_SPLITTER_

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

#include "control.h"

namespace Layout
{
	class Splitter : public CStatic, public Control
	{
		DECLARE_DYNAMIC(Splitter)
		friend class Area;
		
	public:
		enum SplitterAlignment
		{
			AlignHigh,
			AlignLow,
			AlignRelative
		};
		
		enum Orientation
		{
			Horizontal,
			Vertical
		};

	private:
		static Splitter* Create(Area const* pArea, CRect rctSplitter, Orientation nOrientation, SplitterAlignment nAlignment);
		Splitter(Area const* pArea, HWND hWnd, Orientation nOrientation, SplitterAlignment nAlignment );
		virtual ~Splitter();
		
	public:
		/** Delivers the alignment of the splitter. */
		LAYOUT_API SplitterAlignment getAlignment() const {return m_nAlignment;}
		
		/** Delivers the orientation of the splitter. */
		LAYOUT_API Orientation getOrientation() const {return m_nOrientation;}
		
		/** Moves the splitter horizontally OR vertically, depending on its orientation.  */
		LAYOUT_API bool move(int iHorzPos, int iVertPos);
		
		/** Set the color used to draw the handle dots.  */
		LAYOUT_API void setHandleColor(COLORREF hColor);
		
		/** Overloaded update method of the splitter.
			Will incorporate the effected areas min sizes into the alignment calculations. */
		virtual LAYOUT_API void update();
		
		/** Returns if the user is currently dragging the splitter. */
		virtual LAYOUT_API bool isDraggedByUser() const {return m_bDragging;}
		
	protected:
		DECLARE_MESSAGE_MAP()
		
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC) {return TRUE;}
		afx_msg virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		
	private:
		SplitterAlignment m_nAlignment;
		Orientation m_nOrientation;
		bool m_bDragging;
		COLORREF m_hHandleColor;
	};
}

#endif // _LAYOUT_SPLITTER_