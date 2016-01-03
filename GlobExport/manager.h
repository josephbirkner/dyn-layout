#ifndef _LAYOUT_MANAGER_
#define _LAYOUT_MANAGER_

#pragma once

#include <map>
#include <set>

#include "alignment.h"
#include "area.h"
#include "window.h"
#include "areacreateparams.h"

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

namespace Layout
{
	static SIZE const NULLSIZE = {0, 0};

	class Control;
	class Editor;
	
	/**
	 * Profiling modes for the Manager. Profiling means, that the managed windows
	 * - position/size
	 * - splitter positions
	 * are automatically recorded in the windows registry, and will be automatically restored.
	 */
	enum ProfilingMode
	{
		ProfileOff,
		ProfileApplicationSpecific,
		ProfileGlobal
	};
		
	/**
	 * Alignment manager class to control the behaviour of a dialog's controls on dialog resizing.
	 * Also provides functionality to control a dialog's min/max size.
	 *
	 * Use:
	 * - You can propably save yourself a lot of pain by using the Layout::Owner class instead.
	 *   However, if what you need the manager for is not a dialog, follow these steps:
	 *   - Add a field "Manager* m_pAlignmentManager;" to your CWnd subclass
	 *   - In the initialization section, create a new Manager and assign it to m_pAlignmentManager.
	 *     Here you can also specify minimum and maximum sizes for your dialog, that will be enforced in the update() method.
	 *   - Still in OnInitDialog, use the addControl() method to set specific controls with specific alignments,
	 *     to set up your dialog's alignment layout.
	 *   - In the OnSizing (!OnSize) method of your CWnd subclass, call "m_pAlignmentManager->update(nSide, lpRect);"
	 *   - In the destructor of your whatever, don't forget to call "delete m_pAlignmentManager;"
	 */
	class Manager
	{
		friend class Splitter;
		friend class Owner;
		friend class Area;
		friend class Window;
		friend class Editor;
		friend LRESULT ManagedLayoutWindowProc(_In_ int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);

	public:
		/**
		 * Alignment Manager Ctor.
		 * @param hParent The window whose children are to be aligned
		 * @param hMinSize [optional] The minimum size of the dialog. Must be smaller than the maximum size if both are specified. Should be in pixels.
		 * @param hMaxSize [optional] An arbitrary name that will identify the dialog in the profiling registry.
		 * @param sProfileIdentifier
		 * @param nProfileMode
		 */
		LAYOUT_API Manager( HWND hParent, const SIZE& hMinSize = NULLSIZE, const SIZE& hMaxSize = NULLSIZE );
		LAYOUT_API Manager( HWND hParent, std::string sLayoutIdentifier, ULONG nProfileMode, const SIZE& hMinSize = NULLSIZE, const SIZE& hMaxSize = NULLSIZE );
		
		/**
		 * Alignment Manager Dtor.
		 */
		LAYOUT_API ~Manager();
		
		/**
		 * Add a control with a specific alignment to be enforced by this manager.
		 * @param hCtrl The control to be added
		 * @param hAlignHorz The horizontal alignment of the control to be added.
		 * @param hAlignVert The vertical alignment of the control to be added.
		 * @param sName The name for the control. This will be needed in the dynamic dialog system
		 * @return True, if the control was successfully added, false if adding failed.
		 *         Adding the control will fail if the control is either already added, or the control's parent is not
		 *         the window this manager has been created for. Also, the designated hWnd must not belong to any of the
		 *         Managers Alignment Areas.
		 * @about If the control has already been added to the manager previously,
		 *        the controls current rect will be seen as its original one!
		 */
		LAYOUT_API bool addControl( HWND hCtrl, Align::Mode& hAlignHorz, Align::Mode& hAlignVert, std::string sName );
		LAYOUT_API bool addControl( UINT nID, Align::Mode& hAlignHorz, Align::Mode& hAlignVert, std::string sName );
		
		/**
		 * Put a splitter between two controls in the dialog.
		 * The controls must not have been separated by a splitter before.
		 * Also, both controls must have been added to the manager.
		 * @param pHigherCtrl The control above or left to the splitter.
		 * @param pLowerCtrl The control below or right to the splitter.
		 * @param nHigher The ID of the control above or left to the splitter.
		 * @param nLower The ID of the control below or right to the splitter.
		 * @param aHighAreaParams The params supposed to be used to create the resulting higher area. See AreaCreateParams.
		 * @param aLowAreaParams The params supposed to be used to create the resulting lower area. See AreaCreateParams.
		 * @param nOrientation One of Splitter::Orientation.
		 * @param nAlignment One of Splitter::SplitterAlignment. This parameter describes
		 *        how the splitters position is to be changed if the Dialog resizes.
		 *        - Splitter::AlignHigh: The Splitter will stick to the top/left edge.
		 *        - Splitter::AlignLow: The Splitter will stick to the bottom/right edge.
		 *        - Splitter::AlignRelative: The Splitter will maintain proportions between its top/left and bottom/right area.
		 * @return The created splitter (for reference purposes, you propably won't have to touch it).
		 *         Or null if creating the splitter failed.
		 */
		LAYOUT_API Splitter const* putSplitter(HWND hHigherCtrl, HWND hLowerCtrl, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment);
		LAYOUT_API Splitter const* putSplitter(UINT nHigherId, UINT nLowerId, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment);
		LAYOUT_API Splitter const* putSplitter(AreaProperties& aHighAreaParams, AreaProperties& aLowAreaParams, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment);
		
		/**
		 * Remove a control whose alignment is not be enforced by this manager anymore.
		 * @param hCtrl The control to be removed.
		 * @return True, if the control was successfully removed, false if otherwise.
		 * Removing will fail if the control was not added.
		 */
		LAYOUT_API bool removeControl( HWND hCtrl );
		
		/**
		 * Get the Alignment of a control that supposedly has been added to the manager.
		 * @param hAlignHorz [out]
		 * @param hAlignVert [out]
		 * @return True, if the control was found, false if otherwise.
		 */
		LAYOUT_API bool getControlAlignment( CWnd*, Align::Mode& hAlignHorz, Align::Mode& hAlignVert );
		
		/**
		 * Get the dialog window this manager has been created for
		 * @return The dialog window pointer passed in the ctor.
		 */
		LAYOUT_API CWnd* getWnd() const { return CWnd::FromHandle(m_hManagedWindow); }
		
		/**
		 * Get the HWND this manager has been created for.
		 * @return HWND
		 */
		LAYOUT_API HWND getHwnd() const { return m_hManagedWindow; };
		
		/**
		 * Get the current size of the window this manager has been created for
		 * @return A reference to the window size member
		 */
		LAYOUT_API SIZE getWindowSize() const { return m_pMainArea->getSize(); }
		
		/**
		 * Get the original size of the window this manager has been created for
		 * @return A reference to the original window size member
		 */
		LAYOUT_API SIZE getOrigWindowSize() const { return m_pMainArea->getOrigSize(); }
		
		/**
		 * Set the base from which control ids for new splitters are allocated.
		 * @param nBase The new base from which control ids are to be allocated.
		 *              Must not be used by a control in your dialog yet.
		 *              Must be the beginning of a sufficiently big enough range of
		 *              unused control ids.
		 */
		LAYOUT_API void setControlIdAllocBase( UINT nBase ) { m_nNextControlID = nBase; }
		
		/**
		 * Obtain the identifier under which profiling entries for the window
		 * managed by this Manager are written. The identifier is issued in
		 * the constructor.
		 */
		LAYOUT_API std::string const& getIdentifier() const { return m_sLayoutIdentifier; }
		
		/**
		 * Obtain the text metric of the window managed by this manager.
		 */
		LAYOUT_API TEXTMETRIC const& getTextMetric() const {return m_textMetric;}
		
		/**
		 * Set the layout temporarily to show the content of a different window
		 * (a different layout owner). The same layout can not host two modal pages at the same time,
		 * but a modal page can host another modal page.
		 * 
		 * The layout will stop hosting the modal page when the modal page's window receives a WM_DESTROY.
		 */
		LAYOUT_API void putModalPage(Layout::Owner const* pPage);
		
		/** Get the modal page set through putModalPage, or Null, if not set. */
		LAYOUT_API Window const* getModalPage() {return m_pModalPage;}
		
		/** Called by the window routine to open the editor */
		LAYOUT_API void openEditor();
		
		/** Restores any shapes stored in the registry for the dialog managed by this manager.
		    All Layout initialization should be performed before this method is called. */
		LAYOUT_API void restoreFromProfile();
		
		/**
		 * Enforce the alignment of all registered with manager.
		 * This method will be called automatically by the window hook installed by the manager.
		 */
		LAYOUT_API void update();
		
		/**
		 * Clamps the given rect between the root areas min and max size.
		 * @param nSide  Sides that should be adjusted
		 * @param lpRect Rect to clamp
		 */
		LAYOUT_API void clampRect(UINT nSide, LPRECT lpRect);

		/** Updates the original rects of all controls registered with the manager.
		    This has become a necessity when adding a control AFTER the dialog has been resized. */
		LAYOUT_API void updateAllOrigRect() const;

		/**  */
		LAYOUT_API void closeModalPageWindow();
		
		/** Delivers a new control id and increments the control id alloc base. */
		LAYOUT_API UINT getNewControlID() const {return m_nNextControlID++;} /// Used by the splitter creation routine

	private:
		mutable HWND m_hManagedWindow; /// The window this manager has been created for.
		mutable Area* m_pMainArea;     /// The main alignment area
		mutable UINT m_nNextControlID; /// The next control id to be assigned to a new splitter
		mutable Area const* m_pHoveredArea; /// The currently hovered area
		
		Window* m_pModalPage; /// The modal page, if set. @see putModalPage()
		
		std::map<HWND, Control*> m_mapHwndControl; /// A map pointing from a specific HWND to a specific Control pointer
		std::string m_sLayoutIdentifier; /// The identifier of this layout owner in the profile
		ProfilingMode m_nProfilingMode; /// The current profiling mode of the manager
		std::string m_sProfilingPath; /// The complete profile path where settings for this layout owner are stored
		
		TEXTMETRIC m_textMetric; /// The text metric of the managers window
		Editor* m_pEditor;

		/** Called by the a newly hovered area to inform an eventual previous
		    hovered area, that did not notice the mouse leaving,
		    that it does not own the mouse anymore. */
		void setHoveredArea(Area* pArea) const;
		
		/** Called by the ctors */
		void initMgr( HWND hParent, const SIZE& hMaxSize, const SIZE& hMinSize );
		void initProfiling();

		/** Returns a control for a specific hwnd */
		Control const* getControl(HWND hCtrl);
		
		/** Returns a set of all the controls. */
		void getControls(__out std::set<Control const*>& aControlSet) const;
		
		/** Called in WM_ERASEBKND from ManagedLayoutWindowProc. Draws the areas into the background. */
		void draw(HDC hDC);
		
		/** Helper function called in OnMove and OnSize */
		void storeSizeAndPosition();	
		
		/** Converts both of the given sizes from logical to physical units. */
		void mapDialogSizesLogicalToPhysical( __inout SIZE& hMaxSize, __inout SIZE& hMinSize );
		
		/** Managed Layout Hook Infrastructure */
		void pushToManagedMap();
		void eraseFromManagedMap();
		
		WNDPROC m_pSuperWndProc;
		WNDPROC getSuperWndProc() {return m_pSuperWndProc;}
		
		static LRESULT CALLBACK ManagedLayoutWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
		static std::map<HWND, Manager*> s_mapWndLayoutManager;
		static HANDLE s_hManagerMapAccess;
		static HHOOK s_hWindowHook; /// A window hook to intercept and manipulate messages sent to the managed window.
		                            /// This applies to WM_SIZING and WM_MOVE.
	}; // Manager

}

#endif // _LAYOUT_MANAGER_