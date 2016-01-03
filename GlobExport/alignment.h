#ifndef _LAYOUT_ALIGNMENT_
#define _LAYOUT_ALIGNMENT_

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

#include "control.h"

namespace Layout
{
	class Control;

	namespace Align
	{
		enum Dimension
		{
			Horizontal,
			Vertical
		};

		/**
		 * Virtual Alignment Base class.
		 * Derivatives must overload the update() function in order
		 * to enforce the alignment for the passed dimension.
		 */
		class LAYOUT_API Mode
		{
		public:
			virtual Mode* copy() = 0;

			/// Purely virtual update function
			virtual void update( Control*, Dimension nDim, CRect& rctResult ) = 0;

			/// Delivers the minimum amount of space necessary for the alignment,
			/// from the top (to bottom), from the bottom (to top), from the left (to right),
			/// and from the right (to left) parent area edge.
			virtual void getMinInsets(__in Control const* pCtrl, __in Dimension nDimension, __out CRect& insets) { insets.SetRect(0, 0, 0, 0); }
		};

		#define DECLARE_COPY(alignment) virtual Mode* copy(){ return new alignment(*this); }

		/**
		 * The control will stick to the parents top/left edge, the width will be constant.
		 * This is the fastest mode, and what is enforced by MFC by default
		 */
		class LAYOUT_API TopLeft : public Mode
		{
		public:
			DECLARE_COPY(TopLeft);
			virtual void update( Control*, Dimension nDim, CRect& rctResult );
			virtual void getMinInsets(__in Control const* pCtrl, __in Dimension nDimension, CRect& insets);
		};

		/**
		 * The control will resize, so that the distances to
		 * the parents left/top and right/bottom edge are always constant.
		 */
		class LAYOUT_API Resize : public Mode
		{
			/// Default minimum size for a resizable control, 10px from both sides.
			static const int iDefaultMinSize = 10 * 2;

		public:
			DECLARE_COPY(Resize);
			
			/** Alternate ctor: allows for creating a resize alignment with a minimum size. */
			Resize(int minSize) : m_iMinSize(minSize) {}

			Resize() : m_iMinSize(iDefaultMinSize) {}
			virtual void update( Control*, Dimension nDim, CRect& rctResult );
			virtual void getMinInsets(__in Control const* pCtrl, __in Dimension nDimension, __out CRect& insets);

		private:
			int m_iMinSize;
		};

		/**
		 * The control will resize, so that the bottom/right distance
		 * always equals the top/left distance to the corresponding parent edge
		 */
		class LAYOUT_API Fit : public Mode
		{
		public:
			DECLARE_COPY(Fit);
			virtual void update( Control*, Dimension nDim, CRect& rctResult );
		};

		/**
		 * The control will stick to the parents right/bottom edge,
		 * the width/height will be constant.
		 */
		class LAYOUT_API BottomRight : public Mode
		{
		public:
			DECLARE_COPY(BottomRight);
			virtual void update( Control*, Dimension nDim, CRect& rctResult );
			virtual void getMinInsets(__in Control const* pCtrl, __in Dimension nDimension, CRect& insets);
		};

		/**
		 * The controls position will be scaled with the dialog.
		 * If m_bResize is true, then the controls size will be
		 * scaled as well.
		 */
		class LAYOUT_API Relative : public Mode
		{
		private:
			bool m_bResize;

		public:
			DECLARE_COPY(Relative);
			Relative(bool bResize);

			virtual void update( Control*, Dimension nDim, CRect& rctResult );
		};
	}
}

#undef DECLARE_COPY
#endif // _LAYOUT_ALIGNMENT_