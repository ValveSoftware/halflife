//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxListBox.cpp
// implementation: Win32 API
// last modified:  Mar 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxListBox.h>
#include <mx/mxWindow.h>
#include <windows.h>



class mxListBox_i
{
public:
	int dummy;
};



mxListBox::mxListBox (mxWindow *parent, int x, int y, int w, int h, int id, int style)
: mxWidget (parent, x, y, w, h)
{
	if (!parent)
		return;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_HSCROLL | WS_BORDER;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	if (style == MultiSelection)
		dwStyle |= LBS_MULTIPLESEL | LBS_EXTENDEDSEL;

	void *handle = (void *) CreateWindowEx (WS_EX_CLIENTEDGE, "LISTBOX", "", dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_LISTBOX);
	setParent (parent);
	setId (id);

	parent->addWidget (this);
}



mxListBox::~mxListBox ()
{
	removeAll ();
}



void
mxListBox::add (const char *item)
{
	SendMessage ((HWND) getHandle (), LB_ADDSTRING, 0, (LPARAM) (LPCTSTR) item);
}



void
mxListBox::select (int index)
{
	SendMessage ((HWND) getHandle (), LB_SETCURSEL, (WPARAM) index, 0L);
}



void
mxListBox::deselect (int index)
{
	SendMessage ((HWND) getHandle (), LB_SETSEL, (WPARAM) FALSE, (LPARAM) (UINT) index);
}



void
mxListBox::remove (int index)
{
	SendMessage ((HWND) getHandle (), LB_DELETESTRING, (WPARAM) index, 0L);
}



void
mxListBox::removeAll ()
{
	SendMessage ((HWND) getHandle (), LB_RESETCONTENT, 0, 0L);
}



void
mxListBox::setItemText (int index, const char *item)
{
	//SendMessage ((HWND) getHandle (), LB_SETTEXT, (WPARAM) index, (LPARAM) (LPCTSTR) item);
}



void
mxListBox::setItemData (int index, void *data)
{
	SendMessage ((HWND) getHandle (), LB_SETITEMDATA, (WPARAM) index, (LPARAM) data);
}



int
mxListBox::getItemCount () const
{
	return (int) SendMessage ((HWND) getHandle (), LB_GETCOUNT, 0, 0L);
}



int
mxListBox::getSelectedIndex () const
{
	return (int) SendMessage ((HWND) getHandle (), LB_GETCURSEL, 0, 0L);
}



bool
mxListBox::isSelected (int index) const
{
	return (bool) (SendMessage ((HWND) getHandle (), LB_GETSEL, (WPARAM) index, 0L) > 0);
}



const char*
mxListBox::getItemText (int index) const
{
	static char text[256];
	SendMessage ((HWND) getHandle (), LB_GETTEXT, (WPARAM) index, (LPARAM) (LPCTSTR) text);
	return text;
}



void*
mxListBox::getItemData (int index) const
{
	return (void *) SendMessage ((HWND) getHandle (), LB_GETITEMDATA, (WPARAM) index, 0L);
}
