// ServerCtrlDlg.h : header file
//

#if !defined(AFX_ServerCtrlDLG_H__E2974CA8_EF9F_11D3_A4D9_00105A1727F3__INCLUDED_)
#define AFX_ServerCtrlDLG_H__E2974CA8_EF9F_11D3_A4D9_00105A1727F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CServerCtrlDlg dialog

class CServerCtrlDlg : public CDialog
{
// Construction
public:
	CServerCtrlDlg( CWnd* pParent = NULL );	// standard constructor
	~CServerCtrlDlg( void );

	void			CloseHandles( void );

	virtual int		RMLPreIdle(void);

	int				RunModalLoop(DWORD dwFlags);
	int				DoModal( void );

	void			SetPumpIfQueued( BOOL bValue );

	void			RefreshText( void );

	int				ProcessMappedResponse( void );
	LPVOID			GetMappedBuffer ( HANDLE hfileBuffer );
	void			ReleaseMappedBuffer ( LPVOID pBuffer );

// Dialog Data
	//{{AFX_DATA(CServerCtrlDlg)
	enum { IDD = IDD_SERVERCTRL_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerCtrlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CServerCtrlDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnStart();
	afx_msg void OnBtnExecute();
	afx_msg void OnBtnGet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	PROCESS_INFORMATION		PI;								// Information about the HLDS process ( mostly the HANDLE )
	HANDLE					m_hMappedFile;					// Shared file for sending/receiving commands to HLDS
	HANDLE					m_hSend;						// Event that will be signaled when we have written commands into mapped file
	HANDLE					m_hReceive;						// Engine will set this when it has readied response to our commands

	int						m_nPendingRequest;				// The last request we issued
	int						m_nPendingLines;				// Number of console lines we want to receive

	BOOL					m_bOnlyPumpIfMessageInQueue;	// TRUE if we should only go into PumpMessage ( which blocks ) if we have seen a message in the MSG queue using PeekMessage
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ServerCtrlDLG_H__E2974CA8_EF9F_11D3_A4D9_00105A1727F3__INCLUDED_)
