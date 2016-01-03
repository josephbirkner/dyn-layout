#ifndef _LAYOUT_AREA_
#define _LAYOUT_AREA_

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

#include <vector>

#include "splitter.h"
#include "control.h"
#include "areacreateparams.h"

class LayoutTest;

namespace Layout
{
	class Control;

	class Area : public CStatic, public AreaProperties
	{
		DECLARE_DYNAMIC(Area)

		friend class Area;
		friend class Splitter;
		friend class Manager;
		friend class LayoutTest;

	protected:
		LAYOUT_API Area(Area const* pParent, CRect const& rctFrame, SIZE const& hMinSize, SIZE const& hMaxSize);
		LAYOUT_API Area(Manager const* pMgr, CRect const& rctFrame, SIZE const& hMinSize, SIZE const& hMaxSize);
		virtual ~Area();

		/** Inserts pControl into this alignment area's control list, if it is within the area.
			Returns true, if the control has been inserted, false if otherwise.
			Will propagate the insertion down to the last child area level. */
		bool insertIfOwned(Control* pControl);

		/** Remove an aligned control from this areas control list. Recurses the removal to the parent area.
			Returns true if the control was found and removed.
			This method will only remove the list entry, the pointer will stay intact. */
		bool removeControl(Control* pCtrl);

		/** Resize the area. If the new shape is below the minimum size,
		    so that the area collapses, newShape will be overwritten with the collapsed shape*/
		void resizeAndAutoFoldIfNecessary(__inout CRect& newShape, __in bool bShapeIsScreenCoords = true);

		/** Updates the splitters position according to its alignment, and resizes the child areas accordingly. */
		void updateSplitter();

		/** Updates the shapes of the child areas according to the splitters current position.
			DOES NOT PERFORM ANY MIN SIZE ACCORDANCE VERIFICATION. */
		void updateChildAreas();

		/** Corrects the position of a given splitter rect according to the child areas minimum sizes.
		    Returns true if the splitter rect was manipulated. */
		bool checkSplitterRectWithChildAreaMinSizesAndAutoFold(__inout CRect& rctSplitter) const;

		/** Delivers the shapes of the child areas according to this areas splitters current position. */
		void getChildAreaShapes(__out CRect& rctHi, __out CRect& rctLo ) const;

		/** Delivers the shapes of the child areas according to a given splitter rect. */
		void getChildAreaShapes(__out CRect& rctHi, __out CRect& rctLo, __in CRect const& rctSplitter, __in bool bSplitterRectIsScreenCoords = false ) const;

		/** Updates the control shapes according to theire alignments. */
		void updateControls();

		/** Update the areas actual minimum size. That is either the sum of the child areas minimum sizes,
			or this areas minimum size, depending on which one is smaller. */
		void updateMinSize() const;

		/** Returns, whether this area has a minimum size. */
		bool hasMinMaxSize() const;

		/** Updates m_rctCurrentRect according to the parent windows current screen coords. */
		void updateCurrentRect();

		/** Updates m_rctOrigRect according to the current m_rctCurrentClientRect. */
		void updateOrigRect();

		/** Updates the folded min size. Incorporates eventual differences between
		    the visible client rect and the actual client rect for areas that touch
		    the parent windows border. */
		void updateProcessedFoldedMinSize();

		/** Split the alignment area between two controls. They both must belong to this alignment area.
			The splitter will  be put below/right to pHigh, and above/left to pLow, depending on the orientation of the splitter.
			This works only if this alignment area doesn't already have a splitter. */
		Splitter const* putSplitter(Control const* pHigh, Control const* pLow, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment);

		/** Obtain a rectangle for a new splitter between pHigh and pLow  */
		void getSplitterRect( Splitter::Orientation nOrientation, __out CRect& rctSplitter, Control const* pHigh, Control const* pLow );

		/** Set the current rect in screen coords. The client coords version will be updated automatically too. */
		virtual void setCurrentRect(CRect const& rctShape, bool bShapeIsScreenCoords = true);

		/** Folds the area on a specific dimension. */
		void fold(Splitter::Orientation nOrientation);

		/** Unfolds the area. */
		void unfold(__in CRect const& rctDesiredUnfoldedShape);

		/** Check if the area is folded on a specific dimension */
		bool isFolded(Splitter::Orientation nDim) const;

		/** Check if the area would fold on a specific dimension, if the test size was applied. */
		bool wouldFold(Splitter::Orientation nDim, SIZE szTestSize) const;

		/** Manipulates the width or height of the given area shape to be folded on a given dimension */
		void getFoldedShape( __in Splitter::Orientation nOrientation, __inout CRect& rctShape );

		/** Creates a static background window for the area to establish the background color and area captioning. */
		void CreateLayoutAreaWindow();

		/** Calls draw recursively on the children, or draws the text and
		    the background color of the area if it does not have children. */
		void draw( HDC hDC );

		/** This method returns a point with added values from the visible
		    TopLeft corner of the area. The visible TopLeft corner is not
		    necessarily the real topleft corner, since the real topleft corner
		    can be covered by the window menu. */
		POINT addToClientRectVisibleTopLeftPoint(int iOffX, int iOffY);

		/** Returns the region of the area visible to the user. The visible region is not
		    necessarily the real region, since part of the real region
		    can be covered by the window menu. */
		CRect getVisibleClientRect();

		/** Returns whether this area touches any of the parent windows edges.
		    The method will only check the edges for which the input parameter value is one. */
		void getTouchedParentWindowEdges(__inout bool& bTop, __inout bool& bBottom, __inout bool& bLeft, __inout bool& bRight) const;

		/** If the designates HWND belongs to the background control of this area or
		    any of this areas child areas, this function returns that area */
		Area* isBackgroundHwnd(HWND hCtrl);

	public:
		/** Get the current size of the area */
		LAYOUT_API SIZE getSize() const { return m_rctCurrentShape.Size(); };

		/** Get the original size of the alignment area. Returns a member reference, manipulate not/with care. */
		LAYOUT_API SIZE getOrigSize() const { return m_rctOrigClientShape.Size(); };

		/** Get the current rect of the area in screen coords. */
		LAYOUT_API CRect const& getRect() const { return m_rctCurrentShape; };

		/** Get the original rect of the area in client coords. */
		LAYOUT_API CRect const& getOrigClientRect() const { return m_rctOrigClientShape; };

		/** Get the current rect of the area in client coords. */
		LAYOUT_API CRect const& getClientRect() const { return m_rctCurrentClientShape; }

		/** Update the shape of the area with a new user specified rect.
		    The area will check to see if the new shape is smaller than the possible minimum. */
		LAYOUT_API void update(__inout CRect rctNewrect);

		/** Clamps the given rect between the min and max size of the area */
		LAYOUT_API bool clampRect(__in UINT nSide, __inout CRect& rctNewrect);

		/** Obtain the Manager maintaining this Alignment Area */
		LAYOUT_API Manager const* getManager() const {return m_pAlignmentManager;}

		/** Get the controls that belong into this alignment area. */
		LAYOUT_API std::vector<Control*> const& getControls() const {return m_vControls;}

		/** Returns true, if the control belongs to this alignment area.
			(That is, when its Top Left corner lies within the area) */
		LAYOUT_API bool isControlInRect(Control const* pCtrl);

		/** Returns whether this area is the parent area of two other areas */
		LAYOUT_API bool isParentArea() const;

		/** Get the areas final minimum size. Updated in updateMinSize() */
		LAYOUT_API SIZE const& getMinSize() const;

		/** Get the areas final minimum size. Updated in updateMinSize() */
		LAYOUT_API SIZE const& getMaxSize() const;

		/** Get the high child (Top/Left). Returns Null if this is not a parent area. */
		LAYOUT_API Area const* getChildHi() const {return m_pHiChild;}

		/** Get the low child (Bottom/Right). Returns Null if this is not a parent area. */
		LAYOUT_API Area const* getChildLo() const {return m_pLoChild;}

		/** Get the parent area. Returns Null if this is a child area. */
		LAYOUT_API Area const* getParent() const {return m_pParent;}

		/** Get the splitter of this Alignment Area. Returns Null if this is not a parent area. */
		LAYOUT_API Splitter const* getSplitter() const {return m_pSplitter;}

		/** Returns which Background color style (normal or hover) is currently used by the area. */
		LAYOUT_API AreaStyles getCurrentBkColorStyle() const;

		/** Returns whether this area is the lower one of the parents child areas.*/
		LAYOUT_API bool isLoArea() const;

		/** Returns whether the first parent area whose parent is
		    split by a splitter of the given orientation is the lower sibling.*/
		LAYOUT_API bool isLoArea(Splitter::Orientation nOrientation) const;

		/** Returns whether this area is the higher one of the parents child areas.*/
		LAYOUT_API bool isHiArea() const;

		/** Returns whether the first parent area whose parent is
		    split by a splitter of the given orientation is the lower sibling.*/
		LAYOUT_API bool isHiArea(Splitter::Orientation nOrientation) const;

		/** Hides/shows the area, its controls and its child areas. */
		LAYOUT_API virtual void setVisible(bool bShow);

		/** Returns whther the area is currently visible */
		LAYOUT_API bool isVisible() {return m_bVisible;}

	protected:
		DECLARE_MESSAGE_MAP()

		// virtual BOOL PreTranslateMessage(MSG* pMsg);

		virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		afx_msg BOOL OnEraseBkgnd(CDC *pDC);
		afx_msg void OnMouseLeave();
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	protected:
		/** Shape management */
		mutable CRect m_rctCurrentShape;        /// The window's current shape, updated in update()
		mutable CRect m_rctCurrentClientShape;  /// The window's current shape in client coords, updated in update()
		mutable CRect m_rctCurrentVisibleShape; /// The window's current shape in screen coords with the visible boundaries
		mutable CRect m_rctCurrentVisibleClientShape; /// The window's current shape in client coords with the visible boundaries
		mutable CRect m_rctOrigClientShape;     /// The window's orginal shape in client coords

		/** Min Size Management */
		SIZE m_hMinSize;      /// The areas user issued minimum size
		mutable SIZE m_hProcessedMinSize; /// The windows processed min size (The result of the recursive minsize check done in getMinSize())
		mutable SIZE m_hFoldedMinSize; /// The windows processed min size (The result of the recursive minsize check done in getMinSize())
		mutable SIZE m_hProcessedFoldedMinSize; /// The windows processed min size (The result of the recursive minsize check done in getMinSize())
		SIZE m_hMaxSize;      /// The areas user issued maximum size

		/** Status management */
		bool m_bHovered;      /// Set in OnMouseHover. Tells draw() to use the hover color.
		bool m_bVisible;      /// Set in setVisible. Tells draw() to draw nothing if false.
		mutable std::map<Splitter::Orientation, bool> m_mapDimensionFolded; /// Tells if the area is folded for a specific splitter orientaion

		/** Associate pointers */
		std::vector<Control*> m_vControls; /// The controls that fall into this area
		Manager const* m_pAlignmentManager; /// The manager this area is maintained by
		Area const* m_pParent; /// The parent area. Can be Null.
		Area* m_pHiChild; /// The higher child area (Top/Left)
		Area* m_pLoChild; /// The lower child area (Bottom/Right)
		Splitter* m_pSplitter; /// The splitter of this Alignment Area. Null if this is not a parent area.
	};
}

#endif // _LAYOUT_AREA_