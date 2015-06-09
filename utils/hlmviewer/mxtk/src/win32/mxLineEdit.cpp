//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxLineEdit.cpp
// implementation: Win32 API
// last modified:  May 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxLineEdit.h>
#include <mx/mxWindow.h>
#include <windows.h>



class mxLineEdit_i
{
public:
	int dummy;
	//WNDPROC wndProc;
};
/*

static LRESULT CALLBACK mxLineEdit_WindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	mxLineEdit *pEdit = (mxLineEdit *) GetWindowLong(hwnd, GWL_USERDATA);
	if (uMsg == WM_CHAR)
	{
		if (wParam == 13)
		{
			//pEdit->getParent()->
		}
	}
	return ::CallWindowProc(pEdit->d_this->wndProc, hwnd, uMsg, wParam, lParam);
}
 */
mxLineEdit::mxLineEdit (mxWindow *parent, int x, int y, int w, int h, const char *label, int id, int style)
: mxWidget (parent, x, y, w, h, label)
{
	if (!parent)
		return;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	if (style == ReadOnly)
		dwStyle |= ES_READONLY;
	else if (style == Password)
		dwStyle |= ES_PASSWORD;

	void *handle = (void *) CreateWindowEx (WS_EX_CLIENTEDGE, "EDIT", label, dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SendMessage ((HWND) handle, EM_LIMITTEXT, (WPARAM) 256, 0L);
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_LINEEDIT);
	setParent (parent);
	setId (id);

	//d_this->wndProc = (WNDPROC) ::SetWindowLong((HWND) handle, GWL_WNDPROC, (DWORD) mxLineEdit_WindowProc);
	parent->addWidget (this);
}



mxLineEdit::~mxLineEdit ()
{
}



void
mxLineEdit::setMaxLength (int max)
{
	SendMessage ((HWND) getHandle (), EM_LIMITTEXT, (WPARAM) max, 0L);
}



int
mxLineEdit::getMaxLength () const
{
	return (int) SendMessage ((HWND) getHandle (), EM_GETLIMITTEXT, 0, 0L);
}
