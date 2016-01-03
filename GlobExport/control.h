#ifndef _LAYOUT_CONTROL_
#define _LAYOUT_CONTROL_

#pragma once

#include <map>

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

class LayoutTest;

namespace Layout
{
	namespace Align
	{
		class Mode;
	}
	
	class Manager;
	class Area;
	class Editor;

	/**
	 * Helper class, instanced for each control added to the Manager.
	 * Defined in alignmgr.cpp
	 */
	class Control
	{
		friend class Manager;
		friend class LayoutTest;
		friend class Area;
		friend class Editor;
	
	protected:
		static std::map<HWND, Control*> s_mapControlForHwnd;
		static LRESULT CALLBACK EditorWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
		static LRESULT CALLBACK NormalWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
		
		HWND m_hID;                        /// The window handle of the represented CWnd
		CRect m_rctOrig;                   /// The control's rect when it was added to the manager
		mutable CRect m_rctCurrent;        /// The control's current rect
		mutable Align::Mode* m_pHorzAlign; /// The controls Horiz. Alignment Mode
		mutable Align::Mode* m_pVertAlign; /// The controls Vert. Alignment Mode
		Manager const* m_pManager;         /// The Alignment manager this instance belongs to
		const Area* m_pAlignmentArea;      /// The Alignment Area this control belongs to
		bool m_bVisibilityBeforeTempHide; /// Tells if the conrol was visible before temporaryHide was called
		std::string m_sName; /// The unique string identifier of the control. Can currently be Null.
		mutable WNDPROC m_pWndProc;
		
		/** CTor. Create a new Aligned Control instance for a specific manager and window pointer, with specific horizontal
			and vertical alignments. */
		LAYOUT_API Control( Manager const*, HWND hCtrl, Align::Mode&, Align::Mode&, std::string sName = "" );
		
		/** DTor. */
		virtual ~Control();
		
		/** Hides the control temporarily and remembers if the control was shown before the temporaryHide() */
		virtual void temporaryHide();
		
		/** Hides the control temporarily and remembers if the control was shown before the temporaryHide() */
		virtual void temporaryShow();
		
		/** Called by the editor to install or uninstall the "editor mode" on the control */
		virtual void installEditorWndProc(bool bInstall) const;
		
	public:
		/** Returns the original window proc of the control. */
		LAYOUT_API WNDPROC getWndProc() {return m_pWndProc;}
		
		/** Get the string identifier of the control */
		LAYOUT_API std::string getName() const {return m_sName;}
	
		/** Get a pointer to the manager this instance belongs to. */
		LAYOUT_API Manager const* getManager() const;
		
		/** Get the alignment area this control belongs to. */
		LAYOUT_API const Area* getArea() const;
		
		/** Set the alignment area this control belongs to. */
		LAYOUT_API void setAlignmentArea(Area const* pArea);

		/** Get a pointer to the control this Control instance has been created for. */
		LAYOUT_API CWnd* getControl() const { return CWnd::FromHandle(m_hID); }

		/** Get a reference to the original rect of the control (It's rect when it was added to the manager) */
		LAYOUT_API CRect const& getOrigRect() const { return m_rctOrig; }
		
		/** Get the controls current screen rect */
		LAYOUT_API void getScreenRect(CRect& rctResult) const;

		/** Get this Control's horizontal alignment mode. */
		LAYOUT_API Align::Mode* getHorzAlignment(){ return m_pHorzAlign; }

		/** Get this Control's vertical alignment mode. */
		LAYOUT_API Align::Mode* getVertAlignment(){ return m_pVertAlign; }
		
		/** Get this Control's horizontal alignment mode. */
		LAYOUT_API void setHorzAlignment( Align::Mode* pAlignHorz );

		/** Get this Control's vertical alignment mode. */
		LAYOUT_API void setVertAlignment( Align::Mode* pAlignVert );

		/** Get this Control's window's current rect. */
		LAYOUT_API CRect const& getRect() const;

		/** Update (enforce) this Control's window's alignment according to the specified modes. */
		LAYOUT_API virtual HDWP update(HDWP windowPosHandle);
		
		/** Update this Control's orig rect to the current window rect of the window it represents */
		LAYOUT_API void updateOrigRect();
		
		/** Delivers the minimum amount of space necessary for the controls alignment,
			from the top (to bottom), from the bottom (to top), from the left (to right),
			and from the right (to left) parent area edge.
			THE MINIMUM INSET WILL BE ANTICIPATED AS IRRELEVANT FOR INVISIBLE CONTROLS. */
		LAYOUT_API void getMinInsets(__out CRect& insets) const;
	}; // Control
}

	
#endif // _LAYOUT_CONTROL_