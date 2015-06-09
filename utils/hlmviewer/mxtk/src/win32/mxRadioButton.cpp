//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxRadioButton.cpp
// implementation: Win32 API
// last modified:  Mar 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxRadioButton.h>
#include <mx/mxWindow.h>
#include <windows.h>



class mxRadioButton_i
{
public:
	int dummy;
};



mxRadioButton::mxRadioButton (mxWindow *parent, int x, int y, int w, int h, const char *label, int id, bool newGroup)
: mxWidget (parent, x, y, w, h, label)
{
	if (!parent)
		return;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTORADIOBUTTON;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	if (newGroup)
		dwStyle |= WS_GROUP;

	void *handle = (void *) CreateWindowEx (0, "BUTTON", label, dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_RADIOBUTTON);
	setParent (parent);
	setId (id);

	setChecked (newGroup);

	parent->addWidget (this);
}



mxRadioButton::~mxRadioButton ()
{
}



void
mxRadioButton::setChecked (bool b)
{
	SendMessage ((HWND) getHandle (), BM_SETCHECK, (WPARAM) b ? BST_CHECKED:BST_UNCHECKED, 0L);
}



bool
mxRadioButton::isChecked () const
{
	return (SendMessage ((HWND) getHandle (), BM_GETCHECK, 0, 0L) == BST_CHECKED);
}
