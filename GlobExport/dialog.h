#ifndef _ALIGNED_DLG_
#define _ALIGNED_DLG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Base/DynLayout/GlobExport/owner.h"

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

namespace Layout
{
	/**
	 * CDialog-derived helper class wrapping the functionality of the Manager.
	 * Use:
	 * - Derive your dialog class from ManagedDialog
	 * - [Optional] Change your dialogs ctor to construct the ManagedDialog part
		 with the alternate ctor specifying min/max size and one of the ::Profiling flags
	 * - In your Implementation, dont forget to change the Message Mapping
	 *   from BEGIN_MESSAGE_MAP(CMyDialog, CDialog) to BEGIN_MESSAGE_MAP(CMyDialog, ManagedDialog)
	 * - In CMyDialog::OnInitDialog, dont forget to call ManagedDialog::OnInitDialog()
	 * - In the overloaded initAlignment() method, use the setLayout() methods to set specific alignments for specific controls
	 */
	class LAYOUT_API ManagedDialog : public CDialog, public Owner
	{
	public:
		/**
		 * Profiling modes for the ManagedDialog. Profiling means, that the dialogs position/size is automatically
		 * stored/restored in/from the windows registry.
		 */
		enum Profiling
		{
			ProfileOff,
			ProfileApplicationSpecific,
			ProfileGlobal
		};
		
		/**
		 * Different behaviours, when the dialog is scaled below its minimum size.
		 */
		enum MinSizeBehaviour
		{
			BlockSmallerSizes = 0,
			ShowScrollbarsX   = 1,
			ShowScrollbarsY   = 2,
			ShowScrollbars    = 3
		};

	private:
		CSize            m_hMinSize;          /// This aligned dialogs minimum size
		CSize            m_hMinCliSize;       /// This dialogs minimum client rect size (calculated from m_hMinSize)
		CSize            m_hMaxSize;          /// This aligned dialogs maximum size
		Profiling        m_nProfiling;        /// This aligned dialogs profiling mode
		UINT             m_nID;               /// This dialogs resource id
		CString          m_sResource;         /// Name of the module providing this dialogs resource id
		MinSizeBehaviour m_nMinSizeBehaviour; /// This aligned dialogs behaviour, if scaled below its minimum size
		int              m_iCliRctDX;         /// Tells how much smaller the client rect is than the window rect (X)
		int              m_iCliRctDY;         /// Tells how much smaller the client rect is than the window rect (Y)
		POINT            m_ptScrollPos;       /// The current horz and vert scrolling positions
		
		/**
		 * Helper function called in OnMove and OnSize
		 */
		void storeSizeAndPosition();
		
		/**
		 * Helper function called in OnSize and OnInitDialog
		 */
		void updateScrollbars(int, int);
		
		/**
		 * Helper function to extract a filename from a Path. E.g. C:\foo.bar would deliver foo
		 */
		static CString FileNameFromPath( CString sPath );
		
	public:
		/**
		 * Standard ctor. Will initialize the AlignmentManager without min/max size or Profiling
		 * @param nID The dialogs resource id
		 * @param pParent The dialogs parent window
		 */
		ManagedDialog( UINT nID, CWnd* pParent );

		/**
		 * Alternate ctors. Enables you to initialize the dialog specifying limit sizes and a profiling mode.
		 * @param nID The dialogs resource id
		 * @param pParent  The dialogs parent window
		 * @param hMinSize The dialogs minimim size. Must be smaller than the maximum size. <B>The given rect should be in DLU's</B>!
		 *                 If hMinSize equals {0,0}, then it will be set to the dialogs original size.
		 * @param hMaxSize The dialogs maximum size. Will be set to hMinSize if smaller on either dimension. <B>The given rect should be in DLU's</B>!
		 * @param bProfile [optional] The dialogs profiling mode. The following Profiling modes are possible:
		 *                 <P><BLOCKQUOTE><UL>
		 *                 <LI>ProfileOff: [default] Profiling is turned off completely, the dialog will remain at its default position and size.</LI>
		 *                 <LI>ProfileApplicationSpecific: The dialogs position and size will be saved/restored specifically for each application it appears in.</LI>
		 *                 <LI>ProfileGlobal: The dialogs position and size will be saved/restored to/from the same dataset for each application the dialog might appear in.</LI>
		 *                 </UL></BLOCKQUOTE></P>
		 *                 If your dialog is present in one application only, there wont be
		 *                 any visible difference between ProfileApplicationSpecific and ProfileGlobal
		 * @param nMinSizeBehaviour [optional] An Element of <B>ManagedDialog::MinSizeBehaviour:</B>
		 *                 <P><BLOCKQUOTE><UL>
		 *                 <LI>BlockSmallerSizes: [default] The Dialog won't be resizable below its minimum size.</LI>
		 *                 <LI>ShowScrollbarsX: It will be possible to resize the dialog below its minimum width, with a scrollbar appearing on the dialogs bottom edge in case.</LI>
		 *                 <LI>ShowScrollbarsY: It will be possible to resize the dialog below its minimum height, with a scrollbar appearing on the dialogs right edge in case.</LI>
		 *                 <LI>ShowScrollbars: It will be possible to resize the dialog below its minimum size, with scrollbars appearing on the dialogs right and bottom edges in case.</LI>
		 *                 </UL></BLOCKQUOTE></P>
		 */
		ManagedDialog( UINT nID, CWnd* pParent, CSize hMinSize, CSize hMaxSize, ManagedDialog::Profiling nProfile = ProfileOff );
		ManagedDialog( UINT nID, CWnd* pParent, CSize hMinSize, CSize hMaxSize, ManagedDialog::Profiling nProfile, MinSizeBehaviour nMinSizeBehaviour );
		
		/**
		 * Dtor. Will delete m_pAlignmentManager.
		 */
		virtual ~ManagedDialog();
		
		/**
		 * The alignment manager will be created here, and your dialogs implementation of initAlignment() will be called
		 * @see initAlignment()
		 */
		virtual BOOL OnInitDialog();


		/**
		 * Get this AlignedDialog behaviour, when the user tries to scale it below its minimum size.
		 * @returns
		 * <P><BLOCKQUOTE><UL>
		 * <LI>BlockSmallerSizes: The Dialog won't be resizable below its minimum size.</LI>
		 * <LI>ShowScrollbars: It will be possible to resize the dialog below its minimum size, with scrollbars appearing on the dialogs right and bottom edges in case.</LI>
		 * </UL></BLOCKQUOTE></P>
		 * @about Can only be set in the ctor.
		 */
		MinSizeBehaviour getMinSizeBehaviour(){return m_nMinSizeBehaviour;}
		
		/**
		 * Get this dialogs Profiling mode.
		 * @returns
		 * <P><BLOCKQUOTE><UL>
		 * <LI>ProfileOff: Profiling is turned off completely, the dialog will remain at its default position and size.</LI>
		 * <LI>ProfileApplicationSpecific: The dialogs position and size will be saved/restored specifically for each application it appears in.</LI>
		 * <LI>ProfileGlobal: The dialogs position and size will be saved/restored to/from the same dataset for each application the dialog might appear in.</LI>
		 * </UL></BLOCKQUOTE></P>
		 * @about Can only be set in the ctor.
		 */
		Profiling getProfilingMode(){return m_nProfiling;}
		
	protected:
		afx_msg void OnSizing(UINT, LPRECT);
		afx_msg void OnMove(int x, int y);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		
		DECLARE_MESSAGE_MAP()
	};
}

#endif // _ALIGNED_DLG_