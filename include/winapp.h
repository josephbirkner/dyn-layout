// efwdlg.h : main header file for the EFWDLG DLL
//

#if !defined(__DYNLAYOUT_WINAPP_H__)
#define __DYNLAYOUT_WINAPP_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// DynLayoutApp
// See efwdlg.cpp for the implementation of this class
// 

class DynLayoutApp : public CWinApp
{
public:
	/////////////////////////////////////////////////////////////////////////////
	// The one and only DynLayoutApp object
	static DynLayoutApp theApp;
	
	DynLayoutApp();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DynLayoutApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(DynLayoutApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//	  DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(__DYNLAYOUT_WINAPP_H__)

