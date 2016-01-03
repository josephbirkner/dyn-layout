#ifndef _LAYOUT_OWNER_
#define _LAYOUT_OWNER_

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

#include "manager.h"
#include "splitter.h"

namespace Layout
{
	class Owner
	{
	private:
		Manager* m_pManager; /// This alignment manager holder's alignment manager
		                              /// Creation is omitted to any extending class!
		                              /// Use setAlignmentManager() to set this field.
		bool     m_bInitializing;     /// Tells if calls to setLayout() are currently allowed
		std::string m_sLayoutName;    /// A unique name identifzyng profile entries for this layout owner
		ProfilingMode m_nProfilingMode; /// The profiling mode
		SIZE m_sizeMin, m_sizeMax;    /// Minimum and maximum sizes for the layout owned by this layout owner
		
		void callDataExchange();
		
	public:
		/// Standard ctor, will set m_pAlignmentManager to NULL
		LAYOUT_API Owner(std::string sLayoutOwnerName, ProfilingMode nProfilingMode = ProfileApplicationSpecific, SIZE sizeMin = NULLSIZE, SIZE sizeMax = NULLSIZE);
		LAYOUT_API Owner();
		
		/// Standard Dtor, will delete m_pAlignmentManager
		virtual LAYOUT_API ~Owner();

		/// Obtain the Manager of this Layout Owner
		LAYOUT_API Manager* getManager(){ return m_pManager; }
		
		/// Set the Manager of this Layout Owner
		LAYOUT_API void setManager(Manager* pAlignmentManager);

		/// Get the HWND of the owner
		LAYOUT_API HWND getHwnd() const;
		
		/// Set the HWND of the owner windows parent window
		LAYOUT_API virtual void setParent(HWND hwndParent) const {;}
		
		/**
		 * Do all the setLayout() and putSplitter() calls in here.
		 * Calling setLayout() or putSplitter() outside this function
		 * will raise an Error-MessageBox in debug mode!
		 * 
		 * This method gets called automatically when you call initLayout().
		 */
		LAYOUT_API virtual void doLayoutDataExchange() {;}
		
		/** Call this method in your derived classes OnInitDialog/OnCreate impl. */
		LAYOUT_API void initLayout(HWND hLayoutParentWindow);
		
		/** Call this method to temporarily "turn the page" to a different Layout.
		    This is effective to display subdialogs that would be modal windows otherwise*/
		LAYOUT_API void putModalPage(Layout::Owner const* pPage);
			
		/**
		 * Set a certain alignment to any of this dialogs child controls.
		 * Use either nID or pCtrl to identify the control.
		 * @param nID The dialog control's item id.
		 * @param pCtrl The dialog control's CWnd pointer.
		 * @param hHorzAlignment The dialog controls HorizontalAlignment
		 * @param hVertAlignment The dialog controls VerticalAlignment
		 * @return true, or false, if adding failed.
		 *         Adding the control will fail if
		 *         - the control is already added,
		 *         - the ID is invalid
		 * @about  CALLING THIS METHOD IS ONLY ALLOWED WITHIN <B>doLayoutDataExchange()</B>!
		 */
		virtual LAYOUT_API bool setLayout(UINT nID, std::string sID, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment);
		virtual LAYOUT_API bool setLayout(HWND hCtrl, std::string sID, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment);
		virtual LAYOUT_API bool setLayout(UINT nID, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment);
		virtual LAYOUT_API bool setLayout(HWND hCtrl, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment);
		
		/**
		 * Set a certain alignment to all of this dialogs child controls.
		 * @about  CALLING THIS METHOD IS ONLY ALLOWED WITHIN <B>doLayoutDataExchange()</B>!
		 */
		virtual LAYOUT_API bool layoutAll(HWND hParent, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment);
		
		/**
		 * Put a splitter between two controls in the dialog.
		 * The controls must not have been separated by a splitter before.
		 * Also, both controls must have been added to the manager.
		 * @param hHigherCtrl The control above or left to the splitter.
		 * @param hLowerCtrl The control below or right to the splitter.
		 * @param nHigherCtrlId The ID of the control above or left to the splitter.
		 * @param nLowerCtrlId The ID of the control below or right to the splitter.
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
		virtual LAYOUT_API Splitter const* putSplitter(HWND hHigherCtrl, HWND hLowerCtrl, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment);
		virtual LAYOUT_API Splitter const* putSplitter(UINT nHigherCtrlId, UINT nLowerCtrlId, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment);
		virtual LAYOUT_API Splitter const* putSplitter(AreaProperties& aHighAreaParams, AreaProperties& aLowAreaParams, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment);
	};
	
	/** Vertical Resize, Horizontal Stretch */
	#define LAYOUT_CONTROL_VSHS(ctrl) setLayout(ctrl, #ctrl, Layout::Align::Resize(), Layout::Align::Resize())
	
	/** Stick to Top, Horizontal Stretch */
	#define LAYOUT_CONTROL_THS(ctrl) setLayout(ctrl, #ctrl, Layout::Align::Resize(), Layout::Align::TopLeft())
	
	/** Stick to Bottom, Horizontal Stretch */
	#define LAYOUT_CONTROL_BHS(ctrl) setLayout(ctrl, #ctrl, Layout::Align::Resize(), Layout::Align::BottomRight())
	
	/** Vertical Stretch, Stick to Left */
	#define LAYOUT_CONTROL_VSL(ctrl) setLayout(ctrl, #ctrl, Layout::Align::TopLeft(), Layout::Align::Resize())
	
	/** Vertical Stretch, Stick to Right */
	#define LAYOUT_CONTROL_VSR(ctrl) setLayout(ctrl, #ctrl, Layout::Align::BottomRight(), Layout::Align::Resize())
	
	/** Stick to TopLeft */
	#define LAYOUT_CONTROL_TL(ctrl) setLayout(ctrl, #ctrl, Layout::Align::TopLeft(), Layout::Align::TopLeft())
	
	/** Stick to BottomRight */
	#define LAYOUT_CONTROL_BR(ctrl) setLayout(ctrl, #ctrl, Layout::Align::BottomRight(), Layout::Align::BottomRight())
	
	/** Stick to TopRight */
	#define LAYOUT_CONTROL_TR(ctrl) setLayout(ctrl, #ctrl, Layout::Align::BottomRight(), Layout::Align::TopLeft())
	
	/** Stick to BottomLeft */
	#define LAYOUT_CONTROL_BL(ctrl) setLayout(ctrl, #ctrl, Layout::Align::TopLeft(), Layout::Align::BottomRight())
	
	#define LAYOUT_AREAPROPS(name, control, color) AreaProperties name; name.setControl(control); name.setColor(color);
}

#endif // _LAYOUT_OWNER_