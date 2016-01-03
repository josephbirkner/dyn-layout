#ifndef _LAYOUT_WINDOW_
#define _LAYOUT_WINDOW_

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

#include "control.h"
#include "area.h"

namespace Layout
{
	class Owner;

	class Window : public Area
	{
		DECLARE_DYNAMIC(Window)
		
		friend class Area;
		friend class Splitter;
		friend class Manager;

	private:
		Window(Owner const& windowedLayout, Area const* pParent, CRect const& rctShape, SIZE const& hMinSize, SIZE const& hMaxSize);
		Window(Owner const& windowedLayout, Manager const* pMgr, CRect const& rctShape, SIZE const& hMinSize, SIZE const& hMaxSize);
		virtual ~Window();
		
		/** Calls the parent impl. Sets the new Size on the child layout. */
		virtual void setCurrentRect(CRect const& rctShape, bool bShapeIsScreenCoords = true);
		
		/** Initializes the layout Window. Called in the constructors. */
		void initWindow();
		
	public:
	
	protected:
		DECLARE_MESSAGE_MAP()
		
		//virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		//afx_msg BOOL OnEraseBkgnd(CDC *pDC);
		
	private:
		Owner const& m_aChildLayout; /// The layout that is visible through this window
	};
}

#endif // _LAYOUT_WINDOW_