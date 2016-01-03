#include "stdafx.h"
#pragma hdrstop

#include "../../GlobExport/owner.h"

using namespace Layout;

/// Standard ctor, will set m_pAlignmentManager to NULL
Owner::Owner(std::string sLayoutOwnerName, ProfilingMode nProfilingMode, SIZE sizeMin, SIZE sizeMax)
{
	m_nProfilingMode = nProfilingMode;
	m_sizeMin = sizeMin;
	m_sizeMax = sizeMax;
	m_bInitializing = false;
	m_pManager = NULL;
	m_sLayoutName = sLayoutOwnerName;
}

/// Standard ctor, will set m_pAlignmentManager to NULL
Owner::Owner()
{
	m_nProfilingMode = ProfileOff;
	m_sizeMin = NULLSIZE;
	m_sizeMax = NULLSIZE;
	m_bInitializing = false;
	m_pManager = NULL;
}

/// Standard Dtor, will delete m_pAlignmentManager
Owner::~Owner()
{
	delete m_pManager;
	m_pManager = NULL;
}

#if defined(_DEBUG)
#define VERIFY_ALIGNENDDLG_INITIALIZING() if(!this->m_bInitializing) \
::MessageBox( NULL, CString( \
"It seems you are calling setLayout() or putSplitter()\r\n \
outside your implementation of Owner::doLayoutDataExchange().\r\n \
I strongly recommend not to do that! ~JBI"), CString("Don't do this!"), MB_OK);
#else
	#define VERIFY_ALIGNENDDLG_INITIALIZING()
#endif

bool Owner::setLayout(UINT nID, std::string sID, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment)
{
	VERIFY_ALIGNENDDLG_INITIALIZING()
	if( m_pManager )
		return m_pManager->addControl(nID, hHorzAlignment, hVertAlignment, sID);
	else
		return false;
}

bool Owner::setLayout(HWND hCtrl, std::string sID, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment)
{
	VERIFY_ALIGNENDDLG_INITIALIZING()
	if( m_pManager )
		return m_pManager->addControl(hCtrl, hHorzAlignment, hVertAlignment, "");
	else
		return false;
}

bool Owner::setLayout(UINT nID, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment)
{
	return setLayout(nID, "", hHorzAlignment, hVertAlignment);
}

bool Owner::setLayout(HWND hCtrl, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment)
{
	return setLayout(hCtrl, "", hHorzAlignment, hVertAlignment);
}

bool Layout::Owner::layoutAll(HWND hParent, Align::Mode& hHorzAlignment, Align::Mode& hVertAlignment)
{
	VERIFY_ALIGNENDDLG_INITIALIZING()
	if( m_pManager )
	{
		bool result(true);
		for (HWND wnd = ::GetWindow(hParent, GW_CHILD); result && wnd != NULL; wnd = ::GetNextWindow(wnd, GW_HWNDNEXT))
			result |= m_pManager->addControl(wnd, hHorzAlignment, hVertAlignment, "");
		return result;
	}
	else
		return false;
}

Splitter const* Owner::putSplitter(HWND hHigherCtrl, HWND hLowerCtrl, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment)
{
	VERIFY_ALIGNENDDLG_INITIALIZING()
	if(m_pManager)
		return m_pManager->putSplitter(hHigherCtrl, hLowerCtrl, nOrientation, nAlignment);
	else
		return NULL;
}

Splitter const* Owner::putSplitter(UINT nHigherCtrlId, UINT nLowerCtrlId, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment)
{
	VERIFY_ALIGNENDDLG_INITIALIZING()
	if(m_pManager)
		return m_pManager->putSplitter(nHigherCtrlId, nLowerCtrlId, nOrientation, nAlignment);
	else
		return NULL;
}

Splitter const* Owner::putSplitter(AreaProperties& aHighAreaParams, AreaProperties& aLowAreaParams, Splitter::Orientation nOrientation, Splitter::SplitterAlignment nAlignment)
{
	VERIFY_ALIGNENDDLG_INITIALIZING()
	if(m_pManager)
		return m_pManager->putSplitter(aHighAreaParams, aLowAreaParams, nOrientation, nAlignment);
	else
		return NULL;
}

#undef VERIFY_ALIGNENDDLG_INITIALIZING

void Owner::setManager(Manager* pAlignmentManager)
{
	if( m_pManager != pAlignmentManager )
		delete m_pManager;
	
	m_pManager = pAlignmentManager;
}

void Owner::callDataExchange()
{
	m_bInitializing = true;
	doLayoutDataExchange();
	m_bInitializing = false;
}

void Owner::initLayout( HWND hLayoutParentWindow )
{
	if( !getManager() || !::IsWindow(getManager()->getWnd()->GetSafeHwnd()) )
	{
		setManager(new Manager(hLayoutParentWindow, m_sLayoutName, m_nProfilingMode, m_sizeMin, m_sizeMax));
	}
	
	callDataExchange();
		
	getManager()->restoreFromProfile();
}

LAYOUT_API HWND Owner::getHwnd() const
{
	return m_pManager->getHwnd();
}

LAYOUT_API void Layout::Owner::putModalPage( Layout::Owner const* pPage )
{
	m_pManager->putModalPage(pPage);
}
