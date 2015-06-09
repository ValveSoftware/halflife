//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxChoice.cpp
// implementation: Win32 API
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxChoice.h>
#include <mx/mxWindow.h>
#include <windows.h>



class mxChoice_i
{
public:
	int dummy;
};



mxChoice::mxChoice (mxWindow *parent, int x, int y, int w, int h, int id)
: mxWidget (parent, x, y, w, h)
{
	if (!parent)
		return;

	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	void *handle = (void *) CreateWindowEx (0, "COMBOBOX", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
				x, y, w, h + 100, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_CHOICE);
	setParent (parent);
	setId (id);

	parent->addWidget (this);
}



mxChoice::~mxChoice ()
{
	removeAll ();
}



void
mxChoice::add (const char *item)
{
	SendMessage ((HWND) getHandle (), CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) item);
}



void
mxChoice::select (int index)
{
	SendMessage ((HWND) getHandle (), CB_SETCURSEL, (WPARAM) index, 0L);
}



void
mxChoice::remove (int index)
{
	SendMessage ((HWND) getHandle (), CB_DELETESTRING, (WPARAM) index, 0L);
}



void
mxChoice::removeAll ()
{
	SendMessage ((HWND) getHandle (), CB_RESETCONTENT, 0, 0L);
}



int
mxChoice::getItemCount () const
{
	return (int) SendMessage ((HWND) getHandle (), CB_GETCOUNT, 0, 0L);
}



int
mxChoice::getSelectedIndex () const
{
	return (int) SendMessage ((HWND) getHandle (), CB_GETCURSEL, 0, 0L);
}
