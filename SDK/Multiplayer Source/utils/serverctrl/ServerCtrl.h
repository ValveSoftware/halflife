// ServerCtrl.h : main header file for the ServerCtrl application
//

#if !defined(AFX_ServerCtrl_H__E2974CA6_EF9F_11D3_A4D9_00105A1727F3__INCLUDED_)
#define AFX_ServerCtrl_H__E2974CA6_EF9F_11D3_A4D9_00105A1727F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CServerCtrlApp:
// See ServerCtrl.cpp for the implementation of this class
//

class CServerCtrlApp : public CWinApp
{
public:
	CServerCtrlApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerCtrlApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CServerCtrlApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ServerCtrl_H__E2974CA6_EF9F_11D3_A4D9_00105A1727F3__INCLUDED_)
