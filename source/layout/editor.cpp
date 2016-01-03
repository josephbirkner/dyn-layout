
#include "StdAfx.h"

#pragma hdrstop

#include "../../GlobExport/editor.h"
#include "../../GlobExport/manager.h"

using namespace Layout;

/**
 * Editor-Dialogfeld
 */
IMPLEMENT_DYNAMIC(Editor, CDialog)

BEGIN_MESSAGE_MAP(Editor, CDialog)
ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

//////////////////////////////////////////
// 
//   Global Dialog Functionality
//

/**
 * Editor::Editor
 */
Layout::Editor::Editor(Manager const* pManager) :
	CDialog( Editor::IDD, CWnd::FromHandle(pManager->getHwnd()) ),
	Layout::Owner("DynLayoutEditor"),
	m_pManager(pManager)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	Create(IDD_DYNLAYOUT_EDITOR, m_pManager->getWnd());
}
	
/**
 * Editor::~Editor
 */
Layout::Editor::~Editor()
{
}

void Layout::Editor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DYNLAYOUT_CONTROLLIST, c_LControls);
}

/**
 * Editor::OnInitDialog
 */
BOOL Layout::Editor::OnInitDialog()
{
	CDialog::OnInitDialog();
	initLayout(GetSafeHwnd());
	
	updateControlList();

	return TRUE;
}

/**
 * Editor::doLayoutDataExchange
 */
void Layout::Editor::doLayoutDataExchange()
{
	// Add all controls to the layout
	LAYOUT_CONTROL_VSHS(IDC_DYNLAYOUT_CONTROLLIST);
}

void Layout::Editor::updateControlList()
{
	// Fill Control List
	c_LControls.ResetContent();
	m_aControlSet.clear();
	m_pManager->getControls(m_aControlSet);
	for each(Control const* pControl in m_aControlSet)
	{
		int iItem = c_LControls.InsertString(-1, pControl->getName().c_str());
		c_LControls.SetItemData(iItem, (DWORD_PTR) pControl);
	}
}

void Layout::Editor::OnShowWindow( BOOL bShow, UINT nStatus )
{
	if(bShow)
	{
		updateControlList();
		installEditorWndProcOnControls(true);
	}
	else
		installEditorWndProcOnControls(false);
}

void Layout::Editor::installEditorWndProcOnControls( bool bInstall )
{
	for each(Control const* pControl in m_aControlSet)
	{
		pControl->installEditorWndProc(bInstall);
	}
}
