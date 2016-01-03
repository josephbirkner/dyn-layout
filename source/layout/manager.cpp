#include "stdafx.h"
#pragma hdrstop

#include <algorithm>

#include "../../GlobExport/alignment.h"
#include "../../GlobExport/manager.h"
#include "../../GlobExport/area.h"
#include "../../GlobExport/splitter.h"
#include "../../GlobExport/owner.h"
#include "../../GlobExport/editor.h"
#include "../../GlobExport/geometry.h"
#include "../../GlobExport/profile.h"

#include "ArchiveUtil/GlobExport/ArchiveUtil.hpp"

#include <assert.h>
#include <boost/filesystem.hpp>
#include <stdlib.h>
using namespace Layout;

#define WNDMSG_LAYOUT_MODALPAGECLOSED WM_USER + 300

#define KEY_PROFILING_ROOT_NODE "DialogSizes"

#define DYNAMIC_IDC_START_VALUE 0x5000

/**
 * Returns the bounds of the main display
 */
CRect GetDisplayBounds();

Registry* Layout::Registry::_instance = NULL;

/**
 * Alignment Manager Ctor.
 * @param hParent The window whose children are to be aligned
 * @param hMinSize [optional] The minimum size of the dialog. Must be smaller than the maximum size if both are specified. Should be in pixels.
 * @param hMaxSize [optional] The maximum size of the dialog. Must be bigger than the minimzm size if both are specified. Should be in pixels.
 */
Manager::Manager( HWND hParent, const SIZE& hMinSize, const SIZE& hMaxSize ) :
	m_pMainArea(NULL),
	m_nNextControlID(DYNAMIC_IDC_START_VALUE),
	m_nProfilingMode(ProfileOff),
	m_pSuperWndProc(NULL),
	m_pModalPage(NULL),
	m_pHoveredArea(NULL),
	m_pEditor(NULL)
{
	initMgr(hParent, hMaxSize, hMinSize);
}

/**
 * Alignment Manager Ctor.
 * @param hParent The window whose children are to be aligned
 * @param
 * @param hMinSize [optional] The minimum size of the dialog. Must be smaller than the maximum size if both are specified. Should be in pixels.
 * @param hMaxSize [optional] The maximum size of the dialog. Must be bigger than the minimzm size if both are specified. Should be in pixels.
 */
Manager::Manager( HWND hParent, std::string sLayoutIdentifier, ULONG nProfileMode, const SIZE& hMinSize, const SIZE& hMaxSize ) :
	m_pMainArea(NULL),
	m_nNextControlID(DYNAMIC_IDC_START_VALUE),
	m_sLayoutIdentifier(sLayoutIdentifier),
	m_nProfilingMode((ProfilingMode) nProfileMode),
	m_pSuperWndProc(NULL),
	m_pModalPage(NULL),
	m_pHoveredArea(NULL),
	m_pEditor(NULL)
{
	initMgr(hParent, hMaxSize, hMinSize);
}

/**
 * Alignment Manager Dtor.
 */
Manager::~Manager()
{
	eraseFromManagedMap();

	// Clean up all Control instances
	for each( std::pair<HWND, Control*> hPair in m_mapHwndControl )
		delete hPair.second;

	// Delete Alignment Areas
	delete m_pMainArea;
}

/**
 * Add a control with a specific alignment to be enforced by this manager.
 * @param pCtrl The control to be added
 * @param hAlignHorz The horizontal alignment of the control to be added.
 * @param hAlignVert The vertical alignment of the control to be added.
 * @return True, if the control was successfully added, false if adding failed.
 * Adding the control will fail if the control is already added.
 * It is no longer necessary that the parent of the control is the window self.
 * @about If the control has already been added to the manager previously,
 * the controls current rect will be seen as its original one!
 */
bool Manager::addControl( HWND hCtrl, Align::Mode& hAlignHorz, Align::Mode& hAlignVert, std::string sName )
{
	// first check whether the control is any of the background controls of the alignment areas
	if(m_pMainArea->isBackgroundHwnd(hCtrl) != NULL)
		return false;

	// create a new map entry and remember the iterator
	std::map<HWND, Control*>::iterator it = m_mapHwndControl.insert(
		m_mapHwndControl.begin(), std::pair<HWND, Control*>(hCtrl, NULL)
	);

	// check if the map entry already existed
	if( (*it).second )
	{
		(*it).second->updateOrigRect();
		(*it).second->setHorzAlignment(hAlignHorz.copy());
		(*it).second->setVertAlignment(hAlignVert.copy());
		return false;
	}
	else
	{
		// create the new Control and assign it to the iterator
		Control* pAlignedControl = new Control(this, hCtrl, hAlignHorz, hAlignVert, sName);
		(*it).second = pAlignedControl;
		m_pMainArea->insertIfOwned(pAlignedControl);
		return true;
	}
}

/**
 * Add a dialog item with a specific alignment to be enforced by this manager.
 * @param nID The dialog item ID of the control to be added
 * @param hAlignHorz The horizontal alignment of the control to be added.
 * @param hAlignVert The vertical alignment of the control to be added.
 * @return A Pointer to the CWnd instance of the specified item, or NULL, if adding failed.
 * Adding the control will fail if
 * - the control is already added,
 * - the control's parent is not the window this manager has been created for
 * - the ID is invalid
 */
bool Manager::addControl( UINT nID, Align::Mode& hAlignHorz, Align::Mode& hAlignVert, std::string sName )
{
	HWND hCtrl = ::GetDlgItem(m_hManagedWindow, nID);

	if( hCtrl )
		return this->addControl(hCtrl, hAlignHorz, hAlignVert, sName);

	return false;
}


/**
 * Remove a control whose alignment is not be enforced by this manager anymore.
 * @param pCtrl The control to be removed.
 * @return True, if the control was successfully removed, false if otherwise.
 * Removing will fail if the control was not added.
 */
bool Manager::removeControl( HWND hCtrl )
{
	std::map<HWND, Control*>::iterator it = m_mapHwndControl.find(hCtrl);

	if( it != m_mapHwndControl.end() )
	{
		if(it->second->getArea())
			const_cast<Area*>(it->second->getArea())->removeControl(it->second);

		m_mapHwndControl.erase(it);
		return true;
	}
	else
		return false;
}

/**
 * Get the Alignment of a control that supposedly has been added to the manager.
 * @param hAlignHorz [out]
 * @param hAlignVert [out]
 * @return True, if the control was found, false if otherwise.
 */
bool Manager::getControlAlignment( CWnd* pCtrl, Align::Mode& hAlignHorz, Align::Mode& hAlignVert )
{
	std::map<HWND, Control*>::iterator it = m_mapHwndControl.find(pCtrl->GetSafeHwnd());

	if( it != m_mapHwndControl.end() )
	{
		hAlignHorz = *((*it).second->getHorzAlignment());
		hAlignVert = *((*it).second->getVertAlignment());
		return true;
	}
	else
		return false;
}

/**
 * Update (Enforce) the alignment of all controls added to the manager.
 */
void Manager::update()
{
	if (::IsWindow(getWnd()->GetSafeHwnd()))
	{
		// update current size member
		CRect currentRect;
		getWnd()->GetWindowRect(currentRect);

		// Update the alignment areas (Recursively)
		m_pMainArea->update(currentRect);
	}
}

void Layout::Manager::clampRect(UINT nSide, LPRECT lpRect)
{
	CRect currentRect(lpRect);
	if (m_pMainArea->clampRect(nSide, currentRect))
	{
		lpRect->top = currentRect.top;
		lpRect->bottom = currentRect.bottom;
		lpRect->left = currentRect.left;
		lpRect->right = currentRect.right;
	}
}

Splitter const* Manager::putSplitter( HWND hHigherCtrl, HWND hLowerCtrl, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment )
{
	Control* pHiCtrl = m_mapHwndControl[hHigherCtrl];
	Control* pLoCtrl = m_mapHwndControl[hLowerCtrl];

	if(pHiCtrl && pLoCtrl)
		return const_cast<Area*>(pHiCtrl->getArea())->putSplitter(pHiCtrl, pLoCtrl, nOrientation, nAlignment);
	else
		return NULL;
}

Splitter const* Manager::putSplitter( UINT nHigherId, UINT nLowerId, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment )
{
	HWND hHiCtrl = ::GetDlgItem(m_hManagedWindow, nHigherId);
	HWND hLoCtrl = ::GetDlgItem(m_hManagedWindow, nLowerId);

	return putSplitter(hHiCtrl, hLoCtrl, nOrientation, nAlignment);
}

Splitter const* Manager::putSplitter( AreaProperties& aHi, AreaProperties& aLo, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment )
{
	Splitter const* pSplitter;
	if(aHi.getCtrlId() && aLo.getCtrlId())
		pSplitter = putSplitter(aHi.getCtrlId(), aLo.getCtrlId(), nOrientation, nAlignment);
	else
		pSplitter = putSplitter(aHi.getCtrlWnd(), aLo.getCtrlWnd(), nOrientation, nAlignment);

	AFXASSUME(pSplitter != NULL);

	// Adapt the area property values
	if(pSplitter)
	{
		Area const* pArea = pSplitter->getArea();
		Area const* pHi = pArea->getChildHi();
		Area const* pLo = pArea->getChildLo();
		AFXASSUME(pHi != NULL && pLo != NULL);
		if(pHi && pLo)
		{
			const_cast<Area*>(pHi)->adaptValues(aHi);
			const_cast<Area*>(pLo)->adaptValues(aLo);
		}
	}

	return pSplitter;
}

void Manager::updateAllOrigRect() const
{
	m_pMainArea->updateOrigRect();
}

/***************************************************************
               Managed Layout Hook Infrastructure
 **************************************************************/

HANDLE Manager::s_hManagerMapAccess = NULL;
std::map<HWND, Manager*> Manager::s_mapWndLayoutManager;

void Manager::pushToManagedMap()
{
	// Obtain secure access to the manager map
	if(s_hManagerMapAccess != NULL)
	{
		WaitForSingleObject(s_hManagerMapAccess, WAIT_ABANDONED);
	}
	// Create the Mutex and install the hook
	else
	{
		s_hManagerMapAccess = CreateMutex(NULL, TRUE, "");
		AFXASSUME(s_hManagerMapAccess != NULL);
	}

	// Set the manager for the window handle in the global map
	s_mapWndLayoutManager[m_hManagedWindow] = this;
	ReleaseMutex(s_hManagerMapAccess);

	// Install Window Proc
	m_pSuperWndProc = (WNDPROC) ::SetWindowLong(m_hManagedWindow, GWL_WNDPROC, (LONG) ManagedLayoutWindowProc);
}

void Manager::eraseFromManagedMap()
{
	// Erase the window-manager association from the global map
	WaitForSingleObject(s_hManagerMapAccess, WAIT_ABANDONED);
	s_mapWndLayoutManager.erase(m_hManagedWindow);
	ReleaseMutex(s_hManagerMapAccess);

	// Set the window procedure back to the old one
	::SetWindowLong(m_hManagedWindow, GWL_WNDPROC, (LONG) m_pSuperWndProc);
}

LRESULT CALLBACK Manager::ManagedLayoutWindowProc( _In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam )
{
	Manager* pManager = NULL;
	LRESULT lResult = FALSE;

	// Obtain the manager for the window. Find is thread-safe.
	std::map<HWND, Manager*>::iterator it = s_mapWndLayoutManager.find(hwnd);
	if(it != s_mapWndLayoutManager.end())
		pManager = (*it).second;
	if(pManager == NULL)
		return lResult;

	// Execute the super window procedure
	lResult = (pManager->getSuperWndProc())(hwnd, uMsg, wParam, lParam);

	// Let the manager process the message
	switch(uMsg)
	{
		case WM_SIZE:
		{
			pManager->update();
			lResult = TRUE;
		}
		case WM_CLOSE:
		case WM_DESTROY:
		{
			pManager->storeSizeAndPosition();
			lResult = FALSE;
			break;
		}
		case WM_SIZING:
		{
			pManager->clampRect((UINT)wParam, (LPRECT)lParam);
			lResult = TRUE;
		}
		case WM_MOVE:
		{
			pManager->storeSizeAndPosition();
			lResult = TRUE;
			break;
		}
		case WM_ERASEBKGND:
		{
			HDC hDC = (HDC) wParam;
			pManager->draw(hDC);
			break;
		}
		case WNDMSG_LAYOUT_MODALPAGECLOSED:
		{
			if(pManager->getModalPage() != NULL)
				pManager->closeModalPageWindow();
			break;
		}
		
		// Auskommentiert da das Zeichnen des Hintergrunds Probleme macht
		// und (in VL) sowieso nicht verwendet wird.
		// 
		// case WM_CTLCOLOR:
		// case WM_CTLCOLORBTN:
		// case WM_CTLCOLOREDIT:
		// case WM_CTLCOLORDLG:
		// case WM_CTLCOLORLISTBOX:
		// case WM_CTLCOLORSCROLLBAR:
		// case WM_CTLCOLORSTATIC:
		// {
		// 	HDC hDC = (HDC) wParam;
		// 	HWND hControl = (HWND) lParam;
		// 	Control const* pControl = pManager->getControl(hControl);
		// 	SetBkMode(hDC, TRANSPARENT);
		// 	if(pControl && pControl->getArea()->getCurrentBkColorStyle() != FALSE)
		// 		lResult = (LRESULT) pControl->getArea()->getBrush(pControl->getArea()->getCurrentBkColorStyle());
		// 	break;
		// }
		// case WM_KEYUP:
		// {
		// 	// Ctrl + Shift + Alt + L will open the editor
		// 	if( // Ctrl + Shift + Alt
		// 		(GetKeyState(VK_LSHIFT)&0x8000) != 0 &&
		// 		(GetKeyState(VK_LCONTROL)&0x8000) != 0 &&
		// 		(GetKeyState(VK_LMENU)&0x8000) != 0 )
		// 	{
		// 		if(wParam == 0x4C) // L Key
		// 			pManager->openEditor();
		// 	}
		// }
		// case WM_CONTEXTMENU:
		// {
		// 	pManager->openEditor();
		// }
	}

	return lResult;
}

void Manager::mapDialogSizesLogicalToPhysical( SIZE& hMaxSize, SIZE& hMinSize )
{
	// convert from dialog units to pixels
	RECT rct = {0, 0, (LONG) hMaxSize.cx, (LONG) hMaxSize.cy};
	::MapDialogRect(m_hManagedWindow, &rct);
	hMaxSize.cx = rct.right;
	hMaxSize.cy = rct.bottom;

	rct.right = hMinSize.cx;
	rct.bottom = hMinSize.cy;
	::MapDialogRect(m_hManagedWindow, &rct);
	hMinSize.cx = rct.right;
	hMinSize.cy = rct.bottom;
}

void Manager::initProfiling()
{
	// initialize profiling
	if( m_nProfilingMode > ProfileOff )
	{
		// prefix the dialogsizes root node to the path and append the dialog id
		m_sProfilingPath = KEY_PROFILING_ROOT_NODE;
		m_sProfilingPath.append(".");

		// prefix the application to the registry path if desired
		if( m_nProfilingMode == ProfileApplicationSpecific )
		{
			char pchPath[1024];
			if( ::GetModuleFileName(NULL, pchPath, 1024) )
			{
				const wchar_t* wchFilename = boost::filesystem::path(std::string(pchPath)).stem().c_str();
				char pchFilename[1024];
				wcstombs(pchFilename, wchFilename, 1023);
				m_sProfilingPath.append(pchFilename);
				m_sProfilingPath.append(".");
			}
		}
		m_sProfilingPath.append(m_sLayoutIdentifier);
	}
}

void Manager::storeSizeAndPosition()
{
	if( !m_sProfilingPath.empty() && ::IsWindowVisible(m_hManagedWindow) )
	{
		BOOL bIconic, bMaximized;
		WINDOWPLACEMENT wndpl;

		wndpl.length = sizeof(WINDOWPLACEMENT);
		// gets current window position and
		//  iconized/maximized status
		BOOL bRet = GetWindowPlacement(m_hManagedWindow, &wndpl);
		if (wndpl.showCmd == SW_SHOWNORMAL)
		{
			bIconic = FALSE;
			bMaximized = FALSE;
		}
		else if (wndpl.showCmd == SW_SHOWMAXIMIZED)
		{
			bIconic = FALSE;
			bMaximized = TRUE;
		}
		else if (wndpl.showCmd == SW_SHOWMINIMIZED)
		{
			bIconic = TRUE;
			if (wndpl.flags)
				bMaximized = TRUE;
			else
				bMaximized = FALSE;
		}

		Registry::getInstance()->writeLong(m_sProfilingPath.c_str(), "x", wndpl.rcNormalPosition.left);
		Registry::getInstance()->writeLong(m_sProfilingPath.c_str(), "y", wndpl.rcNormalPosition.top);
		Registry::getInstance()->writeLong(m_sProfilingPath.c_str(), "w", wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left);
		Registry::getInstance()->writeLong(m_sProfilingPath.c_str(), "h", wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top);
		Registry::getInstance()->writeLong(m_sProfilingPath.c_str(), "i", bIconic);
		Registry::getInstance()->writeLong(m_sProfilingPath.c_str(), "m", bMaximized);
	}
}

void Manager::restoreFromProfile()
{
	if(m_sProfilingPath.empty() || m_nProfilingMode == ProfileOff)
		return;

	bool success(true);
	CRect rctRestore;
	long width = 0, heigth = 0, iconic = 0, maximized = 0;
	success &= Registry::getInstance()->readLong(m_sProfilingPath.c_str(), "x", rctRestore.left, 0);
	success &= Registry::getInstance()->readLong(m_sProfilingPath.c_str(), "y", rctRestore.top, 0);
	success &= Registry::getInstance()->readLong(m_sProfilingPath.c_str(), "w", width, 0);
	success &= Registry::getInstance()->readLong(m_sProfilingPath.c_str(), "h", heigth, 0);
	Registry::getInstance()->readLong(m_sProfilingPath.c_str(), "i", iconic, 0); // neu, daher success nicht prüfen
	Registry::getInstance()->readLong(m_sProfilingPath.c_str(), "m", maximized, 0);

	bool bIconic = iconic > 0;
	bool bMaximized = maximized > 0;
	rctRestore.right = rctRestore.left + width;
	rctRestore.bottom = rctRestore.top + heigth;
	rctRestore.NormalizeRect();

	UINT flags;
	WINDOWPLACEMENT wndpl;

	int nCmdShow = SW_SHOWNA;

	if (bIconic)
	{
		if (bMaximized)
			flags = WPF_RESTORETOMAXIMIZED;
		else
			flags = WPF_SETMINPOSITION;
	}
	else
	{
		if (bMaximized)
		{
			flags = WPF_RESTORETOMAXIMIZED;
		}
		else
		{
			flags = WPF_SETMINPOSITION;
		}
	}

	// Test if the window intersects the main display.
	// If not, do not restore position and size
	if (success && Layout::RectIntersectsRect(GetDisplayBounds(), rctRestore))
	{
		// Adjust size to the min/max size of the area
		long width = std::max(m_pMainArea->getMinSize().cx, (long)rctRestore.Width());
		long height = std::max(m_pMainArea->getMinSize().cy, (long)rctRestore.Height());
		if(CSize(m_pMainArea->getMaxSize()) != CSize(NULLSIZE))
		{
			width = std::min(width, m_pMainArea->getMaxSize().cx);
			height = std::min(height, m_pMainArea->getMaxSize().cx);
		}

		// Set the adjusted sizes back to the rect for intersetion rect calculation
		rctRestore.right = rctRestore.left + width;
		rctRestore.bottom = rctRestore.top + height;

		// Calculate the intersection rectangle
		CRect intersection = Layout::RectGetIntersectionRect(GetDisplayBounds(), rctRestore);

		// Offset the new rects position by the x delta
		long deltaX = rctRestore.Width() - intersection.Width();
		long newXPos = rctRestore.left + (intersection.left > 0 ? -deltaX : deltaX);

		// Offset the new rects position by the y delta
		long deltaY = rctRestore.Height() - intersection.Height();
		long newYPos = rctRestore.top + (intersection.top > 0 ? -deltaY : deltaY);

		if (width > 0 && height > 0)
		{
			wndpl.length = sizeof(WINDOWPLACEMENT);
			wndpl.showCmd = nCmdShow;
			wndpl.flags = flags;
			wndpl.ptMinPosition = CPoint(0, 0);
			wndpl.ptMaxPosition =
				CPoint(-::GetSystemMetrics(SM_CXBORDER),
				-::GetSystemMetrics(SM_CYBORDER));
			wndpl.rcNormalPosition = rctRestore;
			// sets window's position and minimized/maximized status
			BOOL bRet = SetWindowPlacement(m_hManagedWindow, &wndpl);
		}
	}

	update();
}

/**
 * Helper function that returns the bounds of the main display
 */
CRect GetDisplayBounds()
{
	CRect displayRect;
	::GetWindowRect(::GetDesktopWindow(), &displayRect);
	return displayRect;
}

void Layout::Manager::draw( HDC hDC )
{
	m_pMainArea->draw(hDC);
}

Control const* Layout::Manager::getControl(HWND hCtrl)
	{
	std::map<HWND, Control*>::iterator it = m_mapHwndControl.find(hCtrl);
	if( it != m_mapHwndControl.end() )
		return it->second;
	else
		return NULL;
}

void Layout::Manager::initMgr( HWND hParent, const SIZE& hMaxSize, const SIZE& hMinSize )
{
	AFXASSUME(hParent);
	m_hManagedWindow = hParent;

	CRect hRect;
	::GetWindowRect(hParent, &hRect);

	SIZE hPhysicalMinSize = hMinSize, hPhysicalMaxSize = hMaxSize;
	mapDialogSizesLogicalToPhysical(hPhysicalMinSize, hPhysicalMaxSize);

	m_pMainArea = new Area(this, hRect, hPhysicalMinSize, hPhysicalMaxSize);
	pushToManagedMap();

	initProfiling();

	// Initialize font metrics info
	::ZeroMemory(&m_textMetric, sizeof(TEXTMETRIC));
	::GetTextMetrics(::GetDC(hParent), &m_textMetric);

	// Create this Managed Layouts Editor
	// m_pEditor = new Layout::Editor(this);
}

void Layout::Manager::putModalPage( Layout::Owner const* pPage )
{
	AFXASSUME(pPage->getHwnd() != getHwnd());
	if(pPage->getHwnd() != getHwnd() && getModalPage() == NULL)
	{
		// Create the layout window
		CRect hRect;
		::GetWindowRect(getHwnd(), &hRect);
		m_pModalPage = new Window(*pPage, this, hRect, NULLSIZE, NULLSIZE);
		m_pMainArea->setVisible(false);
	}
}

void Layout::Manager::closeModalPageWindow()
{
	AFXASSUME(m_pModalPage != NULL && m_pMainArea->isVisible() == false);
	if(m_pModalPage != NULL && m_pMainArea->isVisible() == false)
	{
		delete m_pModalPage;
		m_pModalPage = NULL;
		m_pMainArea->setVisible(true);
	}
}

void Layout::Manager::setHoveredArea( Area* pArea ) const
{
	if(pArea != m_pHoveredArea)
	{
		if(m_pHoveredArea != NULL)
		{
			Area* pPrevHoveredArea = const_cast<Area*>(m_pHoveredArea);
			m_pHoveredArea = NULL; // Set to Null so when the area calls setHoveredArea(NULL) we don't run into a cyclic call
			pPrevHoveredArea->OnMouseLeave();
		}

		if(pArea != NULL)
			m_pHoveredArea = pArea;
	}
}

void Layout::Manager::getControls( __out std::set<Control const*>& aControlSet ) const
{
	for each( std::pair<HWND, Control*> const& aPair in m_mapHwndControl )
		aControlSet.insert(aPair.second);
}

void Layout::Manager::openEditor()
{
	if(m_pEditor != NULL)
		m_pEditor->ShowWindow(SW_SHOW);
}

