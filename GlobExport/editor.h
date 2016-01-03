#ifndef __LAYOUT_EDITOR_H__
#define __LAYOUT_EDITOR_H__

#include "StdAfx.h"

#include "resource.h"

#include "owner.h"

namespace Layout
{
	class Editor : public CDialog, public Layout::Owner
	{
		DECLARE_DYNAMIC(Editor);

	public:
		Editor(Manager const* pManager);
		virtual ~Editor();
		
		enum { IDD = IDD_DYNLAYOUT_EDITOR };
		
	protected:
		virtual void doLayoutDataExchange();
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
		virtual BOOL OnInitDialog();
		virtual void OnShowWindow(BOOL bShow, UINT nStatus);

		void updateControlList();
		void installEditorWndProcOnControls(bool bInstall);

		DECLARE_MESSAGE_MAP()
		
		CListBox c_LControls;
		std::set<Control const*> m_aControlSet;
	
	private:
		Manager const* m_pManager;
	};
}

#endif