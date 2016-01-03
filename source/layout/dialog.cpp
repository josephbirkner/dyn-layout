#include "StdAfx.h"
#pragma hdrstop

#include "Base/DynLayout/GlobExport/dialog.h"

using namespace Layout;

#define KEY_DIALOGSIZES_PROFILE_ROOT "DialogSizes"

#define SB_SHIFT 20 /// If SB_SHIFT is issued in On[H|V]Scroll for nSBCode, then the scrollbar will be shifted by nPos

/**
 * Standard ctor. Will initialize the AlignmentManager without min/max size or Profiling
 * @param nID The dialogs resource id
 * @param pParent The dialogs parent window
 */
ManagedDialog::ManagedDialog( UINT nID, CWnd* pParent ) : CDialog(nID, pParent), Owner()
{
	m_nProfiling = ProfileGlobal;
	m_hMinSize = CSize(0, 0);
	m_hMaxSize = CSize(0, 0);
	m_sResource = "";
	m_nID = nID;
	m_ptScrollPos.x = m_ptScrollPos.y = 0;
	m_hMinSize.cx = m_hMinSize.cy = 0;
}

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
ManagedDialog::ManagedDialog(
	UINT nID,
	CWnd*
	pParent,
	CSize hMinSize,
	CSize hMaxSize,
	Profiling nProfile
	) : CDialog(nID, pParent), Owner()
{
	m_nProfiling = nProfile;
	m_hMinSize = hMinSize;
	m_hMaxSize = hMaxSize;
	m_sResource = "";
	m_nID = nID;
	m_nMinSizeBehaviour = BlockSmallerSizes;
	m_ptScrollPos.x = m_ptScrollPos.y = 0;
}

ManagedDialog::ManagedDialog(
	UINT nID,
	CWnd* pParent,
	CSize hMinSize,
	CSize hMaxSize,
	Profiling nProfile,
	MinSizeBehaviour nMinSizeBehaviour
) : CDialog(nID, pParent), Owner()
{
	m_nProfiling = nProfile;
	m_hMinSize = hMinSize;
	m_hMaxSize = hMaxSize;
	m_sResource = "";
	m_nID = nID;
	m_nMinSizeBehaviour = nMinSizeBehaviour;
	m_ptScrollPos.x = m_ptScrollPos.y = 0;
}

/**
 * Dtor. Will delete m_pAlignmentManager.
 */
ManagedDialog::~ManagedDialog()
{
}

BEGIN_MESSAGE_MAP(ManagedDialog, CDialog)
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

/**
 * Helper function to extract a filename from a Path. E.g. C:\foo.bar would deliver foo
 */
CString ManagedDialog::FileNameFromPath(CString sPath)
{
	CString sName(sPath);
	CString sPathReversed(sPath);
	sPathReversed.MakeReverse();
	
	int iPos = (sName.GetLength() - 1) - sPathReversed.Find('\\');
	sName = sName.Mid(iPos + 1, sName.GetLength() - iPos - 1);
	iPos = (sName.GetLength() - 1) - sPathReversed.Find('.');
	sName = sName.Mid(0, iPos);
	return sName;
}

/**
 * The alignment manager will be created here, and your dialogs implementation of initAlignment() will be called
 * @see initAlignment()
 */
BOOL ManagedDialog::OnInitDialog()
{
	// call the super dialog initialization
	CDialog::OnInitDialog();
	
	return true;
}

/**
 * ManagedDialog::OnSizing
 */
void ManagedDialog::OnSizing(UINT nSide, LPRECT lpRect)
{
	CDialog::OnSizing(nSide, lpRect);
	
	if( m_nMinSizeBehaviour > 0 )
	{
		RECT hRect = *lpRect;
		getAlignmentManager()->update(nSide, lpRect);
		
		/* If the MinSizeBehaviour is ShowScrollbars, then
		 * the alignment managers output regarding lpRect
		 * must be ignored if the window is scaled below its
		 * minimum size on either dimension.
		 *
		 * If the window gets bigger while having a positive scrolling position,
		 * then we must scroll towards zero by the resize delta.
		 */
		int iScrollPos;
		RECT rctCur;
		this->GetWindowRect(&rctCur);
		
		if( m_nMinSizeBehaviour & ShowScrollbarsX && (hRect.right - hRect.left) <= m_hMinSize.cx )
		{
			lpRect->left   = hRect.left;
			lpRect->right  = hRect.right;
			int iDX = (lpRect->right - lpRect->left) - (rctCur.right - rctCur.left);
			
			if( (iScrollPos = m_ptScrollPos.x) > 0 && iDX > 0 )
				OnHScroll( SB_SHIFT, -(iDX > iScrollPos ? iScrollPos : iDX), NULL);
		}
		
		if( m_nMinSizeBehaviour & ShowScrollbarsY && (hRect.bottom - hRect.top) <= m_hMinSize.cy )
		{
			lpRect->bottom = hRect.bottom;
			lpRect->top    = hRect.top;
			int iDY = (lpRect->bottom - lpRect->top) - (rctCur.bottom - rctCur.top);
				
			if( (iScrollPos = m_ptScrollPos.y) > 0 && iDY > 0 )
				OnVScroll( SB_SHIFT, -(iDY > iScrollPos ? iScrollPos : iDY), NULL );
		}
	}
	else
		getAlignmentManager()->update(nSide, lpRect);
}

/**
 * Helper function called in OnMove and OnSize
 */
void ManagedDialog::storeSizeAndPosition()
{
	if( !m_sResource.IsEmpty() )
	{
		CRect rct;
		
		this->GetWindowRect(rct);
		Profile::WriteLong(m_sResource, "w", rct.Width());
		Profile::WriteLong(m_sResource, "h", rct.Height());
		Profile::WriteLong(m_sResource, "x", rct.left);
		Profile::WriteLong(m_sResource, "y", rct.top);
	}
}	

/**
 * Helper function called in OnSize and OnInitDialog
 */
void ManagedDialog::updateScrollbars(int iWidth, int iHeight)
{
	if((m_nMinSizeBehaviour & ShowScrollbarsX) || (m_nMinSizeBehaviour & ShowScrollbarsY))
	{
		// calculate client rect difference
		RECT rctCur;
		RECT rctCli;
		this->GetWindowRect(&rctCur);
		this->GetClientRect(&rctCli);
		int iCliRctDX = (rctCur.right - rctCur.left) - (rctCli.right - rctCli.left);
		int iCliRctDY = (rctCur.bottom - rctCur.top) - (rctCli.bottom - rctCli.top);
	
		// Horizontal
		SCROLLINFO hScrollInfoH;
		hScrollInfoH.cbSize = sizeof(SCROLLINFO);
		hScrollInfoH.fMask = SIF_PAGE|SIF_RANGE;
		hScrollInfoH.nMin = 0;
		hScrollInfoH.nMax = m_hMinSize.cx - iCliRctDX;
		hScrollInfoH.nPage = iWidth + 1;
		
		// Vertical
		SCROLLINFO hScrollInfoV;
		hScrollInfoV.cbSize = sizeof(SCROLLINFO);
		hScrollInfoV.fMask = SIF_PAGE|SIF_RANGE;
		hScrollInfoV.nMin = 0;
		hScrollInfoV.nMax = m_hMinSize.cy - iCliRctDY;
		hScrollInfoV.nPage = iHeight + 1;
		
		SetScrollInfo(SB_HORZ, &hScrollInfoH, TRUE); 
		SetScrollInfo(SB_VERT, &hScrollInfoV, TRUE);
	}
}

/**
 * ManagedDialog::OnMove
 */
void ManagedDialog::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);
	this->storeSizeAndPosition();
}

/**
 * ManagedDialog::OnSize
 */
void ManagedDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	/* Scrollbar Updating */
	this->updateScrollbars(cx, cy);
	
	/* Store Size and Position */
	this->storeSizeAndPosition();
	
	/* React on maximization */
	if(nType == SIZE_MAXIMIZED || nType == SIZE_RESTORED)
		if ( this->getAlignmentManager() )
			this->getAlignmentManager()->update();
}

/**
 * ManagedDialog::OnHScroll
 */
void ManagedDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	
	// Get the minimum, maximum and current scroll-bar positions
	int iMin = 0;
	int iMax = m_hMinSize.cx;
	int iCur;
	int iNew = iCur = m_ptScrollPos.x;
	
	// calculate new position
	switch( nSBCode )
	{
		case SB_LEFT:
		case SB_PAGELEFT:
		{
			iNew = iMin;
			break;
		}
		case SB_RIGHT:
		case SB_PAGERIGHT:
		{
			iNew = iMax;
			break;
		}
		case SB_LINELEFT:
		{
			iNew = iNew < iMax ? iNew + 1 : iNew;
			break;
		}
		case SB_LINERIGHT:
		{
			iNew = iNew > iMin ? iNew - 1 : iNew;
			break;
		}
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
		{
			iNew = nPos;
			break;
		}
		case SB_SHIFT:
		{
			iNew = iCur + nPos;
			break;
		}
	} // switch
	
	m_ptScrollPos.x = iNew;
  SetScrollPos(SB_HORZ, iNew, TRUE);
  ScrollWindow(iCur - iNew, 0);
}

/**
 * ManagedDialog::OnVScroll
 */
void ManagedDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
	if( !pScrollBar ) this->GetScrollBarCtrl(SB_HORZ);
	
	// Get the minimum, maximum and current scroll-bar positions
	int iMin = 0;
	int iMax = m_hMinSize.cy;
	int iCur;
	int iNew = iCur = m_ptScrollPos.y;
	
	// calculate new position
	switch( nSBCode )
	{
		case SB_TOP:
		case SB_PAGEUP:
		{
			iNew = iMin;
			break;
		}
		case SB_BOTTOM:
		case SB_PAGEDOWN:
		{
			iNew = iMax;
			break;
		}
		case SB_LINEUP:
		{
			iNew = iNew < iMax ? iNew + 1 : iNew;
			break;
		}
		case SB_LINEDOWN:
		{
			iNew = iNew > iMin ? iNew - 1 : iNew;
			break;
		}
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
		{
			iNew = nPos;
			break;
		}
		case SB_SHIFT:
		{
			iNew = iCur + nPos;
			break;
		}
	} // switch
	
	m_ptScrollPos.y = iNew;
  SetScrollPos(SB_VERT, iNew, TRUE);
  ScrollWindow(0, iCur - iNew);
}
