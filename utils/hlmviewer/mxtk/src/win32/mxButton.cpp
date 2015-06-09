//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxButton.cpp
// implementation: Win32 API
// last modified:  Apr 12 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxButton.h>
#include <mx/mxWindow.h>
#include <windows.h>



class mxButton_i
{
public:
	int dummy;
};



mxButton::mxButton (mxWindow *parent, int x, int y, int w, int h, const char *label, int id)
: mxWidget (parent, x, y, w, h, label)
{
	if (!parent)
		return;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	void *handle = (void *) CreateWindowEx (0, "BUTTON", label, dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setType (MX_BUTTON);
	setHandle (handle);
	setParent (parent);
	setId (id);

	parent->addWidget (this);
}



mxButton::~mxButton ()
{
}
