// ServerCtrlDlg.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>
#include <mmsystem.h>
#include "ServerCtrl.h"
#include "ServerCtrlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Each ENGINE command token is a 32 bit integer

#define ENGINE_ISSUE_COMMANDS				0x2
// Param1 : char *		text to issue

#define ENGINE_RETRIEVE_CONSOLE_CONTENTS	0x3
// Param1 : int32		Begin line
// Param2 : int32		End line

#define ENGINE_RETRIEVE_GET_CONSOLE_HEIGHT	0x4
// No params

#define ENGINE_RETRIEVE_SET_CONSOLE_HEIGHT	0x5
// Param1 : int32		Number of lines

/////////////////////////////////////////////////////////////////////////////
// CServerCtrlDlg dialog

CServerCtrlDlg::CServerCtrlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerCtrlDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerCtrlDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	memset( &PI, 0, sizeof( PI ) );

	m_nPendingRequest	= 0;
	m_nPendingLines		= 0;

	m_hMappedFile		= (HANDLE)0;
	m_hSend				= (HANDLE)0;
	m_hReceive			= (HANDLE)0;

	m_bOnlyPumpIfMessageInQueue = FALSE;
}

void CServerCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerCtrlDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServerCtrlDlg, CDialog)
	//{{AFX_MSG_MAP(CServerCtrlDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, OnBtnStart)
	ON_BN_CLICKED(IDC_BTN_EXECUTE, OnBtnExecute)
	ON_BN_CLICKED(IDC_BTN_GET, OnBtnGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CServerCtrlDlg::RunModalLoop(DWORD dwFlags)
{
	ASSERT(::IsWindow(m_hWnd)); // window must be created
	ASSERT(!(m_nFlags & WF_MODALLOOP)); // window must not already be in modal state

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;
	BOOL bShowIdle = (dwFlags & MLF_SHOWONIDLE) && !(GetStyle() & WS_VISIBLE);
	HWND hWndParent = ::GetParent(m_hWnd);
	m_nFlags |= (WF_MODALLOOP|WF_CONTINUEMODAL);
	MSG* pMsg = &AfxGetThread()->m_msgCur;

	// acquire and dispatch messages until the modal state is done
	for (;;)
	{
		ASSERT(ContinueModal());

		int iRet = RMLPreIdle();

		if (iRet < 0)
			goto ExitModal;
		else if (iRet > 0)
			continue;

		// phase1: check to see if we can do idle work
		while (bIdle &&
			!::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			ASSERT(ContinueModal());

			// show the dialog when the message queue goes idle
			if (bShowIdle)
			{
				ShowWindow(SW_SHOWNORMAL);
				UpdateWindow();
				bShowIdle = FALSE;
			}

			// call OnIdle while in bIdle state
			if (!(dwFlags & MLF_NOIDLEMSG) && hWndParent != NULL && lIdleCount == 0)
			{
				// send WM_ENTERIDLE to the parent
				::SendMessage(hWndParent, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)m_hWnd);
			}
			if ((dwFlags & MLF_NOKICKIDLE) ||
				!SendMessage(WM_KICKIDLE, MSGF_DIALOGBOX, lIdleCount++))
			{
				// stop idle processing next time
				bIdle = FALSE;
			}
		}

		// phase2: pump messages while available
		do
		{
			BOOL ShouldPump = TRUE;

			ASSERT(ContinueModal());

			// See if we are requiring messages to be in queue?
			if ( m_bOnlyPumpIfMessageInQueue )
			{
				// If there isn't a message, don't turn over control to PumpMessage
				//  since it will block
				if ( !::PeekMessage( pMsg, NULL, NULL, NULL, PM_NOREMOVE ) )
				{
					ShouldPump = FALSE;
				}
			}

			// pump message, but quit on WM_QUIT
			if ( ShouldPump )
			{
				if (!AfxGetThread()->PumpMessage())
				{
					AfxPostQuitMessage(0);
					return -1;
				}

				// show the window when certain special messages rec'd
				if (bShowIdle &&
					(pMsg->message == 0x118 || pMsg->message == WM_SYSKEYDOWN))
				{
					ShowWindow(SW_SHOWNORMAL);
					UpdateWindow();
					bShowIdle = FALSE;
				}

				if (!ContinueModal())
					goto ExitModal;

				// reset "no idle" state after pumping "normal" message
				if (AfxGetThread()->IsIdleMessage(pMsg))
				{
					bIdle = TRUE;
					lIdleCount = 0;
				}
			}

		} while (::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE));
	}
ExitModal:
	m_nFlags &= ~(WF_MODALLOOP|WF_CONTINUEMODAL);
	return m_nModalResult;
}

int CServerCtrlDlg::DoModal()
{
	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = AfxGetResourceHandle();
	if (m_lpszTemplateName != NULL)
	{
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	// return -1 in case of failure to load the dialog template resource
	if (lpDialogTemplate == NULL)
		return -1;

	// disable parent (before creating dialog)
	HWND hWndParent = PreModal();
	AfxUnhookWindowCreate();
	BOOL bEnableParent = FALSE;
	if (hWndParent != NULL && ::IsWindowEnabled(hWndParent))
	{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
	}

	TRY
	{
		// create modeless dialog
		AfxHookWindowCreate(this);
		if (CreateDlgIndirect(lpDialogTemplate,
						CWnd::FromHandle(hWndParent), hInst))
		{
			if (m_nFlags & WF_CONTINUEMODAL)
			{
				// enter modal loop
				DWORD dwFlags = MLF_SHOWONIDLE;
				if (GetStyle() & DS_NOIDLEMSG)
					dwFlags |= MLF_NOIDLEMSG;
				VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
			}

			// hide the window before enabling the parent, etc.
			if (m_hWnd != NULL)
				SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
					SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
	CATCH_ALL(e)
	{
		//DELETE_EXCEPTION(e);
		m_nModalResult = -1;
	}
	END_CATCH_ALL

	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);

	// destroy modal window
	DestroyWindow();
	PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	return m_nModalResult;
}

void CServerCtrlDlg::SetPumpIfQueued( BOOL bValue )
{
	m_bOnlyPumpIfMessageInQueue = bValue;
}

/////////////////////////////////////////////////////////////////////////////
// CServerCtrlDlg message handlers

BOOL CServerCtrlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	SetPumpIfQueued( TRUE );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerCtrlDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CServerCtrlDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/////////////////////////////////////
/////////////////////////////////////
// IMPORTANT STUFF STARTS HERE
/////////////////////////////////////

/*
=====================
ProcessMappedResponse

The engine has signaled the receive event, see what the results of the command we issued are.
=====================
*/
int CServerCtrlDlg::ProcessMappedResponse( void )
{
	char *psz;
	int *pBuffer;
	int i;
	char line[ 1024 ];
	char szwindow[ 4096 ];
	CEdit *pEdit;

	pBuffer = (int *)GetMappedBuffer ( m_hMappedFile );
	
	// buffer is invalid.  Just leave.
	if ( !pBuffer )
	{
		return 0;
	}

	// Will be non-zero upon success
	if ( pBuffer[0] )
	{
		switch ( m_nPendingRequest )
		{
		case ENGINE_RETRIEVE_CONSOLE_CONTENTS:
			// Write text lines to console area
			pEdit = (CEdit *)GetDlgItem( IDC_EDIT_CONSOLE );
			if ( pEdit )
			{
				szwindow[ 0 ] = '\0';
				
				for ( i = 0; i < m_nPendingLines; i++ )
				{
					// Skip first int32 result code
					// Lines are assumed to be 80 characters wide
					psz = (char *)( pBuffer + 1 ) + 80 * i;
					strncpy( line, psz, 80 );
					line[ 79 ] = '\0';

					strcat( szwindow, line );
					if ( i != ( m_nPendingLines ) - 1 )
					{
						strcat( szwindow, "\r\n" );
					}
				}

				// Send to display control
				pEdit->SetWindowText( szwindow );
			}
			break;

		default:
			break;
		}
	}

	// Reset results stuff
	m_nPendingRequest	= 0;
	m_nPendingLines		= 0;

	// Free up buffer pointer
	ReleaseMappedBuffer( pBuffer );

	return 1;
}

/*
==============
GetMappedBuffer

==============
*/
LPVOID CServerCtrlDlg::GetMappedBuffer (HANDLE hfileBuffer)
{
	LPVOID pBuffer;
	pBuffer = MapViewOfFile (hfileBuffer, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	return pBuffer;
}

/*
==============
ReleaseMappedBuffer

==============
*/
void CServerCtrlDlg::ReleaseMappedBuffer (LPVOID pBuffer)
{
	UnmapViewOfFile (pBuffer);
}

/*
==============
OnBtnStart

==============
*/
void CServerCtrlDlg::OnBtnStart( void ) 
{
	STARTUPINFO SI;

	// Still active?  Or did user kill it or did it crash
	if ( PI.hProcess )
	{
		DWORD dwEC;
		if ( GetExitCodeProcess( PI.hProcess, &dwEC ) )
		{
			if ( dwEC == STILL_ACTIVE )
			{
				return;
			}
			else
			{
				memset( &PI, 0, sizeof( PI ) );
				CloseHandles();
			}
		}
	}

	// Really still active
	if ( PI.hProcess || m_hMappedFile || m_hSend || m_hReceive )
		return;

	// Startup dedicated server
	SECURITY_ATTRIBUTES SA;
	memset( &SA, 0, sizeof( SA ) );
	SA.nLength = sizeof( SA );

	// HLDS must be able to inherit the handles we pass via command line so we need to mark the handle as inheritable
	SA.bInheritHandle = TRUE;
	SA.lpSecurityDescriptor = NULL;

	// Create handles && hlds process
	m_hMappedFile = CreateFileMapping( (HANDLE)0xFFFFFFFF, &SA, PAGE_READWRITE, 0, 16384, NULL );
	if ( !m_hMappedFile )
	{
		AfxMessageBox( "Couldn't create mapped file", MB_OK );
		return;
	}

	// Uses same security attributes to make handle inheritable
	m_hSend		= CreateEvent( &SA, FALSE, FALSE, NULL );
	m_hReceive	= CreateEvent( &SA, FALSE, FALSE, NULL );

	memset( &SI, 0, sizeof( SI ) );
	SI.cb = sizeof( SI );

	memset( &PI, 0, sizeof( PI ) );

	char sz[ 256 ];
	char szdir[ 256 ];

	// FIXME:  You'll want to fill in your executable path here, of course.
	// The key thing is to invoke the engine using the three HANDLES that were just created
	sprintf( szdir, "d:\\quiver" );
	sprintf( sz, "%s\\hlds.exe +sv_lan 1 -HFILE %i -HPARENT %i -HCHILD %i", szdir, (int)m_hMappedFile, (int)m_hSend, (int)m_hReceive );

	// Run it
	if ( !CreateProcess( NULL, sz, &SA, NULL, TRUE, 0, NULL, szdir, &SI, &PI ) )
	{
		AfxMessageBox( "Couldn't create dedicated server process?  Correct path", MB_OK );
	}
}

/*
==============
~CServerCtrlDlg

==============
*/
CServerCtrlDlg::~CServerCtrlDlg( void )
{
	// Quitting the front end can kill HLDS, if you want it to, since we created that process
	if ( PI.hProcess )
	{
		// Kill process
		TerminateProcess( PI.hProcess, 0 );
		memset( &PI, 0, sizeof( PI ) );
	}

	// Close remaining handles.
	CloseHandles();
}

/*
==============
OnBtnExecute

User wants to issue commands
==============
*/
void CServerCtrlDlg::OnBtnExecute( void ) 
{
	if ( !m_hSend || !m_hMappedFile )
		return;

	// Get commands
	CEdit *pEdit = (CEdit *)GetDlgItem( IDC_EDIT_COMMANDS );
	if ( pEdit )
	{
		CString cmds;
		pEdit->GetWindowText( cmds );

		// Get buffer
		char *sz = (char *)GetMappedBuffer( m_hMappedFile );
		if ( sz )
		{
			// Write command int32 token
			*(int *)&sz[ 0 ] = ENGINE_ISSUE_COMMANDS;

			// Write rest of line, including a return character ( necessary? )
			sprintf( sz + sizeof( int ), "%s\n", (char *)(LPCSTR)cmds );

			// Release the buffer
			ReleaseMappedBuffer( sz );

			// Store off what we are expecting in return buffer
			m_nPendingRequest = ENGINE_ISSUE_COMMANDS;

			// Signal HLDS to check for requests
			SetEvent( m_hSend );
		}
	}
}

/*
==============
OnBtnGet

==============
*/
void CServerCtrlDlg::OnBtnGet( void ) 
{
	RefreshText();
}

/*
==============
CloseHandles

==============
*/
void CServerCtrlDlg::CloseHandles( void )
{
	if (m_hMappedFile )
	{
		CloseHandle( m_hMappedFile );
		m_hMappedFile = ( HANDLE)0;
	}
	
	if ( m_hSend )
	{
		CloseHandle( m_hSend );
		m_hSend = (HANDLE)0;
	}

	if ( m_hReceive )
	{
		CloseHandle( m_hReceive );
		m_hReceive = (HANDLE)0;
	}
}

/*
==============
RefreshText

==============
*/
void CServerCtrlDlg::RefreshText( void )
{
	if ( !m_hSend || !m_hMappedFile )
		return;

	int i = 0;
	char *sz = (char *)GetMappedBuffer( m_hMappedFile );
	if ( sz )
	{
		// Command token
		*(int *)&sz[ i ] = ENGINE_RETRIEVE_CONSOLE_CONTENTS;
		i += sizeof( int );

		// Start at line 0
		*(int *)&sz[ i ] = 0;
		i += sizeof( int );

		// End at line 23 ( assumes 24 line console )
		*(int *)&sz[ i ] = 23;
		i += sizeof( int );

		// Done creating commands
		ReleaseMappedBuffer( sz );

		// Store off pending state info
		m_nPendingRequest = ENGINE_RETRIEVE_CONSOLE_CONTENTS;
		m_nPendingLines = 23 - 0 + 1;

		// Signal HLDS that we have written commands to the buffer
		SetEvent( m_hSend );
	}
}

/*
==============
RMLPreIdle

Called every "frame" by the modal dialog loop
==============
*/
int CServerCtrlDlg::RMLPreIdle(void)
{
	static  DWORD lastupdate;
	DWORD currenttime;

	// Haven't started up HLDS
	if ( !m_hReceive || !m_hSend || !m_hMappedFile )
		return 0;

	// Refresh console text every 1/2 second
	currenttime = timeGetTime();
	if ( ( currenttime - lastupdate ) > 500 )
	{
		lastupdate = currenttime;
		
		RefreshText();
	}

	// Has event fired? Signaling that we have received a response from the engine?
	if ( WaitForSingleObject( m_hReceive, 0 ) == WAIT_OBJECT_0 )
	{
		// Process response
		ProcessMappedResponse();
	}

	return 0;
}