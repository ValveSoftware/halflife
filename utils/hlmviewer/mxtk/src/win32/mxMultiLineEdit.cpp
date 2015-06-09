//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxMultiLineEdit.cpp
// implementation: Win32 API
// last modified:  May 24 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxMultiLineEdit.h>
#include <mx/mxWindow.h>
#include <windows.h>



class mxMultiLineEdit_i
{
public:
	int dummy;
};



mxMultiLineEdit::mxMultiLineEdit (mxWindow *parent, int x, int y, int w, int h, const char *label, int id, int style)
: mxWidget (parent, x, y, w, h, label)
{
	if (!parent)
		return;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	if (style == ReadOnly)
		dwStyle |= ES_READONLY;
	else if (style == Password)
		dwStyle |= ES_PASSWORD;

	void *handle = (void *) CreateWindowEx (WS_EX_CLIENTEDGE, "EDIT", label, dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_MULTILINEEDIT);
	setParent (parent);
	setId (id);

	parent->addWidget (this);
}



mxMultiLineEdit::~mxMultiLineEdit ()
{
}



void
mxMultiLineEdit::setMaxLength (int max)
{
	SendMessage ((HWND) getHandle (), EM_LIMITTEXT, (WPARAM) max, 0L);
}



void
mxMultiLineEdit::appendText (const char *text)
{
	SendMessage ((HWND) getHandle (), EM_SETSEL, (WPARAM) -1, (LPARAM) -1);
	SendMessage ((HWND) getHandle (), EM_REPLACESEL, 0, (LPARAM) text);
}



int
mxMultiLineEdit::getMaxLength () const
{
	return (int) SendMessage ((HWND) getHandle (), EM_GETLIMITTEXT, 0, 0L);
}
