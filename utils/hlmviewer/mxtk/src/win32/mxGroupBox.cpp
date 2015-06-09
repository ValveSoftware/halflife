//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxGroupBox.cpp
// implementation: Win32 API
// last modified:  Mar 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxGroupBox.h>
#include <mx/mxWindow.h>
#include <windows.h>



class mxGroupBox_i
{
public:
	int dummy;
};



mxGroupBox::mxGroupBox (mxWindow *parent, int x, int y, int w, int h, const char *label)
: mxWidget (parent, x, y, w, h, label)
{
	if (!parent)
		return;

	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	void *handle = (void *) CreateWindowEx (0, "BUTTON", label, WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
				x, y, w, h, hwndParent,
				(HMENU) NULL, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));

	setHandle (handle);
	setType (MX_GROUPBOX);
	setParent (parent);

	parent->addWidget (this);
}



mxGroupBox::~mxGroupBox ()
{
}
