//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxWindow.cpp
// implementation: Win32 API
// last modified:  Jun 01 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxWindow.h>
#include <mx/mxInit.h>
#include <mx/mxLinkedList.h>
#include <windows.h>
//#include <ostream.h>
#include <vector>
using namespace std;


#define CLASSNAME "MXTK::CLASSNAME"

extern mxWindow *g_mainWindow;



class mxWindow_i
{
public:
	UINT d_uTimer;
	int d_dlgRet;
	bool d_dlgRunning;
	vector<mxWidget *> d_widgetList;
};



mxWindow::mxWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style)
: mxWidget (parent, x, y, w, h, label)
{
	d_this = new mxWindow_i;
	d_this->d_uTimer = 0;
	d_this->d_dlgRet = 0;
	d_this->d_dlgRunning = false;

	DWORD dwStyle;
	DWORD dwStyleEx = 0;
	if (style == Normal)
		dwStyle = WS_OVERLAPPEDWINDOW;
	else if (style == Popup)
		dwStyle = WS_POPUP;
	else if (style == Dialog)
		dwStyle = WS_DLGFRAME | WS_CAPTION;
	else if (style == ModalDialog)
		 dwStyle = WS_DLGFRAME | WS_CAPTION;

	void *parentHandle = 0;
	if (parent)
	{
		parentHandle = parent->getHandle ();
		dwStyle = WS_CHILD | WS_VISIBLE/* | WS_CLIPCHILDREN | WS_CLIPSIBLINGS*/ | WS_GROUP | WS_TABSTOP;
		//dwStyleEx = WS_EX_CONTROLPARENT;
	}
	else
	{
		x = y = CW_USEDEFAULT;
	}

	void *handle = (void *) CreateWindowEx (dwStyleEx, CLASSNAME, label, dwStyle,
					x, y, w, h, (HWND) parentHandle,
					(HMENU) NULL, (HINSTANCE) GetModuleHandle (NULL), NULL);

	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_WINDOW);
	setParent (parent);
	if (style == ModalDialog)
		setType (MX_DIALOGWINDOW);

	if (!parent && !g_mainWindow)
		g_mainWindow = this;

	if (parent)
		parent->addWidget(this);

	//MC: d_widgetList = new mxLinkedList ();
}



mxWindow::~mxWindow ()
{
/*MC: 
	mxListNode *node = d_widgetList->getFirst ();
	while (node)
	{
		mxWidget *widget = (mxWidget *) d_widgetList->getData (node);
		delete widget;
		node = d_widgetList->getNext (node);
	}
	d_widgetList->removeAll ();
	delete d_widgetList;
*/
	int nSize = d_this->d_widgetList.size();
	for (int i = 0; i < nSize; i++)
	{
		delete d_this->d_widgetList[i];
	}
	d_this->d_widgetList.clear();

	SetWindowLong ((HWND) getHandle (), GWL_USERDATA, (LONG) 0);
	delete d_this;
}



void
mxWindow::addWidget (mxWidget *widget)
{
	if (d_this->d_widgetList.empty())
	//MC: if (d_widgetList->isEmpty ())
	{
		HWND hWnd = (HWND) widget->getHandle ();
		if (::IsWindow(hWnd))
		{
			LONG l = GetWindowLong (hWnd, GWL_STYLE);
			l |= WS_GROUP;
			SetWindowLong (hWnd, GWL_STYLE, l);
		}
	}
	//MC: d_widgetList->add (widget);
	d_this->d_widgetList.push_back(widget);
}



int
mxWindow::handleEvent (mxEvent *event)
{
	return 0;
}



void
mxWindow::redraw ()
{
}



void
mxWindow::setTimer (int milliSeconds)
{
	if (d_this->d_uTimer)
	{
		KillTimer ((HWND) getHandle (), d_this->d_uTimer);
		d_this->d_uTimer = 0;
	}

	if (milliSeconds > 0)
	{
		d_this->d_uTimer = 21001;
		d_this->d_uTimer = SetTimer ((HWND) getHandle (), d_this->d_uTimer, milliSeconds, NULL);
	}
}



void
mxWindow::setMenuBar (mxMenuBar *menuBar)
{
	SetMenu ((HWND) getHandle (), (HMENU) ((mxWidget *) menuBar)->getHandle ());
}



int
mxWindow::doModal ()
{
	d_this->d_dlgRet = 0;
	d_this->d_dlgRunning = true;
	MSG msg;

	mx::setEventWindow (this);
	setVisible (true);

	// acquire and dispatch messages until the modal state is done
	for (;;)
	{
#if 0
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
#endif
		// phase2: pump messages while available
		do
		{
#if 0
			// pump message, but quit on WM_QUIT
			if (!AfxGetThread()->PumpMessage())
			{
				AfxPostQuitMessage(0);
				return -1;
			}
#endif
			if (GetMessage (&msg, 0, 0, 0))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
			else
			{
				PostQuitMessage (0);
				return -1;
			}

			// show the window when certain special messages rec'd
			if (/*bShowIdle &&*/
				(msg.message == 0x118 || msg.message == WM_SYSKEYDOWN))
			{
				ShowWindow ((HWND) getHandle (), SW_SHOWNORMAL);
				UpdateWindow ((HWND) getHandle ());
				//bShowIdle = FALSE;
			}

			if (!d_this->d_dlgRunning)
				goto ExitModal;
#if 0
			// reset "no idle" state after pumping "normal" message
			if (AfxGetThread()->IsIdleMessage(pMsg))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}
#endif
		} while (PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE));
	}

ExitModal:

	PostMessage ((HWND) getHandle (), WM_NULL, 0, 0L);

	return d_this->d_dlgRet;
}



void
mxWindow::endDialog (int retValue)
{
	d_this->d_dlgRet = retValue;
	d_this->d_dlgRunning = false;
	mx::setEventWindow (0);
	setVisible (false);
}
