//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxToggleButton.cpp
// implementation: Win32 API
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxToggleButton.h>
#include <mx/mxWindow.h>
#include <windows.h>



class mxToggleButton_i
{
public:
	int dummy;
};



mxToggleButton::mxToggleButton (mxWindow *parent, int x, int y, int w, int h, const char *label, int id)
: mxWidget (parent, x, y, w, h, label)
{
	if (!parent)
		return;

	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	void *handle = (void *) CreateWindowEx (0, "BUTTON", label, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHLIKE | BS_AUTOCHECKBOX,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_TOGGLEBUTTON);
	setParent (parent);
	setId (id);
	setChecked (false);

	parent->addWidget (this);
}



mxToggleButton::~mxToggleButton ()
{
}



void
mxToggleButton::setChecked (bool b)
{
	SendMessage ((HWND) getHandle (), BM_SETCHECK, (WPARAM) b ? BST_CHECKED:BST_UNCHECKED, 0L);
}



bool
mxToggleButton::isChecked () const
{
	return (SendMessage ((HWND) getHandle (), BM_GETCHECK, 0, 0L) == BST_CHECKED);
}
