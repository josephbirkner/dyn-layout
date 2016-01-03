#include "stdafx.h"
#pragma hdrstop

#include "../../GlobExport/manager.h"
#include "../../GlobExport/area.h"
#include "../../GlobExport/control.h"

using namespace Layout;

/**
 * CTor. Create a new Aligned Control instance for a specific manager and window, with specific horizontal
 * and vertical alignments.
 */
Control::Control( Manager const* pMgr, HWND hCtrl, Align::Mode& hAlignHorz, Align::Mode& hAlignVert, std::string sName /*= ""*/ ) :
	m_pAlignmentArea(NULL),
	m_pManager(NULL),
	m_bVisibilityBeforeTempHide(FALSE),
	m_sName(sName)
{
	m_hID = hCtrl;
	m_pManager = pMgr;
	m_pHorzAlign = hAlignHorz.copy();
	m_pVertAlign = hAlignVert.copy();
	m_bVisibilityBeforeTempHide = (::GetWindowLong(hCtrl, GWL_STYLE) & WS_VISIBLE) != FALSE;
	updateOrigRect();
	getRect();
	m_pWndProc = (WNDPROC) ::GetWindowLong(hCtrl, GWL_WNDPROC);
	s_mapControlForHwnd[hCtrl] = this;
	//installEditorWndProc(false);
}

/**
 * Control DTor.
 */
Control::~Control()
{
	delete m_pHorzAlign;
	delete m_pVertAlign;
}

/**
 * Get this Control's window's current rect.
 * Also updates the m_rctCurrent member.
 */
CRect const& Control::getRect() const
{
	AFXASSUME(("Invalid handle!", m_hID && m_pManager));
	if (m_hID && m_pManager)
	{
		getControl()->GetWindowRect(m_rctCurrent);
		m_pManager->getWnd()->ScreenToClient(m_rctCurrent);
		return m_rctCurrent;
	}
	
	return m_rctCurrent;
}

/**
 * Update (enforce) this Control's window's alignment according to the specified modes.
 */
HDWP Control::update(HDWP windowPosHandle)
{
	CRect rctOld(getRect());
	
	// update horizontal and vertical alignment
	m_pHorzAlign->update(this, Align::Horizontal, m_rctCurrent);
	m_pVertAlign->update(this, Align::Vertical, m_rctCurrent); 
	
	// enforce the new rect if necessary
	return DeferWindowPos(
		windowPosHandle, 
		getControl()->GetSafeHwnd(), 
		HWND_TOP, 
		m_rctCurrent.TopLeft().x, 
		m_rctCurrent.TopLeft().y, 
		m_rctCurrent.Width(), 
		m_rctCurrent.Height(), 
		SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER
	);
}

/**
 * Update this Control's orig rect to the current window rect of the window it represents
 */
void Control::updateOrigRect()
{
	AFXASSUME(("Invalid handle!", m_hID && m_pManager));
	if (m_hID && m_pManager)
	{
		getControl()->GetWindowRect(m_rctOrig);
		m_pManager->getWnd()->ScreenToClient(m_rctOrig);
	}
}

void Control::setAlignmentArea( Area const* pArea )
{
	m_pAlignmentArea = pArea;
}

Manager const* Control::getManager() const
{
	return m_pManager;
}

const Area* Control::getArea() const
{
	return m_pAlignmentArea;
}

void Control::getScreenRect(CRect& rctResult) const
{
	getControl()->GetWindowRect(rctResult);
}

void Control::getMinInsets(CRect& bounds) const
{
	if (::GetWindowLong(m_hID, GWL_STYLE) & WS_VISIBLE)
	{
		m_pHorzAlign->getMinInsets(this, Align::Horizontal, bounds);
		m_pVertAlign->getMinInsets(this, Align::Vertical, bounds);
	}
}

void Control::setHorzAlignment( Align::Mode* pAlignHorz )
{
	if (m_pHorzAlign != pAlignHorz)
		delete m_pHorzAlign;
	
	m_pHorzAlign = pAlignHorz;
}

void Control::setVertAlignment( Align::Mode* pAlignVert )
{
	if (m_pVertAlign != pAlignVert)
		delete m_pVertAlign;
	
	m_pVertAlign = pAlignVert;
}

void Control::temporaryHide()
{
	m_bVisibilityBeforeTempHide = (::GetWindowLong(m_hID, GWL_STYLE) & WS_VISIBLE) != FALSE;
	::ShowWindow(m_hID, FALSE);
}

void Control::temporaryShow()
{
	if (m_bVisibilityBeforeTempHide)
		::ShowWindow(m_hID, TRUE);
}

std::map<HWND, Control*> Layout::Control::s_mapControlForHwnd;

LRESULT CALLBACK Layout::Control::EditorWindowProc( _In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam )
{
	Control* pCtrl = s_mapControlForHwnd[hwnd];
	if(!pCtrl)
		return FALSE;
		
	switch(uMsg)
	{
		// intercept input events
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_DEADCHAR:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_SYSCHAR:
		case WM_SYSDEADCHAR:
		{
			return TRUE;
		}
	}
	
	return (pCtrl->getWndProc())(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Layout::Control::NormalWindowProc( _In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam )
{
	Control* pCtrl = s_mapControlForHwnd[hwnd];
	if(!pCtrl)
		return FALSE;
		
	switch(uMsg)
	{
		case WM_KEYUP:
		{
			// Ctrl + Shift + Alt + L will open the editor
			if( // Ctrl + Shift + Alt
				(GetKeyState(VK_LSHIFT)&0x8000) != 0 &&
				(GetKeyState(VK_LCONTROL)&0x8000) != 0 &&
				(GetKeyState(VK_LMENU)&0x8000) != 0 )
			{
				if(wParam == 0x4C) // L Key
				const_cast<Manager*>(pCtrl->getManager())->openEditor();
				return true;
			}
		}
	}
	
	return (pCtrl->getWndProc())(hwnd, uMsg, wParam, lParam);
}

void Layout::Control::installEditorWndProc( bool bInstall ) const
{
//	if(bInstall)
//		m_pWndProc = (WNDPROC) ::SetWindowLong(m_hID, GWL_WNDPROC, (LONG) EditorWindowProc);
//	else
//		::SetWindowLong(m_hID, GWL_WNDPROC, (LONG) m_pWndProc);
}
